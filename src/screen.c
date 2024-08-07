/* screen.c: Status, stage, keyword screens
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "bg.h"
#include "constants.h"
#include "game.h"
#include "joy.h"
#include "lucia.h"
#include "palette.h"
#include "screen.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "textscroll.h"
#include "weapon.h"

static Uint16 *Screen_WriteTiles(Uint16 *tiles) {
    Uint16 x = *tiles++;
    Uint16 y = *tiles++;
    while (*tiles) {
        BG_SetTile(x++, y, *tiles, 0);
        tiles++;
    }
    tiles++; // advance past NUL terminator to next row
    return tiles;
}

static Uint8 statusPalette[] = {
    0x0B, 0x2B, 0x2B, 0x2B,
    0x0B, 0x2B, 0x2B, 0x2B,
    0x0B, 0x2B, 0x2B, 0x2B,
    0x0B, 0x2B, 0x2B, 0x2B,
};

static Uint8 statusPaletteArcade[] = {
    0x1D, 0x2B, 0x2B, 0x2B,
};

static Uint8 statusItemPalettes[] = {
    1, 3, 3, 1, 3, 3, 1, 0,
};

static Uint16 statusItemTiles[] = {
    0x60,
    0x60,
    0x66,
    0x62,
    0x64,
    0x68,
    0x6A,
    0x2BC,
};

static void Screen_StatusIcon(Uint16 x, Uint16 y, Uint16 tile, Uint8 palette, Uint8 level) {
    Sprite spr = { 0 };
    spr.size = SPRITE_8X16;
    spr.x = x;
    spr.y = y;
    spr.tile = tile;
    spr.palette = palette;

    for (Uint8 i = 0; i < level; i++) {
        Sprite_Draw(&spr, NULL);
        spr.x += 16;
    }
}

void Screen_Status(void) {
    BG_Clear();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetAllPalettes(statusPalette);
    if (gameType == GAME_TYPE_ARCADE) {
        BG_SetPalette(0, statusPaletteArcade);
    }
    Sprite_SetAllPalettes(spritePalettes);
    Sound_Reset();
    Sound_Play(MUS_START);

    // print the text
    BG_Print(13,  3, 0, "STATUS");
    BG_Print(11,  6, 0, "HITS  %04d/%04d", health, maxHealth);
    BG_Print( 9,  8, 0, "MAGICS  %04d/%04d", magic, maxMagic);
    BG_Print(10, 11, 0, "SWORD");
    BG_Print( 4, 13, 0, "FLAME SWORD");
    BG_Print( 5, 15, 0, "MAGIC BOMB");
    BG_Print( 5, 17, 0, "BOUND BALL");
    BG_Print( 4, 19, 0, "SHIELD BALL");
    BG_Print( 8, 21, 0, "SMASHER");
    BG_Print(10, 23, 0, "FLASH");
    BG_Print(10, 25, 0, "BOOTS");

    // set up the weapon icons
    Sprite_ClearList();
    Uint16 iconY = 85;
    for (int i = 0; i < 8; i++) {
        Uint8 level = (i < 7) ? weaponLevels[i] : bootsLevel;
        Screen_StatusIcon(144 - BG_CENTERED_X, iconY, statusItemTiles[i], statusItemPalettes[i], level);
        iconY += 16;
    }

    // wait 3 seconds, or until user presses start
    for (int i = 0; i < 180; i++) {
        System_StartFrame();
        BG_Draw();
        Sprite_EndFrame();
        System_EndFrame();
        if (joyEdge & JOY_START) {
            break;
        }
    }
}

static Uint8 stagePalette[] = {
    0x0F, 0x2C, 0x2B, 0x2B,
};

void Screen_Stage(void) {
    BG_Clear();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetPalette(0, stagePalette);
    BG_Print(12, 15, 0, "STAGE %2u", stage + 1);

    // wait 3 seconds, or until user presses start
    for (int i = 0; i < 180; i++) {
        System_StartFrame();
        BG_Draw();
        System_EndFrame();
        if (joyEdge & JOY_START) {
            break;
        }
    }
}

void Screen_GameOver(void) {
    BG_Clear();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetPalette(0, stagePalette);
    BG_Print(12, 15, 0, "GAME OVER");

    // wait 4 seconds
    for (int i = 0; i < 240; i++) {
        System_StartFrame();
        BG_Draw();
        System_EndFrame();
    }
}

static Uint8 keywordPalette[] = {
    0x0F, 0x25, 0x29, 0x2C,
};

static Uint16 keywordTiles[] = {
//  x   y   tiles
    12, 10, 0x729, 0x72a, 0x00,
    11, 11, 0x73a, 0x73b, 0x73c, 0x73d, 0x00,
    12, 12, 0x72b, 0x72c, 0x00,
    14, 15, 0x73e, 0x73f, 0x75c, 0x75d, 0x75e, 0x75f, 0x00,
};

void Screen_Keyword(void) {
    BG_Clear();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetPalette(0, keywordPalette);
    Sound_Reset();
    Sound_Play(MUS_CLEAR);
    // --- show "THE KEYWORD IS" for 2.5 seconds ---
    BG_Print(9, 14, 0, "THE KEYWORD IS");
    for (int i = 0; i < 150; i++) {
        System_StartFrame();
        BG_Draw();
        System_EndFrame();
    }
    // --- show the keyword (neko dayo~) for 1 second ---
    BG_Clear();
    BG_SetTile(12, 10, 0x29, 0);
    // draw each word of the keyword graphics
    Uint16 *row = keywordTiles;
    for (int i = 0; i < 4; i++) {
        row = Screen_WriteTiles(row);
    }
    for (int i = 0; i < 60; i++) {
        System_StartFrame();
        BG_Draw();
        System_EndFrame();
    }
}
