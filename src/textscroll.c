/* textscroll.c: Text scroll handler
 * Copyright (c) 2023 Nathan Misner
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
#include "joy.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "textscroll.h"

#define TEXT_BASE 0x800

// bg scroll position
static Uint16 yScroll;
// row to write text to
static Uint16 row;
// whether pressing start will end the scroll
static Uint8 canSkip;


static char *TextScroll_PrintLine(char *text) {
    Uint16 cursor = 0;
    while (*text && (*text != '\n')) {
        BG_SetTile(cursor++, row, 0, (Uint8)(*text) + TEXT_BASE);
        text++;
    }
    // advance past the newline
    if (*text == '\n') {
        text++;
        row++;
        row %= BG_HEIGHT;
    }
    return text;
}

static Uint16 *TextScroll_PrintTiles(Uint16 *tiles) {
    // 1 = blank line
    if (*tiles != 1) {
        Uint16 cursor = 0;
        while (*tiles) {
            BG_SetTile(cursor++, row, 0, *tiles);
            tiles++;
        }
    }
    row++;
    row %= BG_HEIGHT;
    // advance past the 0 "end of line" byte
    tiles++;
    return tiles;
}

static int TextScroll_WaitFrames(int frames) {
    for (int i = 0; i < frames; i++) {
        System_StartFrame();
        BG_Draw();
        Sprite_EndFrame();
        System_EndFrame();
        if (canSkip && (joyEdge & JOY_START)) { return 1; }
    }
    return 0;
}

static int TextScroll_Down4(void (*func)(void)) {
    for (int i = 0; i < 4; i++) {
        BG_Scroll(BG_CENTERED_X, ++yScroll);
        if (func) {
            func();
        }
        if (TextScroll_WaitFrames(3)) { return 1; }
    }
    return 0;
}

static void TextScroll_EraseOffscreenText(void) {
    Uint16 eraseRow = (row - 34) % BG_HEIGHT;
    for (int i = 0; i < 32; i++) {
        BG_SetTile(i, eraseRow, 0, ' ' + TEXT_BASE);
    }
}

void TextScroll_Init(int skippable) {
    yScroll = 0;
    row = 32;
    canSkip = skippable;
}

int TextScroll_DispStr(char *text, void (*func)(void)) {
    while (*text) {
        text = TextScroll_PrintLine(text);
        if (TextScroll_Down4(func)) { return 1; }
        TextScroll_EraseOffscreenText();
        if (TextScroll_Down4(func)) { return 1; }
    }
    return 0;
}

int TextScroll_DispTiles(Uint16 *tiles, void (*func)(void)) {
    while (*tiles) {
        tiles = TextScroll_PrintTiles(tiles);
        if (TextScroll_Down4(func)) { return 1; }
        TextScroll_EraseOffscreenText();
        if (TextScroll_Down4(func)) { return 1; }
    }
    return 0;
}
