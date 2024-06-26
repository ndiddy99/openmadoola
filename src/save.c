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

#include <string.h>

#include "bg.h"
#include "constants.h"
#include "file.h"
#include "game.h"
#include "item.h"
#include "joy.h"
#include "lucia.h"
#include "platform.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
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

// :)
#define VALID_SIGNATURE 0xB16B00B5

typedef struct {
    Uint16 maxHealth;
    Uint16 maxMagic;
    Uint8 highestStage;
    Uint8 keywordDisplay;
    Uint8 orbCollected;
    Uint8 levels[NUM_WEAPONS + 1];
    Uint8 items[8];
    Uint8 bossDefeated[16];
    Uint32 signature;
} SaveFile;

#define NUM_FILES 3
static SaveFile files[NUM_FILES];
static int currFile;

void Save_SaveFile(void) {
    char filename[20];
    snprintf(filename, sizeof(filename), "file%d.sav", currFile + 1);
    FILE *fp = File_Open(filename, "wb");
    if (!fp) {
        Platform_ShowError("Error opening save file for writing");
        return;
    }

    files[currFile].maxHealth = maxHealth;
    File_WriteUint16BE(maxHealth, fp);
    files[currFile].maxMagic = maxMagic;
    File_WriteUint16BE(maxMagic, fp);
    files[currFile].highestStage = highestReachedStage;
    fputc(highestReachedStage, fp);
    files[currFile].keywordDisplay = keywordDisplay;
    fputc(keywordDisplay, fp);
    files[currFile].orbCollected = orbCollected;
    fputc(orbCollected, fp);
    for (int i = 0; i < NUM_WEAPONS; i++) {
        files[currFile].levels[i] = weaponLevels[i];
        fputc(weaponLevels[i], fp);
    }
    files[currFile].levels[NUM_WEAPONS] = bootsLevel;
    fputc(bootsLevel, fp);
    for (int i = 0; i < ARRAY_LEN(itemsCollected); i++) {
        files[currFile].items[i] = itemsCollected[i];
        fputc(itemsCollected[i], fp);
    }
    for (int i = 0; i < ARRAY_LEN(bossDefeated); i++) {
        files[currFile].bossDefeated[i] = bossDefeated[i];
        fputc(bossDefeated[i], fp);
    }
    files[currFile].signature = VALID_SIGNATURE;
    File_WriteUint32BE(VALID_SIGNATURE, fp);
    fclose(fp);
}

void Save_LoadFile(void) {
    maxHealth = files[currFile].maxHealth;
    maxMagic = files[currFile].maxMagic;
    highestReachedStage = files[currFile].highestStage;
    keywordDisplay = files[currFile].keywordDisplay;
    orbCollected = files[currFile].orbCollected;
    memcpy(weaponLevels, files[currFile].levels, sizeof(weaponLevels));
    bootsLevel = files[currFile].levels[NUM_WEAPONS];
    memcpy(itemsCollected, files[currFile].items, sizeof(itemsCollected));
    memcpy(bossDefeated, files[currFile].bossDefeated, sizeof(bossDefeated));
}

void Save_EraseFile(int num) {
    memset(&files[num], 0, sizeof(SaveFile));
    char filename[20];
    snprintf(filename, sizeof(filename), "file%d.sav", num + 1);
    FILE *fp = File_Open(filename, "wb");
    if (fp) {
        for (int i = 0; i < sizeof(SaveFile); i++) {
            fputc(0, fp);
        }
        fclose(fp);
    }
}

void Save_Init(void) {
    char filename[20];
    for (int i = 0; i < NUM_FILES; i++) {
        snprintf(filename, sizeof(filename), "file%d.sav", i + 1);
        FILE *fp = File_Open(filename, "rb");
        if (fp) {
            files[i].maxHealth = File_ReadUint16BE(fp);
            files[i].maxMagic = File_ReadUint16BE(fp);
            files[i].highestStage = fgetc(fp);
            files[i].keywordDisplay = fgetc(fp);
            files[i].orbCollected = fgetc(fp);
            fread(&files[i].levels[0], 1, ARRAY_LEN(files[0].levels), fp);
            fread(&files[i].items[0], 1, ARRAY_LEN(files[0].items), fp);
            fread(&files[i].bossDefeated[0], 1, ARRAY_LEN(files[0].bossDefeated), fp);
            files[i].signature = File_ReadUint32BE(fp);
            fclose(fp);
        }
    }
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
#define TITLE_INDEX (ERASE_INDEX + 1)

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

int Save_Screen(void) {
    int erase = 0;
    int cursor = currFile;
    int stages[NUM_FILES];
    Sprite cursorSpr = { 0 };

    Sound_Reset();
    BG_Clear();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetAllPalettes(savePalette);
    Sprite_SetAllPalettes(savePalette + 16);
    for (int i = 0; i < NUM_FILES; i++) {
        stages[i] = files[i].highestStage;
    }

    while (1) {
        System_StartFrame();
        Sprite_ClearList();

        // cursor movement (with wraparound)
        if (joyEdge & JOY_UP) {
            cursor--;
            if (cursor < 0) { cursor = TITLE_INDEX; }
            Sound_Play(SFX_MENU);
        }
        if (joyEdge & (JOY_DOWN | JOY_SELECT)) {
            cursor++;
            if (cursor > TITLE_INDEX) { cursor = 0; }
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
            int saveValid = (files[cursor].signature == VALID_SIGNATURE);
            if (saveValid && !erase) {
                if (joyEdge & JOY_LEFT) {
                    stages[cursor]--;
                    stages[cursor] = MAX(stages[cursor], 0);
                    Sound_Play(SFX_MENU);
                }
                if (joyEdge & JOY_RIGHT) {
                    stages[cursor]++;
                    stages[cursor] = MIN(stages[cursor], files[cursor].highestStage);
                    Sound_Play(SFX_MENU);
                }
            }
            // move the cursor over if there's a file
            cursorSpr.x = (saveValid ? 30 : 80) - BG_CENTERED_X;
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
            BG_Print(TITLE_X,  TITLE_Y,  0, "Title Screen");
        }
        else {
            BG_Print(HEADER_X, HEADER_Y, 0, "Erase File ");
            BG_Print(ERASE_X,  ERASE_Y,  0, "Cancel    ");
            BG_Print(TITLE_X,  TITLE_Y,  0, "Title Screen");
        }

        // draw save file text
        for (int i = 0; i < NUM_FILES; i++) {
            if (files[i].signature == VALID_SIGNATURE) {
                BG_Print(SAVE_X, (i * SAVE_SPACING) + SAVE_Y, 0, "File %d   Stage - %2u -", i + 1, stages[i] + 1);
                // if the save file exists, draw a Lucia sprite for it
                Save_DrawLucia(LUCIA_X_PX, (i * SAVE_SPACING_PX) + LUCIA_Y_PX, 0);
            }
            else {
                BG_Print(SAVE_X, (i * SAVE_SPACING) + SAVE_Y, 0, "   New Game            ");
            }
        }

        BG_Draw();
        Sprite_EndFrame();
        System_EndFrame();

        if (joyEdge & (JOY_A | JOY_START)) {
            // toggle normal/erase mode
            if (cursor == ERASE_INDEX) {
                erase ^= 1;
                cursor = 0;
            }
            // return to tile screen
            else if (cursor == TITLE_INDEX) {
                return 0;
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
    if (files[cursor].signature == VALID_SIGNATURE) {
        // lucia running offscreen animation
        Sint16 luciaX = LUCIA_X_PX;
        int frames = 0;
        while (luciaX < 256) {
            System_StartFrame();
            Sprite_ClearList();
            Sprite_Draw(&cursorSpr, NULL);
            frames++;
            for (int i = 0; i < NUM_FILES; i++) {
                if (i == currFile) {
                    Save_DrawLucia(luciaX, (i * SAVE_SPACING_PX) + LUCIA_Y_PX, luciaRunTiles[(frames >> 3) & 3]);
                }
                else if (files[i].signature == VALID_SIGNATURE) {
                    Save_DrawLucia(LUCIA_X_PX, (i * SAVE_SPACING_PX) + LUCIA_Y_PX, 0);
                }
            }
            luciaX += 3;
            BG_Draw();
            Sprite_EndFrame();
            System_EndFrame();
            if (joyEdge & JOY_START) { break; }
        }

        Save_LoadFile();
        stage = stages[cursor];
        return 2;
    }
    return 1;
}
