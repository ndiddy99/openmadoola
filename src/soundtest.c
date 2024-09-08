/* soundtest.c: Sound test screen
 * Copyright (c) 2024 Nathan Misner
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
#include <string.h>

#include "bg.h"
#include "constants.h"
#include "joy.h"
#include "mainmenu.h"
#include "menu.h"
#include "mml.h"
#include "sound.h"
#include "soundtest.h"
#include "sprite.h"
#include "system.h"
#include "task.h"

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

static int soundNum;

static int getSoundNum(void) {
    return soundNum;
}

static int setSoundNum(int sound) {
    if (sound < 0) { sound = NUM_SOUNDS - 1; }
    else if (sound >= NUM_SOUNDS) { sound = 0; }
    soundNum = sound;
    return soundNum;
}

static void playSound(void) {
    Sound_Reset();
    Sound_Play(soundNum);
}

static MenuItem items[] = {
    MENU_NUMSET("Sound", getSoundNum, setSoundNum, 1, playSound),
    MENU_TASK("Back", MainMenu_Run),
};

static void SoundTest_Draw(void) {
    BG_Print(11, 2, 0, "Sound Test");
    BG_Print(3, 13, 0, "%s", Sound_GetDebugText(soundNum));
}

void SoundTest_Run(void) {
    BG_SetAllPalettes(palette);
    Sprite_SetAllPalettes(palette + 16);
    soundNum = 0;
    Menu_Run(11, 7, 2, items, ARRAY_LEN(items), SoundTest_Draw);
    Sound_Reset();
}

void SoundTest_RunStandalone(char *mmlPath) {
    if (MML_Compile(mmlPath, &sounds[0])) {
        BG_Clear();
        BG_SetAllPalettes(palette);
        BG_Print(8, 2, 0, "OpenMadoola MML");
        BG_Print(3, 6, 0, "Now playing:");
#ifdef OM_WINDOWS
        // Windows lets the user use either all forward slashes, all
        // backslashes, or a mixture of the two as path separators. The
        // simplest way to handle this situation is to replace the last
        // backslash in the string with a forward slash.
        char *backslash = strrchr(mmlPath, '\\');
        if (backslash) { *backslash = '/'; }
#endif
        char *mmlFilename;
        if ((mmlFilename = strrchr(mmlPath, '/'))) {
            mmlFilename++;
        }
        else {
            mmlFilename = mmlPath;
        }
        BG_Print(3, 8, 0, "%s", mmlFilename);
        Sound_Reset();
        Sound_Play(0);
        while (1) {
            BG_Print(3, 13, 0, "%s", Sound_GetDebugText(soundNum));
            BG_Display();
            Task_Yield();
            if (joyEdge & (JOY_A | JOY_B | JOY_SELECT | JOY_START)) {
                Sound_Reset();
                Sound_Play(0);
            }
        }
    }
}
