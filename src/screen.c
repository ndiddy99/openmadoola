/* screen.c: Title, status, stage screens
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

static Uint16 madoolaTiles[] = {
    0x7FC,0x7FC,0x762,0x763,0x764,0x765,0x766,0x7FC,0x762,0x769,0x76A,0x76B,0x7FC,0x76D,0x76B,0x76D,0x76B,0x760,0x766,0x7FC,
    0x770,0x766,0x772,0x773,0x774,0x775,0x776,0x777,0x778,0x779,0x77A,0x77B,0x77C,0x77D,0x77E,0x77D,0x761,0x767,0x776,0x777,
    0x780,0x7FC,0x782,0x783,0x784,0x785,0x786,0x787,0x788,0x7FC,0x78A,0x78B,0x78C,0x78D,0x78E,0x78D,0x768,0x76C,0x786,0x76E,
    0x790,0x791,0x792,0x793,0x794,0x795,0x796,0x797,0x798,0x7FC,0x79A,0x79B,0x79C,0x79D,0x79E,0x79F,0x76F,0x795,0x796,0x771,
    0x7FC,0x7A1,0x7A2,0x7A3,0x7A4,0x7A5,0x7A6,0x7A7,0x7A8,0x7A9,0x7AA,0x7AB,0x7AC,0x7AD,0x7AE,0x7AF,0x77F,0x7FC,0x7A9,0x781,
    0x7FC,0x7B1,0x7B2,0x7B3,0x7FC,0x7FC,0x7FC,0x789,0x78F,0x799,0x7A0,0x7FC,0x7A0,0x7FC,0x789,0x7B0,0x7B4,0x7B5,0x7B6,0x77F,
};

static Uint8 titlePalette[] = {
    0x0F, 0x36, 0x26, 0x16,
    0x0F, 0x36, 0x26, 0x16,
    0x0F, 0x12, 0x22, 0x32,
    0x0F, 0x13, 0x23, 0x33,
    0x0F, 0x10, 0x20, 0x30,
    0x0F, 0x11, 0x21, 0x31,
    0x0F, 0x12, 0x22, 0x32,
    0x0F, 0x13, 0x23, 0x33,
};

static Uint8 titlePaletteArcade[] = {
    0x0F, 0x36, 0x26, 0x16,
    0x0F, 0x21, 0x11, 0x28,
    0x0F, 0x12, 0x22, 0x32,
    0x0F, 0x13, 0x23, 0x33,
    0x0F, 0x10, 0x20, 0x30,
    0x0F, 0x11, 0x21, 0x31,
    0x0F, 0x12, 0x22, 0x32,
    0x0F, 0x13, 0x23, 0x33,
};

static Uint8 titleCyclePals[] = {
    0x0F, 0x21, 0x11, 0x28,
    0x0F, 0x22, 0x12, 0x21,
    0x0F, 0x26, 0x16, 0x22,
    0x0F, 0x2a, 0x1a, 0x26,
    0x0F, 0x20, 0x10, 0x2a,
    0x0F, 0x24, 0x14, 0x20,
    0x0F, 0x28, 0x18, 0x24,
};

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

static void Screen_TitleDraw(void) {
    // draw title screen text
    if (gameType == GAME_TYPE_ORIGINAL) {
        BG_Print(6, 6, 1, "THE WING OF");
    }
    else {
        BG_Print(6, 6, 1, "The Wings of");
    }
    BG_Print(6, 19, 0, "OpenMadoola " OPENMADOOLA_VERSION);
    BG_Print(6, 21, 0, "infochunk.com/madoola");
    BG_Print(6, 23, 0, "Assets @ 1986 SUNSOFT");


    // draw "Madoola" graphic
    for (int y = 0; y < 6; y++) {
        for (int x = 0; x < 20; x++) {
            BG_SetTile(x + 6, y + 8, madoolaTiles[y * 20 + x], 1);
        }
    }
}

static Uint8 introTextPalette[] = {
    0x0F, 0x20, 0x20, 0x20,
};

static char introText[] = {
    "      This is the story of\n\n"
    "     {The Wings of Madoola},\n\n"
    "  a legend that was told about\n\n"
    "  a magic bird stature in the\n\n"
    "   land of Badam a long, long\n\n"
    "    time ago.  People of the\n\n"
    "     country, knowing that\n\n"
    "   the Wings of Madoola would\n\n"
    "   give its holder a special\n\n"
    "   magic power to conquer the\n\n"
    "     entire world, repeated\n\n"
    "   countress struggles among\n\n"
    "   them to win the strength.\n\n"
    "  After years of battles, the\n\n"
    "  land was always in the state\n\n"
    "           of chaos.\n\n"
    "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
};

void Screen_Title(void) {
    int frames = 0;
    int scroll = 255;
    int state = 0;
    int musicPlaying = 0;
    int exitFlag = 0;

    flashTimer = 0;
    BG_Clear();
    if (gameType == GAME_TYPE_ARCADE) {
        BG_SetAllPalettes(titlePaletteArcade);
    }
    else {
        BG_SetAllPalettes(titlePalette);
    }
    Sprite_ClearList();
    Sound_Reset();
    Screen_TitleDraw();

    if (gameType == GAME_TYPE_ORIGINAL) {
        Sound_Play(MUS_TITLE);
        BG_Scroll(BG_CENTERED_X, 0);
        while (!exitFlag) {
            System_StartFrame();
            BG_Draw();
            System_EndFrame();
            if (joyEdge & JOY_START) { exitFlag = 1; }
        }
    }
    else {
        while (!exitFlag) {
            System_StartFrame();
            switch (state) {
            // animate in
            case 0:
                if (scroll >= 2) {
                    scroll -= 2;
                    if (++frames & 1) {
                        BG_Scroll(BG_CENTERED_X + scroll, 0);
                    } else {
                        BG_Scroll(BG_CENTERED_X - scroll, 0);
                    }
                }
                else {
                    if (!musicPlaying && (gameType == GAME_TYPE_PLUS)) {
                        musicPlaying = 1;
                        Sound_Play(MUS_TITLE);
                    }
                    BG_Scroll(BG_CENTERED_X, 0);
                    frames = 0;
                    scroll = 0;
                    state++;
                }
                break;
            // wait a few seconds
            case 1:
                // arcade palette cycling
                if (gameType == GAME_TYPE_ARCADE) {
                    BG_SetPalette(1, titleCyclePals + (frames % 7) * 4);
                }
                if (++frames >= 900) {
                    frames = 0;
                    state++;
                }
                break;
            // animate out
            case 2:
                if (scroll < 254) {
                    scroll += 2;
                    if (++frames & 1) {
                        BG_Scroll(BG_CENTERED_X + scroll, 0);
                    } else {
                        BG_Scroll(BG_CENTERED_X - scroll, 0);
                    }
                }
                else {
                    // display intro text
                    BG_Clear();
                    System_EndFrame();
                    BG_SetPalette(0, introTextPalette);
                    TextScroll_Init(1);
                    // break out of title screen if the user skipped the intro text
                    if (TextScroll_DispStr(introText, NULL)) { exitFlag = 1; }
                    // set up scrolling in the title screen again
                    System_StartFrame();
                    frames = 0;
                    scroll = 255;
                    BG_SetPalette(0, titlePalette);
                    BG_Scroll(BG_CENTERED_X + scroll, 0);
                    Screen_TitleDraw();
                    state = 0;
                }
            }
            BG_Draw();
            System_EndFrame();
            if (joyEdge & JOY_START) { exitFlag = 1; }
        }
    }
}

static Uint8 statusPalette[] = {
    0x0B, 0x2B, 0x2B, 0x2B,
    0x0B, 0x2B, 0x2B, 0x2B,
    0x0B, 0x2B, 0x2B, 0x2B,
    0x0B, 0x2B, 0x2B, 0x2B,
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
