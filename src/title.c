/* title.c: Title screen
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

#include "bg.h"
#include "constants.h"
#include "game.h"
#include "joy.h"
#include "palette.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "textscroll.h"
#include "title.h"

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
};

static Uint8 titlePaletteArcade[] = {
    0x0F, 0x28, 0x26, 0x16,
    0x0F, 0x21, 0x11, 0x28,
    0x0F, 0x20, 0x22, 0x32,
    0x0F, 0x0F, 0x0F, 0x0F,
};

static Uint8 titleCyclePals1[] = {
    0x0F, 0x21, 0x11, 0x28,
    0x0F, 0x22, 0x12, 0x21,
    0x0F, 0x26, 0x16, 0x22,
    0x0F, 0x2a, 0x1a, 0x26,
    0x0F, 0x20, 0x10, 0x2a,
    0x0F, 0x24, 0x14, 0x20,
    0x0F, 0x28, 0x18, 0x24,
};

static Uint8 titleCyclePals3[] = {
    0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0x06, 0x06, 0x06,
    0x0F, 0x16, 0x16, 0x16,
    0x0F, 0x26, 0x26, 0x26,
    0x0F, 0x36, 0x36, 0x36,
    0x0F, 0x26, 0x26, 0x26,
    0x0F, 0x16, 0x16, 0x16,
    0x0F, 0x06, 0x06, 0x06,
};

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

static Uint8 highScorePalette[] = {
    0x0F, 0x26, 0x20, 0x20,
};

static int musicPlaying = 0;

static void Title_DrawMadoolaGraphic(int x, int y) {
    for (int yCursor = 0; yCursor < 6; yCursor++) {
        for (int xCursor = 0; xCursor < 20; xCursor++) {
            BG_SetTile(x + xCursor, y + yCursor, madoolaTiles[yCursor * 20 + xCursor], 1);
        }
    }
}

static void Title_DrawCopyrightText(int x, int y) {
    BG_Print(x, y + 0, 0, "OpenMadoola " OPENMADOOLA_VERSION);
    BG_Print(x, y + 2, 0, "infochunk.com/madoola");
    BG_Print(x, y + 4, 0, "Assets @ 1986 SUNSOFT");
}

static int Title_ScreenOriginal(void) {
    BG_Clear();
    BG_SetAllPalettes(titlePalette);
    BG_Print(6, 6, 1, "THE WING OF");
    Title_DrawMadoolaGraphic(6, 8);
    Title_DrawCopyrightText(6, 19);
    Sound_Play(MUS_TITLE);
    BG_Scroll(BG_CENTERED_X, 0);
    while (1) {
        System_StartFrame();
        BG_Draw();
        System_EndFrame();
        if (joyEdge & JOY_START) { return 1; }
    }
}

static int Title_PlusInit(void) {
    BG_Clear();
    BG_SetAllPalettes(titlePalette);
    BG_Print(6, 6, 1, "The Wings of");
    Title_DrawMadoolaGraphic(6, 8);
    Title_DrawCopyrightText(6, 19);
    return 0;
}

static int Title_ArcadeInit(void) {
    BG_Clear();
    BG_SetAllPalettes(titlePaletteArcade);
    BG_Print(5, 2, 0, "SCORE");
    BG_Print(5, 4, 2, "00000000");
    BG_Print(19, 2, 0, "HIGH-SCORE");
    BG_Print(19, 4, 2, "00000000");
    BG_Print(7, 8, 1, "The Wings of");
    Title_DrawMadoolaGraphic(7, 10);
    BG_Print(6, 18, 3, "- PRESS START BUTTON -");
    Title_DrawCopyrightText(6, 20);
    return 0;
}

static int Title_AnimateIn(void) {
    int scroll = 255;
    Uint8 scrollDir = 0;

    while (scroll >= 2) {
        System_StartFrame();
        scroll -= 2;
        scrollDir ^= 1;
        if (scrollDir) {
            BG_Scroll(BG_CENTERED_X + scroll, 0);
        }
        else {
            BG_Scroll(BG_CENTERED_X - scroll, 0);
        }
        BG_Draw();
        System_EndFrame();
        if (joyEdge & JOY_START) { return 1; }
    }
    BG_Scroll(BG_CENTERED_X, 0);
    return 0;
}


static int Title_ScreenPlus(void) {
    int frames = 0;
    if (!musicPlaying) {
        musicPlaying = 1;
        Sound_Play(MUS_TITLE);
    }
    while (++frames < 900) {
        System_StartFrame();
        BG_Draw();
        System_EndFrame();
        if (joyEdge & JOY_START) { return 1; }
    }
    return 0;
}

static int Title_ScreenArcade(void) {
    int frames = 0;
    while (++frames < 200) {
        System_StartFrame();
        int madoolaPaletteNum = (frames / 2) % 7;
        BG_SetPalette(1, titleCyclePals1 + madoolaPaletteNum * 4);
        // NOTE: this is most likely supposed to be "frames / 2" but I think
        // frames / 8 looks a LOT better
        int startPaletteNum = (frames / 8) % 8;
        BG_SetPalette(3, titleCyclePals3 + startPaletteNum * 4);
        BG_Draw();
        System_EndFrame();
        if (joyEdge & JOY_START) { return 1; }
    }
    BG_SetPalette(3, titleCyclePals3);
    return 0;
}

static int Title_AnimateOut(void) {
    int scroll = 0;
    Uint8 scrollDir = 0;
    
    while (scroll < 254) {
        System_StartFrame();
        scroll += 2;
        scrollDir ^= 1;
        if (scrollDir) {
            BG_Scroll(BG_CENTERED_X + scroll, 0);
        }
        else {
            BG_Scroll(BG_CENTERED_X - scroll, 0);
        }
        BG_Draw();
        System_EndFrame();
        if (joyEdge & JOY_START) { return 1; }
    }
    return 0;
}

static int Title_TextScroll(void) {
    BG_Clear();
    BG_SetPalette(0, introTextPalette);
    TextScroll_Init(1);
    return TextScroll_DispStr(introText, NULL);
}

static int Title_HighScoresInit(void) {
    BG_Clear();
    BG_SetPalette(0, highScorePalette);
    BG_Print(6,  6, 0, "RANK  NAME    SCORE");
    BG_Print(6,  9, 0, "TOP   MSSAN   00400000");
    BG_Print(6, 11, 0, "2ND   OIYTA   00350000");
    BG_Print(6, 13, 0, "3RD   RMUSK   00300000");
    BG_Print(6, 15, 0, "4TH   OOGUA   00250000");
    BG_Print(6, 17, 0, "5TH   TMISG   00200000");
    BG_Print(6, 19, 0, "6TH   TUUHA   00150000");
    BG_Print(6, 21, 0, "7TH   ARRIW   00100000");
    BG_Print(6, 23, 0, "8TH   RAAIA   00050000");
    return 0;
}

static int Title_HighScores(void) {
    int frames = 0;
    while (++frames < 200) {
        System_StartFrame();
        BG_Draw();
        System_EndFrame();
        if (joyEdge & JOY_START) { return 1; }
    }
    return 0;
}

typedef int (*SequenceItem)(void);

static SequenceItem originalSequence[] = {
    Title_ScreenOriginal,
};
static SequenceItem plusSequence[] = {
    Title_PlusInit,
    Title_AnimateIn,
    Title_ScreenPlus,
    Title_AnimateOut,
    Title_TextScroll,
};
static SequenceItem arcadeSequence[] = {
    Title_ArcadeInit,
    Title_AnimateIn,
    Title_ScreenArcade,
    Title_AnimateOut,
    Title_TextScroll,
    Title_ArcadeInit,
    Title_AnimateIn,
    Title_ScreenArcade,
    Title_AnimateOut,
    Title_HighScoresInit,
    Title_AnimateIn,
    Title_HighScores,
    Title_AnimateOut,
    // attract mode gameplay goes here
};

void Title_DoSequence(SequenceItem *sequence, int len) {
    int cursor = 0;
    while (!(sequence[cursor]())) {
        cursor++;
        if (cursor >= len) { cursor = 0; }
    }
}

void Title_Run(void) {
    flashTimer = 0;
    musicPlaying = 0;
    BG_Clear();
    Sprite_ClearList();
    Sound_Reset();

    switch (gameType) {
    case GAME_TYPE_ORIGINAL:
        Title_DoSequence(originalSequence, ARRAY_LEN(originalSequence));
        break;

    case GAME_TYPE_PLUS:
        Title_DoSequence(plusSequence, ARRAY_LEN(plusSequence));
        break;

    case GAME_TYPE_ARCADE:
        Title_DoSequence(arcadeSequence, ARRAY_LEN(arcadeSequence));
        break;
    }
}
