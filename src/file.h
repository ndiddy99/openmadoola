/* file.h: File management utility functions
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
#include <stdio.h>
#include "constants.h"

/**
 * @brief Writes a 16-bit number to a file as big endian data.
 * @param data The number to write
 * @param fp The file to write to
 */
void File_WriteUint16BE(Uint16 data, FILE *fp);

/**
 * @brief Reads a 16-bit big endian number from a file.
 * @param fp The file to read from
 * @returns The read number
 */
Uint16 File_ReadUint16BE(FILE *fp);

/**
 * @brief Writes a 32-bit number to a file as big endian data.
 * @param data The number to write
 * @param fp The file to write to
 */
void File_WriteUint32BE(Uint32 data, FILE *fp);

/**
 * @brief Reads a 32-bit big endian number from a file.
 * @param fp The file to read from
 * @returns The read number
 */
Uint32 File_ReadUint32BE(FILE *fp);