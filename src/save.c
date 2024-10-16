/* save.c: Save file handler
 * Copyright (c) 2023, 2024 Nathan Misner
 *
 * This file is part of OpenMadoola.
 *
 * OpenMadoola is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * OpenMadoola is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMadoola. If not, see <https://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "bg.h"
#include "buffer.h"
#include "constants.h"
#include "demo.h"
#include "file.h"
#include "game.h"
#include "item.h"
#include "joy.h"
#include "lucia.h"
#include "mainmenu.h"
#include "platform.h"
#include "save.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "task.h"
#include "util.h"
#include "weapon.h"

static Uint8 savePalette[] = {
    0x0f, 0x20, 0x20, 0x20,
    0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f,
    0x0F, 0x12, 0x16, 0x36,
    0x0F, 0x36, 0x26, 0x16,
    0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f,
};

#define NUM_FILES 3
static SaveData *saves[NUM_FILES];
static int currFile;
SaveData *sd;

void Save_Serialize(Buffer *buf) {
    assert(sd);
    Buffer_AddSint16(buf, sd->maxHealth);
    Buffer_AddSint16(buf, sd->maxMagic);
    Buffer_Add(buf, sd->highestReachedStage);
    Buffer_Add(buf, sd->keywordDisplay);
    Buffer_Add(buf, sd->orbCollected);
    for (int i = 0; i < NUM_WEAPONS; i++) {
        Buffer_Add(buf, sd->weaponLevels[i]);
    }
    Buffer_Add(buf, sd->bootsLevel);
    for (int i = 0; i < ARRAY_LEN(sd->itemsCollected); i++) {
        Buffer_Add(buf, sd->itemsCollected[i]);
    }
    for (int i = 0; i < ARRAY_LEN(sd->bossDefeated); i++) {
        Buffer_Add(buf, sd->bossDefeated[i]);
    }
}

static int Save_DeserializeToPtr(Uint8 *data, SaveData *out) {
    int cursor = 0;
    out->maxHealth = Util_LoadSint16(data + cursor);
    cursor += 2;
    out->maxMagic = Util_LoadSint16(data + cursor);
    cursor += 2;
    out->highestReachedStage = data[cursor++];
    out->keywordDisplay = data[cursor++];
    out->orbCollected = data[cursor++];
    memcpy(out->weaponLevels, data + cursor, sizeof(out->weaponLevels));
    cursor += sizeof(out->weaponLevels);
    out->bootsLevel = data[cursor++];
    memcpy(out->itemsCollected, data + cursor, sizeof(out->itemsCollected));
    cursor += sizeof(out->itemsCollected);
    memcpy(out->bossDefeated, data + cursor, sizeof(out->bossDefeated));
    cursor += sizeof(out->bossDefeated);
    return cursor;
}

int Save_Deserialize(Uint8 *data) {
    return Save_DeserializeToPtr(data, sd);
}

void Save_SaveFile(void) {
    static Buffer *buf = NULL;
    char filename[20];

    if (!buf) {
        buf = Buffer_Init(sizeof(SaveData));
    }
    buf->dataSize = 0;
    Save_Serialize(buf);
    snprintf(filename, sizeof(filename), "file%d.sav", currFile + 1);
    Buffer_WriteToFile(buf, filename);
}

void Save_EraseFile(int num) {
    free(saves[num]);
    saves[num] = NULL;
    char filename[20];
    snprintf(filename, sizeof(filename), "file%d.sav", num + 1);
    remove(filename);
}

void Save_Init(void) {
    char filename[20];
    Buffer *buf = Buffer_Init(sizeof(SaveData));
    for (int i = 0; i < NUM_FILES; i++) {
        snprintf(filename, sizeof(filename), "file%d.sav", i + 1);
        FILE *fp = File_Open(filename, "rb");
        if (fp) {
            // load file into the buffer
            buf->dataSize = 0;
            Buffer_AddFile(buf, fp);
            // load data from the buffer
            assert(!saves[i]);
            saves[i] = ommalloc(sizeof(SaveData));
            Save_DeserializeToPtr(buf->data, saves[i]);
            fclose(fp);
        }
        else {
            saves[i] = NULL;
        }
    }
    Buffer_Destroy(buf);
}

static void Save_DrawLucia(Sint16 x, Sint16 y, Uint16 tile) {
    Sprite luciaSpr = { 0 };
    luciaSpr.x = x - BG_CENTERED_X;
    luciaSpr.y = y;
    luciaSpr.size = SPRITE_16X16;
    luciaSpr.tile = tile;
    luciaSpr.palette = 0;
    luciaSpr.mirror = H_MIRROR;
    Sprite_Draw(&luciaSpr, NULL);
    luciaSpr.tile += 2;
    luciaSpr.y += 16;
    Sprite_Draw(&luciaSpr, NULL);
}

static Uint16 luciaRunTiles[] = {
    0x4, 0x8, 0xc, 0x8,
};

// menu indices
#define ERASE_INDEX NUM_FILES
#define MAIN_MENU_INDEX (ERASE_INDEX + 1)

// position defines
#define HEADER_X 10
#define HEADER_Y 2

#define ERASE_X 10
#define ERASE_Y 23
#define ERASE_Y_PX (ERASE_Y * 8)
#define ERASE_SPACING 3
#define ERASE_SPACING_PX (ERASE_SPACING * 8)

#define TITLE_X 10
#define TITLE_Y (ERASE_Y + ERASE_SPACING)

#define SAVE_X 8
#define SAVE_Y 6
#define SAVE_Y_PX (SAVE_Y * 8)
#define SAVE_SPACING 6
#define SAVE_SPACING_PX (SAVE_SPACING * 8)

#define LUCIA_X_PX 48
#define LUCIA_Y_PX (SAVE_Y_PX - 16)

void Save_Screen(void) {
    int erase = 0;
    int cursor = currFile;
    int stages[NUM_FILES];
    int highestStages[NUM_FILES];
    Sprite cursorSpr = { 0 };

    Sound_Reset();
    BG_Clear();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetAllPalettes(savePalette);
    Sprite_SetAllPalettes(savePalette + 16);
    for (int i = 0; i < NUM_FILES; i++) {
        if (saves[i]) {
            stages[i] = saves[i]->highestReachedStage;
            highestStages[i] = stages[i];
        }
    }

    while (1) {
        Sprite_ClearList();

        // cursor movement (with wraparound)
        if (joyEdge & JOY_UP) {
            cursor--;
            if (cursor < 0) { cursor = MAIN_MENU_INDEX; }
            Sound_Play(SFX_MENU);
        }
        if (joyEdge & (JOY_DOWN | JOY_SELECT)) {
            cursor++;
            if (cursor > MAIN_MENU_INDEX) { cursor = 0; }
            Sound_Play(SFX_MENU);
        }

        if (!erase) {
            cursorSpr.size = SPRITE_8X16;
            cursorSpr.tile = 0xee;
            cursorSpr.palette = 1;
            cursorSpr.mirror = 0;
        }
        else {
            cursorSpr.size = SPRITE_8X16;
            cursorSpr.tile = 0x48;
            cursorSpr.palette = 0;
            cursorSpr.mirror = H_MIRROR;
        }
        // first 3 positions = save files, last = "erase game" text
        if (cursor < NUM_FILES) {
            if (saves[cursor] && !erase) {
                if (joyEdge & JOY_LEFT) {
                    stages[cursor]--;
                    stages[cursor] = MAX(stages[cursor], 0);
                    Sound_Play(SFX_MENU);
                }
                if (joyEdge & JOY_RIGHT) {
                    stages[cursor]++;
                    stages[cursor] = MIN(stages[cursor], highestStages[cursor]);
                    Sound_Play(SFX_MENU);
                }
            }
            // move the cursor over if there's a file
            cursorSpr.x = (saves[cursor] ? 30 : 80) - BG_CENTERED_X;
            cursorSpr.y = (SAVE_Y_PX - 5) + (cursor * SAVE_SPACING_PX);
        }
        else {
            cursorSpr.x = 72 - BG_CENTERED_X;
            cursorSpr.y = (ERASE_Y_PX - 5) + ((cursor - NUM_FILES) * ERASE_SPACING_PX);
        }
        Sprite_Draw(&cursorSpr, NULL);

        // draw the text
        if (!erase) {
            BG_Print(HEADER_X, HEADER_Y, 0, "Select Game");
            BG_Print(ERASE_X,  ERASE_Y,  0, "Erase File");
            BG_Print(TITLE_X,  TITLE_Y,  0, "Main Menu");
        }
        else {
            BG_Print(HEADER_X, HEADER_Y, 0, "Erase File ");
            BG_Print(ERASE_X,  ERASE_Y,  0, "Cancel    ");
            BG_Print(TITLE_X,  TITLE_Y,  0, "Main Menu");
        }

        // draw save file text
        for (int i = 0; i < NUM_FILES; i++) {
            if (saves[i]) {
                BG_Print(SAVE_X, (i * SAVE_SPACING) + SAVE_Y, 0, "File %d   Stage - %2u -", i + 1, stages[i] + 1);
                // if the save file exists, draw a Lucia sprite for it
                Save_DrawLucia(LUCIA_X_PX, (i * SAVE_SPACING_PX) + LUCIA_Y_PX, 0);
            }
            else {
                BG_Print(SAVE_X, (i * SAVE_SPACING) + SAVE_Y, 0, "   New Game            ");
            }
        }

        BG_Display();
        Sprite_Display();
        Task_Yield();

        if (joyEdge & (JOY_A | JOY_START)) {
            // toggle normal/erase mode
            if (cursor == ERASE_INDEX) {
                erase ^= 1;
                cursor = 0;
            }
            // return to main menu
            else if (cursor == MAIN_MENU_INDEX) {
                Task_Switch(MainMenu_Run);
            }
            // selected a file to erase
            else if (erase) {
                Save_EraseFile(cursor);
                erase = 0;
                cursor = 0;
            }
            // selected a file to play
            else { break; }
        }
    }

    currFile = cursor;
    if (saves[cursor]) {
        // lucia running offscreen animation
        Sint16 luciaX = LUCIA_X_PX;
        int frames = 0;
        while (luciaX < 256) {
            Sprite_ClearList();
            Sprite_Draw(&cursorSpr, NULL);
            frames++;
            for (int i = 0; i < NUM_FILES; i++) {
                if (i == currFile) {
                    Save_DrawLucia(luciaX, (i * SAVE_SPACING_PX) + LUCIA_Y_PX, luciaRunTiles[(frames >> 3) & 3]);
                }
                else if (saves[i]) {
                    Save_DrawLucia(LUCIA_X_PX, (i * SAVE_SPACING_PX) + LUCIA_Y_PX, 0);
                }
            }
            luciaX += 3;
            BG_Display();
            Sprite_Display();
            Task_Yield();
            if (joyEdge & JOY_START) { break; }
        }

        sd = saves[cursor];
        stage = stages[cursor];
        Task_Switch(Game_LoadGame);
    }
    Task_Switch(Game_NewGame);
}
