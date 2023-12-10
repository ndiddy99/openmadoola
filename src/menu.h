/* menu.h: Menu display code
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
 * @brief Should be run at the start of a menu
 * @param cursor Pointer to the menu cursor variable
 */
void Menu_Begin(int *cursor);

/**
 * @brief Displays a menu option
 * @param x menu x tile
 * @param y menu y tile
 * @param cursor cursor variable
 * @param text Text to print (printf formatted)
 * @param ... any additional arguments
 * @returns nonzero if the option is selected, zero otherwise
 */
int Menu_Item(Uint16 x, Uint16 y, int cursor, char *text, ...);

/**
 * @brief Should be run at the end of a menu
 * @param cursor Pointer to the menu cursor variable
 */
void Menu_End(int *cursor);
