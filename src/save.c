/* save.c: Save file handler
*
*  This file is part of OpenMadoola.
*
*  OpenMadoola is free software: you can redistribute it and/or modify it
*  under the terms of the GNU General Public License as published by the Free
*  Software Foundation; either version 2 of the License, or (at your option)
*  any later version.
*
*  OpenMadoola is distributed in the hope that it will be useful, but WITHOUT
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
*  more details.
*
*  You should have received a copy of the GNU General Public License
*  along with OpenMadoola. If not, see <https://www.gnu.org/licenses/>.
*/

#include <string.h>

#include "bg.h"
#include "constants.h"
#include "db.h"
#include "file.h"
#include "game.h"
#include "item.h"
#include "joy.h"
#include "lucia.h"
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

static Uint16 Save_Read16BE(Uint8 *buff, int *cursor) {
    Uint16 val = (buff[*cursor] << 8) | buff[*cursor + 1];
    *cursor += 2;
    return val;
}

static Uint32 Save_Read32BE(Uint8 *buff, int *cursor) {
    Uint32 val;
    val  = buff[*cursor + 0] << 24;
    val |= buff[*cursor + 1] << 16;
    val |= buff[*cursor + 2] << 8;
    val |= buff[*cursor + 3];
    *cursor += 4;
    return val;
}

static void Save_Write16BE(Uint16 val, Uint8 *buff, int *cursor) {
    buff[(*cursor)++] = (val >> 8) & 0xff;
    buff[(*cursor)++] = (val >> 0) & 0xff;
}

static void Save_Write32BE(Uint32 val, Uint8 *buff, int *cursor) {
    buff[(*cursor)++] = (val >> 24) & 0xff;
    buff[(*cursor)++] = (val >> 16) & 0xff;
    buff[(*cursor)++] = (val >>  8) & 0xff;
    buff[(*cursor)++] = (val >>  0) & 0xff;
}

void Save_SaveFile(void) {
    char name[20];
    snprintf(name, sizeof(name), "file%d", currFile + 1);
    // we save without padding so this will always be >= the size we save
    Uint8 buff[sizeof(SaveFile)];
    int cursor = 0;

    files[currFile].maxHealth = maxHealth;
    Save_Write16BE(maxHealth, buff, &cursor);
    files[currFile].maxMagic = maxMagic;
    Save_Write16BE(maxMagic, buff, &cursor);
    files[currFile].highestStage = highestReachedStage;
    buff[cursor++] = highestReachedStage;
    files[currFile].keywordDisplay = keywordDisplay;
    buff[cursor++] = keywordDisplay;
    files[currFile].orbCollected = orbCollected;
    buff[cursor++] = orbCollected;
    for (int i = 0; i < NUM_WEAPONS; i++) {
        files[currFile].levels[i] = weaponLevels[i];
        buff[cursor++] = weaponLevels[i];
    }
    files[currFile].levels[NUM_WEAPONS] = bootsLevel;
    buff[cursor++] = bootsLevel;
    for (int i = 0; i < ARRAY_LEN(itemsCollected); i++) {
        files[currFile].items[i] = itemsCollected[i];
        buff[cursor++] = itemsCollected[i];
    }
    for (int i = 0; i < ARRAY_LEN(bossDefeated); i++) {
        files[currFile].bossDefeated[i] = bossDefeated[i];
        buff[cursor++] = bossDefeated[i];
    }
    files[currFile].signature = VALID_SIGNATURE;
    Save_Write32BE(VALID_SIGNATURE, buff, &cursor);
    DB_Set(name, buff, (Uint32)cursor);
    DB_Save();
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
    char name[20];
    snprintf(name, sizeof(name), "file%d", num + 1);
    DBEntry *entry = DB_Find(name);
    if (entry) {
        memset(entry->data, 0, entry->dataLen);
    }
    DB_Save();
}

static void Save_ReadRange(Uint8 *dest, Uint8 *src, int len, int *cursor) {
    memcpy(dest, &src[*cursor], len);
    *cursor += len;
}

void Save_Init(void) {
    char name[20];
    DBEntry *entry;
    for (int i = 0; i < NUM_FILES; i++) {
        snprintf(name, sizeof(name), "file%d", i + 1);
        entry = DB_Find(name);
        if (entry) {
            int cursor = 0;
            files[i].maxHealth = Save_Read16BE(entry->data, &cursor);
            files[i].maxMagic = Save_Read16BE(entry->data, &cursor);
            files[i].highestStage = entry->data[cursor++];
            files[i].keywordDisplay = entry->data[cursor++];
            files[i].orbCollected = entry->data[cursor++];
            Save_ReadRange(&files[i].levels[0], entry->data, ARRAY_LEN(files[0].levels), &cursor);
            Save_ReadRange(&files[i].items[0], entry->data, ARRAY_LEN(files[0].items), &cursor);
            Save_ReadRange(&files[i].bossDefeated[0], entry->data, ARRAY_LEN(files[0].bossDefeated), &cursor);
            files[i].signature = Save_Read32BE(entry->data, &cursor);
        }
    }
}

static void Save_DrawLucia(Sint16 x, Sint16 y, Uint16 tile) {
    Sprite luciaSpr = { 0 };
    luciaSpr.x = x;
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

int Save_Screen(void) {
    int erase = 0;
    int cursor = 0;
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
            if (cursor < 0) { cursor = NUM_FILES; }
            Sound_Play(SFX_MENU);
        }
        if (joyEdge & (JOY_DOWN | JOY_SELECT)) {
            cursor++;
            if (cursor > NUM_FILES) { cursor = 0; }
            Sound_Play(SFX_MENU);
        }
        int saveValid = (files[cursor].signature == VALID_SIGNATURE);

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
            cursorSpr.x = saveValid ? 26 : 64;
            cursorSpr.y = 59 + (cursor * 48);
        }
        else {
            cursorSpr.x = 64;
            cursorSpr.y = 187 + ((cursor - NUM_FILES) * 32);
        }
        Sprite_Draw(&cursorSpr, NULL);

        // draw the text
        if (!erase) {
            BG_Print(10, 4, 0, "Select Game");
            BG_Print(10, 24, 0, "Erase File");
        }
        else {
            BG_Print(10, 4, 0, "Erase File ");
            BG_Print(10, 24, 0, " Cancel   ");
        }

        // draw save file text
        for (int i = 0; i < NUM_FILES; i++) {
            if (files[i].signature == VALID_SIGNATURE) {
                BG_Print(8, (i * 6) + 8, 0, "File %d   Stage - %2u -", i + 1, stages[i] + 1);
                // if the save file exists, draw a Lucia sprite for it
                Save_DrawLucia(48, (i * 48) + 48, 0);
            }
            else {
                BG_Print(8, (i * 6) + 8, 0, "  New Game           ");
            }
        }

        BG_Draw();
        Sprite_EndFrame();
        System_EndFrame();

        if (joyEdge & (JOY_A | JOY_START)) {
            // toggle normal/erase mode
            if (cursor == NUM_FILES) {
                erase ^= 1;
                cursor = 0;
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
        Sint16 luciaX = 48;
        int frames = 0;
        while (luciaX < 256) {
            System_StartFrame();
            Sprite_ClearList();
            Sprite_Draw(&cursorSpr, NULL);
            frames++;
            for (int i = 0; i < NUM_FILES; i++) {
                if (i == currFile) {
                    Save_DrawLucia(luciaX, (i * 48) + 48, luciaRunTiles[(frames >> 3) & 3]);
                }
                else if (files[i].signature == VALID_SIGNATURE) {
                    Save_DrawLucia(48, (i * 48) + 48, 0);
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
        return 1;
    }
    return 0;

}
