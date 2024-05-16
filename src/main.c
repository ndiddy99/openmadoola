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

#include <string.h>

#include "game.h"
#include "soundtest.h"
#include "system.h"

int main(int argc, char **argv) {
    if (!System_Init()) { return -1; }

    if (argc == 3 && (strcmp(argv[1], "-p") == 0)) {
        SoundTest_RunStandalone(argv[2]);
    }
    else {
        Game_Run();
    }
    return 0;
}
