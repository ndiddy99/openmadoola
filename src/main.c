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
        SoundTest_RunStandalone(argv[2]);
        return 0;
    }

    if ((argc == 3) && (strcmp(argv[1], "-d") == 0)) {
        Demo_Playback(argv[2]);
        return 0;
    }

    if ((argc == 2) && (strcmp(argv[1], "-r") == 0)) {
        recordDemos = 1;
    }
    Game_Run();
    return 0;
}
