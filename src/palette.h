/* palette.h: Color palette
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
#include "graphics.h"

extern Uint8 colorPalette[PALETTE_SIZE * 8];
extern Uint8 flashTimer;

/**
 * @brief Sets up the color palette. Should be run at the start of each frame.
*/
void Palette_Run(void);

/**
 * @brief Gets the palette that should be used to draw the current frame. Should
 * be used by graphics.c
 * @returns the current palette
*/
Uint8 *Palette_Get(void);
