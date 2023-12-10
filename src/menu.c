/* menu.c: Menu display code
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

#include <stdarg.h>
#include <string.h>

#include "bg.h"
#include "constants.h"
#include "joy.h"
#include "menu.h"
#include "sprite.h"

static int currItem;
static Sprite cursorSpr;

void Menu_Begin(int *cursor) {
    currItem = 0;
    if (joyEdge & JOY_UP) {
        (*cursor)--;
    }
    if (joyEdge & (JOY_DOWN | JOY_SELECT)) {
        (*cursor)++;
    }
}

int Menu_Item(Uint16 x, Uint16 y, int cursor, char *text, ...) {
    va_list args;
    va_start(args, text);
    BG_VPrint(x, y, 0, text, args);
    va_end(args);
    if (currItem++ == cursor) {
        cursorSpr.x = (x - 1) * TILE_WIDTH;
        cursorSpr.y = (y * TILE_HEIGHT) - 5;
        cursorSpr.size = SPRITE_8X16;
        cursorSpr.tile = 0xee;
        cursorSpr.palette = 0;
        cursorSpr.mirror = 0;
        Sprite_Draw(&cursorSpr, NULL);
        return 1;
    }
    return 0;
}

void Menu_End(int *cursor) {
    int maxItem = currItem - 1;
    if (*cursor < 0) { *cursor = maxItem; }
    else if (*cursor > maxItem) { *cursor = 0; }
}