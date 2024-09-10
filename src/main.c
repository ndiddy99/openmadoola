/* main.c: Entry point
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

#include "constants.h"
#ifdef OM_WINDOWS
#include <io.h>
#include <stdio.h>
#include <Windows.h>
#endif

#include <string.h>

#include "demo.h"
#include "game.h"
#include "soundtest.h"
#include "system.h"
#include "task.h"
#include "title.h"
#include "weapon.h"

int main(int argc, char **argv) {
    // Windows has two types of programs, "Console" and "Windows". Console
    // programs will pop up a command line window when launched while Windows
    // programs will never output anything to the console. We don't want either
    // behavior. Instead, we want command line output when the program is
    // launched from a command line, and no output when it's launched from a GUI.
    // Every other OS has this behavior by default, but Windows requires a
    // workaround. We build the executable as a Windows program. At launch, we
    // check whether the program was launched from the command line. If it was,
    // we manually attach console output to the command prompt.
#ifdef OM_WINDOWS
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        // the console prompt gets printed when the program is launched, so
        // this makes it so we start printing on the next line rather than
        // the prompt's line
        putchar('\n');
    }
#endif

    if (!System_Init()) { return -1; }

    if ((argc == 3) && (strcmp(argv[1], "-p") == 0)) {
        SoundTest_RunStandaloneInit(argv[2]);
        Task_Init(SoundTest_RunStandaloneTask);
    }
    else if ((argc == (8 + NUM_WEAPONS)) && (strcmp(argv[1], "-r") == 0)) {
        int param = 2;
        char *filename = argv[param++];
        Uint8 type = (Uint8)atoi(argv[param++]);
        Uint8 stage = (Uint8)atoi(argv[param++]) - 1;
        Sint16 health = (Sint16)atoi(argv[param++]);
        Sint16 magic = (Sint16)atoi(argv[param++]);
        Uint8 boots = (Uint8)atoi(argv[param++]);
        Uint8 weapons[NUM_WEAPONS];
        for (int i = 0; i < NUM_WEAPONS; i++) {
            weapons[i] = (Uint8)atoi(argv[param++]);
        }
        Game_RecordDemoInit(filename, type, stage, health, magic, boots, weapons);
        Task_Init(Game_RecordDemoTask);
    }
    else {
        Task_Init(Title_Run);
    }
    System_GameLoop();
    return 0;
}
