/* menu.c: Menu display code
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
#include <string.h>

#include "bg.h"
#include "constants.h"
#include "joy.h"
#include "menu.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "task.h"

void Menu_Run(Uint16 menuX, Uint16 menuY, int spacing,  MenuItem *items, int numItems, void (*draw)(void)) {
    int cursor = 0;
    Sprite cursorSpr;

    Sound_Reset();
    BG_Scroll(BG_CENTERED_X, 0);

    // run init functions
    for (int i = 0; i < numItems; i++) {
        if (items[i].init) {
            items[i].num = items[i].init();
        }
    }

    while (1) {
        Sprite_ClearList();
        BG_Clear();

        if (draw) { draw(); }

        // cursor movement
        if (joyEdge & JOY_UP) {
            Sound_Play(SFX_MENU);
            cursor--;
            if (cursor < 0) { cursor = numItems - 1; }
        }
        if (joyEdge & (JOY_DOWN | JOY_SELECT)) {
            Sound_Play(SFX_MENU);
            cursor++;
            if (cursor >= numItems) { cursor = 0; }
        }
        cursorSpr.x = ((menuX - 1) * TILE_WIDTH) - BG_CENTERED_X;
        cursorSpr.y = ((cursor * spacing + menuY) * TILE_HEIGHT) - 5;
        cursorSpr.size = SPRITE_8X16;
        cursorSpr.tile = 0xee;
        cursorSpr.palette = 0;
        cursorSpr.mirror = 0;
        Sprite_Draw(&cursorSpr, NULL);

        // draw each menu item
        Uint16 currY = menuY;
        for (int i = 0; i < numItems; i++) {
            switch (items[i].type) {
            case ITEM_TYPE_NONE:
                break;

            case ITEM_TYPE_TASK:
                if (i == cursor) {
                    if (joyEdge & (JOY_A | JOY_START)) {
                        Task_Switch(items[i].function);
                    }
                }
                BG_Print(menuX, currY, 0, "%s", items[i].text);
                break;

            case ITEM_TYPE_LINK:
                if (i == cursor) {
                    if (joyEdge & (JOY_A | JOY_START)) {
                        items[i].function();
                        BG_Clear();
                        Sprite_ClearList();
                    }
                }
                BG_Print(menuX, currY, 0, "%s", items[i].text);
                break;

            case ITEM_TYPE_LIST:
                if (i == cursor) {
                    if (joyEdge & JOY_LEFT) {
                        Sound_Play(SFX_MENU);
                        items[i].num = items[i].change(--items[i].num);
                    }
                    if (joyEdge & JOY_RIGHT) {
                        Sound_Play(SFX_MENU);
                        items[i].num = items[i].change(++items[i].num);
                    }
                }
                BG_Print(menuX, currY, 0, "%s - %s -", items[i].text, items[i].options[items[i].num]);
                break;

            case ITEM_TYPE_NUM:
                if (i == cursor) {
                    if (joyEdge & JOY_LEFT) {
                        Sound_Play(SFX_MENU);
                        items[i].num = items[i].change(items[i].num - items[i].step);
                    }
                    if (joyEdge & JOY_RIGHT) {
                        Sound_Play(SFX_MENU);
                        items[i].num = items[i].change(items[i].num + items[i].step);
                    }
                }
                BG_Print(menuX, currY, 0, "%s - %d -", items[i].text, items[i].num);
                break;

            case ITEM_TYPE_NUMSET:
                if (i == cursor) {
                    if (joyEdge & JOY_LEFT) {
                        Sound_Play(SFX_MENU);
                        items[i].num = items[i].change(items[i].num - items[i].step);
                    }
                    if (joyEdge & JOY_RIGHT) {
                        Sound_Play(SFX_MENU);
                        items[i].num = items[i].change(items[i].num + items[i].step);
                    }
                    if (joyEdge & (JOY_A | JOY_START)) {
                        items[i].function();
                    }
                }
                BG_Print(menuX, currY, 0, "%s - %d -", items[i].text, items[i].num);
                break;
            }
            currY += spacing;
        }

        BG_Display();
        Sprite_Display();
        Task_Yield();
    }
}
