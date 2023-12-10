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

static void Options_InputCallback(int button) {
    last = button;
}

void Options_Map(int type) {
    BG_Fill(0x7fc, 0);
    Input_SetOnPressFunc(Options_InputCallback);
    for (int i = 0; i < ARRAY_LEN(buttonNames); i++) {
        while (1) {
            System_StartFrame();
            Sprite_ClearList();
            BG_Print(2, (i * 2) + 2, 0, "Press key for %s", buttonNames[i].name);
            BG_Draw();
            System_EndFrame();
            if (last != INPUT_INVALID) {
                BG_Print(24, (i * 2) + 2, 0, "%s", Input_ButtonName(last));
                if (type == KEYBOARD_CONTROLS) {
                    Joy_MapKey(last, buttonNames[i].button);
                }
                else {
                    Joy_MapGamepad(last, buttonNames[i].button);
                }
                last = INPUT_INVALID;
                break;
            }
        }
    }
    Input_SetOnPressFunc(NULL);
    // save key mappings to disk
    Joy_SaveMappings();
    // wait a few frames so we don't accidentally re-trigger the map keys function
    for (int i = 0; i < 3; i++) {
        System_StartFrame();
        BG_Draw();
        System_EndFrame();
    }
}

void Options_Controls(int type) {
    int cursor = 0;

    while (1) {
        System_StartFrame();
        Sprite_ClearList();

        BG_Fill(0x7fc, 0);
        if (type == KEYBOARD_CONTROLS) {
            BG_Print(8, 2, 0, "Keyboard Controls");
        }
        else {
            BG_Print(8, 2, 0, "Gamepad Controls");
        }
        for (int i = 0; i < ARRAY_LEN(buttonNames); i++) {
            if (type == KEYBOARD_CONTROLS) {
                BG_Print(6, (i * 2) + 6, 0, "%-10s %s", buttonNames[i].name,
                         Joy_StrKey(buttonNames[i].button));
            }
            else {
                BG_Print(6, (i * 2) + 6, 0, "%-10s %s", buttonNames[i].name,
                         Joy_StrGamepad(buttonNames[i].button));
            }
        }

        Menu_Begin(&cursor);
        if (Menu_Item(12, 24, cursor, "Map")) {
            if (joyEdge & (JOY_A | JOY_START)) {
                Options_Map(type);
            }
        }
        if (Menu_Item(12, 26, cursor, "Back")) {
            if (joyEdge & (JOY_A | JOY_START)) {
                return;
            }
        }
        Menu_End(&cursor);
        BG_Draw();
        Sprite_EndFrame();
        System_EndFrame();
    }
}

void Options_Run(void) {
    int cursor = 0;

    Sound_Reset();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetAllPalettes(palette);
    Sprite_SetAllPalettes(palette + 16);

    while (1) {
        System_StartFrame();
        Sprite_ClearList();
        BG_Fill(0x7fc, 0);

        if (joyEdge & ~JOY_START) {
            Sound_Play(SFX_MENU);
        }

        BG_Print(12, 2, 0, "Options");
        Menu_Begin(&cursor);
        if (Menu_Item(4, 4, cursor, "Fullscreen - %s - ", Platform_GetFullscreen() ? "ON" : "OFF")) {
            if (joyEdge & (JOY_A | JOY_LEFT | JOY_RIGHT)) {
                Platform_SetFullscreen(!Platform_GetFullscreen());
            }
        }
        if (Menu_Item(4, 6, cursor, "Window scale  - %d -", Platform_GetVideoScale())) {
            if (joyEdge & JOY_LEFT) {
                Platform_SetVideoScale(Platform_GetVideoScale() - 1);
            }
            else if (joyEdge & JOY_RIGHT) {
                Platform_SetVideoScale(Platform_GetVideoScale() + 1);
            }
        }

        if (Menu_Item(4, 8, cursor, "NTSC filter - %s - ", Platform_GetNTSC() ? "ON" : "OFF")) {
            if (joyEdge & (JOY_A | JOY_LEFT | JOY_RIGHT)) {
                Platform_SetNTSC(!Platform_GetNTSC());
            }
        }

        if (Menu_Item(4, 10, cursor, "NTSC noise  - %d -", Platform_GetNTSCNoise())) {
            if (joyEdge & JOY_LEFT) {
                Platform_SetNTSCNoise(Platform_GetNTSCNoise() - 1);
            }
            else if (joyEdge & JOY_RIGHT) {
                Platform_SetNTSCNoise(Platform_GetNTSCNoise() + 1);
            }
        }

        if (Menu_Item(4, 12, cursor, "NTSC scanlines - %s - ", Platform_GetNTSCScanlines() ? "ON" : "OFF")) {
            if (joyEdge & (JOY_A | JOY_LEFT | JOY_RIGHT)) {
                Platform_SetNTSCScanlines(!Platform_GetNTSCScanlines());
            }
        }

        if (Menu_Item(4, 14, cursor, "Keyboard controls")) {
            if (joyEdge & (JOY_A | JOY_START)) {
                Options_Controls(KEYBOARD_CONTROLS);
            }
        }
        if (Menu_Item(4, 16, cursor, "Gamepad controls")) {
            if (joyEdge & (JOY_A | JOY_START)) {
                Options_Controls(GAMEPAD_CONTROLS);
            }
        }
        if (Menu_Item(4, 18, cursor, "Game type - %s -", gameType == GAME_TYPE_PLUS ? "Plus" : "Original")) {
            if (joyEdge & (JOY_A | JOY_LEFT | JOY_RIGHT)) {
                // make sure to change this to a proper cycle if you add more game types
                gameType ^= 1;
            }
        }
        if (Menu_Item(4, 20, cursor, "Back")) {
            if (joyEdge & (JOY_A | JOY_START)) {
                break;
            }
        }

        Menu_End(&cursor);
        BG_Draw();
        Sprite_EndFrame();
        System_EndFrame();
    }
}
