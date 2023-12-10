/* textscroll.h: Text scroll handler
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

/**
 * @brief Initializes text scrolling state, should be run when doing a new text scroll
 * @param skippable Nonzero = pressing start will exit the scroll
*/
void TextScroll_Init(int skippable);

/**
 * @brief Displays scrolling text
 * @param text The text to display (one long string with \n for newlines)
 * @param func Optional function pointer that gets run each frame before displaying text
 * @returns nonzero if the user skipped the scroll, zero otherwise
*/
int TextScroll_DispStr(char *text, void (*func)(void));

/**
 * @brief Displays scrolling tiles
 * @param tiles Tiles to display (0x00 = end of line, 0x01 = newline,
 * 0x00 not in a line = end of tiles array)
 * @param func Optional function pointer that gets run each frame before displaying tiles
 * @returns nonzero if the user skipped the scroll, zero otherwise
*/
int TextScroll_DispTiles(Uint16 *tiles, void (*func)(void));
