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
#include "sound.h"
#include "sprite.h"

static int currItem;
static Sprite cursorSpr;

typedef enum {
    MENU_ITEM_NONE,
    MENU_ITEM_LINK,
    MENU_ITEM_LIST,
    MENU_ITEM_NUM,
} MenuItemType;

typedef struct {
    MenuItemType type;
    char *text;
    int num;
    // --- for list items ---
    int numOptions;
    char **options;
    int (*cb)(int num);
} MenuItem;

#define MAX_MENU_ITEMS 8
static MenuItem items[MAX_MENU_ITEMS];
static int cursor;
static int itemCount;
static Uint16 menuX;
static Uint16 menuY;

void Menu_Init(Uint16 x, Uint16 y) {
    for (int i = 0; i < MAX_MENU_ITEMS; i++) {
        items[i].type = MENU_ITEM_NONE;
    }

    cursor = 0;
    itemCount = 0;
    menuX = x;
    menuY = y;
}

void Menu_AddLink(char *text, int num, int (*cb)(int num)) {
    if (itemCount >= MAX_MENU_ITEMS) { return; }
    items[itemCount].type = MENU_ITEM_LINK;
    items[itemCount].text = text;
    items[itemCount].num = num;
    items[itemCount].cb = cb;
    itemCount++;
}

void Menu_AddList(char *text, char **options, int startVal, int (*cb)(int num)) {
    if (itemCount >= MAX_MENU_ITEMS) { return; }
    items[itemCount].type = MENU_ITEM_LIST;
    items[itemCount].text = text;
    items[itemCount].num = startVal;
    items[itemCount].options = options;
    items[itemCount].cb = cb;
    itemCount++;
}

void Menu_AddNum(char *text, int startVal, int (*cb)(int num)) {
    if (itemCount >= MAX_MENU_ITEMS) { return; }
    items[itemCount].type = MENU_ITEM_NUM;
    items[itemCount].text = text;
    items[itemCount].num = startVal;
    items[itemCount].cb = cb;
    itemCount++;
}

void Menu_Run(int spacing) {
    if (joyEdge & JOY_UP) {
        Sound_Play(SFX_MENU);
        cursor--;
        if (cursor < 0) { cursor = itemCount - 1; }
    }
    if (joyEdge & (JOY_DOWN | JOY_SELECT)) {
        Sound_Play(SFX_MENU);
        cursor++;
        if (cursor >= itemCount) { cursor = 0; }
    }
    cursorSpr.x = (menuX - 1) * TILE_WIDTH;
    cursorSpr.y = ((cursor * spacing + menuY) * TILE_HEIGHT) - 5;
    cursorSpr.size = SPRITE_8X16;
    cursorSpr.tile = 0xee;
    cursorSpr.palette = 0;
    cursorSpr.mirror = 0;
    Sprite_Draw(&cursorSpr, NULL);

    int currY = menuY;
    for (int i = 0; i < itemCount; i++) {
        switch(items[i].type) {
        case MENU_ITEM_LINK:
            if (i == cursor) {
                if (joyEdge & (JOY_A | JOY_START)) {
                    items[i].cb(items[i].num);
                }
            }
            BG_Print(menuX, currY, 0, items[i].text);
            break;

        case MENU_ITEM_LIST:
            if (i == cursor) {
                if (joyEdge & JOY_LEFT) {
                    Sound_Play(SFX_MENU);
                    items[i].num = items[i].cb(--items[i].num);
                }
                if (joyEdge & JOY_RIGHT) {
                    Sound_Play(SFX_MENU);
                    items[i].num = items[i].cb(++items[i].num);
                }
            }
            BG_Print(menuX, currY, 0, "%s - %s -", items[i].text, items[i].options[items[i].num]);
            break;

        case MENU_ITEM_NUM:
            if (i == cursor) {
                if (joyEdge & JOY_LEFT) {
                    Sound_Play(SFX_MENU);
                    items[i].num = items[i].cb(--items[i].num);
                }
                if (joyEdge & JOY_RIGHT) {
                    Sound_Play(SFX_MENU);
                    items[i].num = items[i].cb(++items[i].num);
                }
            }
            BG_Print(menuX, currY, 0, "%s - %d -", items[i].text, items[i].num);
            break;
        }
        currY += spacing;
    }
}