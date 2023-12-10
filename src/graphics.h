/* graphics.h: Graphics primitives
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

#pragma once
#include "constants.h"

#define TILE_WIDTH (8)
#define TILE_HEIGHT (8)
#define PALETTE_SIZE (4)
#define H_MIRROR (1 << 0)
#define V_MIRROR (1 << 1)

// --- Functions that you need to implement for the game to work ---

/**
 * @brief Initializes graphics and framebuffer
 * @returns 1 on success, 0 on failure
*/
int Graphics_Init(void);

/**
 * @brief Should be run at the start of each frame
 */
void Graphics_StartFrame(void);

/**
 * @brief draws an 8x8 tile to the framebuffer
 * @param x tile x pos
 * @param y tile y pos
 * @param tilenum tile number
 * @param palette 4 byte palette array
 * @param mirror V_MIRROR, H_MIRROR, or both
*/
void Graphics_DrawTile(int x, int y, int tilenum, int palnum, int mirror);
