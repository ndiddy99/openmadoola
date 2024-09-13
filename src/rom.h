/* rom.h: Reads from the game ROM file
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

#pragma once
#include "constants.h"
#include "map.h"

#define PRG_ROM_SIZE 0x8000
extern Uint8 prgRom[PRG_ROM_SIZE];
extern Uint8 *chrRom;
extern int chrRomSize;
extern Uint16 tilesetBases[3];

/**
 * @brief Loads the ROM file into PRG and CHR ROM
 * @returns nonzero on successful load, 0 if there's been an error
*/
int Rom_Load(void);

/**
 * @brief Loads a file to the end of the CHR ROM array. Rom_Load must be called
 * before this function.
 * @param filename The filename to load
 * @param size How many bytes to load
 * @returns 1 on success, 0 on error
*/
int Rom_LoadChr(char *filename, int size);

/**
 * @brief allocates a MapData struct and fills it with map data from the ROM image
*/
MapData *Rom_GetMapData(void);

/**
 * @brief allocates a MapData struct and fills it with map data from the arcade ROM
 * (still uses the console ROM, patches the level data to match the arcade version)
 */
MapData *Rom_GetMapDataArcade(void);

