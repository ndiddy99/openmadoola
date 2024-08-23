/* options.c: Options screen
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
#include "db.h"
#include "game.h"
#include "graphics.h"
#include "highscore.h"
#include "input.h"
#include "joy.h"
#include "menu.h"
#include "options.h"
#include "platform.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"

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

#define KEYBOARD_CONTROLS 0
#define GAMEPAD_CONTROLS 1
static int mapType;

void Options_Map(void) {
    BG_Clear();
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
                if (mapType == KEYBOARD_CONTROLS) {
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

void controlsDraw(void) {
    if (mapType == KEYBOARD_CONTROLS) {
        BG_Print(8, 2, 0, "Keyboard Controls");
    }
    else {
        BG_Print(8, 2, 0, "Gamepad Controls");
    }
    for (int i = 0; i < ARRAY_LEN(buttonNames); i++) {
        if (mapType == KEYBOARD_CONTROLS) {
            BG_Print(6, (i * 2) + 6, 0, "%-10s %s", buttonNames[i].name,
                     Joy_StrKey(buttonNames[i].button));
        }
        else {
            BG_Print(6, (i * 2) + 6, 0, "%-10s %s", buttonNames[i].name,
                     Joy_StrGamepad(buttonNames[i].button));
        }
    }
}

static MenuItem controlsItems[] = {
    MENU_LINK("Map", Options_Map),
    MENU_ABORT("Back", 0),
};

static char *boolOptions[] = {"OFF", "ON"};

static int fullscreenCB(int num) {
    num &= 1;
    return Platform_SetFullscreen(num);
}

static int ntscCB(int num) {
    num &= 1;
    return Platform_SetNTSC(num);
}

static void keyboardLink(void) {
    mapType = KEYBOARD_CONTROLS;
    Menu_Run(12, 24, 2, controlsItems, ARRAY_LEN(controlsItems), controlsDraw);
}

static void gamepadLink(void) {
    mapType = GAMEPAD_CONTROLS;
    Menu_Run(12, 24, 2, controlsItems, ARRAY_LEN(controlsItems), controlsDraw);
}

static char *gameTypeOptions[] = {"Original", "Plus", "Arcade"};

static int gameTypeInit(void) { return gameType; }

static int gameTypeCB(int num) {
    if (num < 0) { num = ARRAY_LEN(gameTypeOptions) - 1; }
    if (num >= ARRAY_LEN(gameTypeOptions)) { num = 0; }
    gameType = num;
    DB_Set("gametype", &gameType, 1);
    DB_Save();
    return num;
}

static MenuItem highScoreItems[] = {
    MENU_ABORT("Reset", 1),
    MENU_ABORT("Back", 0),
};

static void highScoreResetDraw(void) {
    BG_Print(7, 2, 0, "Reset High Scores");
    BG_Print(4, 6, 0, "Are you sure you want to\n\nreset the arcade mode\n\nhigh score table?");
}

static void highScoreResetLink(void) {
    if (Menu_Run(12, 14, 2, highScoreItems, ARRAY_LEN(highScoreItems), highScoreResetDraw)) {
        HighScore_ResetScores();
    }
}

static MenuItem optionsItems[] = {
    MENU_LIST("Fullscreen", boolOptions, Platform_GetFullscreen, fullscreenCB),
    MENU_NUM("Window scale", Platform_GetVideoScale, Platform_SetVideoScale, 1),
    MENU_LIST("NTSC filter", boolOptions, Platform_GetNTSC, ntscCB),
    MENU_NUM("Volume", Sound_GetVolume, Sound_SetVolume, 5),
    MENU_LINK("Keyboard controls", keyboardLink),
    MENU_LINK("Gamepad controls", gamepadLink),
    MENU_LIST("Game type", gameTypeOptions, gameTypeInit, gameTypeCB),
    MENU_LINK("Reset high scores", highScoreResetLink),
    MENU_ABORT("Back", 0),
};

void Options_Draw(void) {
    BG_Print(12, 2, 0, "Options");
}

void Options_Run(void) {
    BG_SetAllPalettes(palette);
    Sprite_SetAllPalettes(palette + 16);
    Menu_Run(6, 6, 2, optionsItems, ARRAY_LEN(optionsItems), Options_Draw);
}
