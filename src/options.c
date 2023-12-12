/* options.c: Options screen
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

#include "bg.h"
#include "constants.h"
#include "game.h"
#include "graphics.h"
#include "input.h"
#include "joy.h"
#include "menu.h"
#include "options.h"
#include "platform.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"

#define KEYBOARD_CONTROLS 0
#define GAMEPAD_CONTROLS 1

static Uint8 palette[] = {
        0x0F, 0x36, 0x26, 0x16,
        0x0F, 0x11, 0x21, 0x31,
        0x0F, 0x12, 0x22, 0x32,
        0x0F, 0x13, 0x23, 0x33,
        0x0F, 0x36, 0x26, 0x16,
        0x0F, 0x11, 0x21, 0x31,
        0x0F, 0x12, 0x22, 0x32,
        0x0F, 0x13, 0x23, 0x33,
};

struct {
    char *name;
    Uint32 button;
} buttonNames[] = {
    {"Up", JOY_UP},
    {"Down", JOY_DOWN},
    {"Left", JOY_LEFT},
    {"Right", JOY_RIGHT},
    {"B", JOY_B},
    {"A", JOY_A},
    {"Select", JOY_SELECT},
    {"Start", JOY_START},
};

static int last = INPUT_INVALID;
static int redraw = 0;
static int exitMenu = 0;

static void Options_InputCallback(int button) {
    last = button;
}

void Options_Map(int type) {
//    BG_Clear();
//    Input_SetOnPressFunc(Options_InputCallback);
//    for (int i = 0; i < ARRAY_LEN(buttonNames); i++) {
//        while (1) {
//            System_StartFrame();
//            Sprite_ClearList();
//            BG_Print(2, (i * 2) + 2, 0, "Press key for %s", buttonNames[i].name);
//            BG_Draw();
//            System_EndFrame();
//            if (last != INPUT_INVALID) {
//                BG_Print(24, (i * 2) + 2, 0, "%s", Input_ButtonName(last));
//                if (type == KEYBOARD_CONTROLS) {
//                    Joy_MapKey(last, buttonNames[i].button);
//                }
//                else {
//                    Joy_MapGamepad(last, buttonNames[i].button);
//                }
//                last = INPUT_INVALID;
//                break;
//            }
//        }
//    }
//    Input_SetOnPressFunc(NULL);
//    // save key mappings to disk
//    Joy_SaveMappings();
//    // wait a few frames so we don't accidentally re-trigger the map keys function
//    for (int i = 0; i < 3; i++) {
//        System_StartFrame();
//        BG_Draw();
//        System_EndFrame();
//    }
}

void Options_Controls(int type) {
//    int cursor = 0;
//
//    while (1) {
//        System_StartFrame();
//        Sprite_ClearList();
//
//        BG_Clear();
//        if (type == KEYBOARD_CONTROLS) {
//            BG_Print(8, 2, 0, "Keyboard Controls");
//        }
//        else {
//            BG_Print(8, 2, 0, "Gamepad Controls");
//        }
//        for (int i = 0; i < ARRAY_LEN(buttonNames); i++) {
//            if (type == KEYBOARD_CONTROLS) {
//                BG_Print(6, (i * 2) + 6, 0, "%-10s %s", buttonNames[i].name,
//                         Joy_StrKey(buttonNames[i].button));
//            }
//            else {
//                BG_Print(6, (i * 2) + 6, 0, "%-10s %s", buttonNames[i].name,
//                         Joy_StrGamepad(buttonNames[i].button));
//            }
//        }
//
//        Menu_Begin(&cursor);
//        if (Menu_Item(12, 24, cursor, "Map")) {
//            if (joyEdge & (JOY_A | JOY_START)) {
//                Options_Map(type);
//            }
//        }
//        if (Menu_Item(12, 26, cursor, "Back")) {
//            if (joyEdge & (JOY_A | JOY_START)) {
//                return;
//            }
//        }
//        Menu_End(&cursor);
//        BG_Draw();
//        Sprite_EndFrame();
//        System_EndFrame();
//    }
}

static char *fullscreenOptions[] = {"OFF", "ON"};

static int fullscreenCB(int num) {
    num &= 1;
    return Platform_SetFullscreen(num);
}

static int scaleCB(int num) {
    return Platform_SetVideoScale(num);
}

static int keyboardControlsCB(int num) {
    Options_Controls(KEYBOARD_CONTROLS);
    redraw = 1;
    return 0;
}

static int gamepadControlsCB(int num) {
    Options_Controls(GAMEPAD_CONTROLS);
    redraw = 1;
    return 0;
}

static char *gameTypeOptions[] = {"Original", "Plus"};

static int gameTypeCB(int num) {
    num &= 1;
    return num;
}

static int backCB(int num) {
    exitMenu = 1;
    return 0;
}

void Options_Run(void) {
    int cursor = 0;

start:
    Sound_Reset();
    BG_Clear();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetAllPalettes(palette);
    Sprite_SetAllPalettes(palette + 16);

    BG_Print(12, 2, 0, "Options");
    Menu_Init(6, 6);
    Menu_AddList("Fullscreen", fullscreenOptions, Platform_GetFullscreen(), fullscreenCB);
    Menu_AddNum("Window scale", Platform_GetVideoScale(), scaleCB);
    Menu_AddLink("Keyboard controls", keyboardControlsCB);
    Menu_AddLink("Gamepad controls", gamepadControlsCB);
    Menu_AddList("Game type", gameTypeOptions, gameType, gameTypeCB);
    Menu_AddLink("Back", backCB);

    while (1) {
        System_StartFrame();
        Sprite_ClearList();

        Menu_Run(2);

        BG_Draw();
        Sprite_EndFrame();
        System_EndFrame();

        if (redraw) { goto start; }
        if (exitMenu) { return; }
    }
}
