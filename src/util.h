/* util.h: General utility functions
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

#pragma once
#include "constants.h"

/**
 * @brief Writes the provided Uint16 to a buffer in big-endian order.
 * @param num the number to save
 * @param out the buffer to write to
 */
void Util_SaveUint16(Uint16 num, Uint8 *out);

/**
 * @brief Reads a Uint16 out of a buffer in big-endian order.
 * @param buff the buffer to read from
 * @returns the number
 */
Uint16 Util_LoadUint16(Uint8 *buff);

/**
 * @brief Writes the provided Sint16 to a buffer in big-endian order.
 * @param num the number to save
 * @param out the buffer to write to
 */
void Util_SaveSint16(Sint16 num, Uint8 *out);

/**
 * @brief Reads a Sint16 out of a buffer in big-endian order.
 * @param buff the buffer to read from
 * @returns the number
 */
Sint16 Util_LoadSint16(Uint8 *buff);

/**
 * @brief Writes the provided Uint32 to a buffer in big-endian order.
 * @param num the number to save
 * @param out the buffer to write to
 */
void Util_SaveUint32(Uint32 num, Uint8 *out);

/**
 * @brief Reads a Uint32 out of a buffer in big-endian order.
 * @param buff the buffer to read from
 * @returns the number
 */
Uint32 Util_LoadUint32(Uint8 *buff);

/**
 * @brief Writes the provided Sint32 to a buffer in big-endian order.
 * @param num the number to save
 * @param out the buffer to write to
 */
void Util_SaveSint32(Sint32 num, Uint8 *out);

/**
 * @brief Reads a Sint32 out of a buffer in big-endian order.
 * @param buff the buffer to read from
 * @returns the number
 */
Sint32 Util_LoadSint32(Uint8 *buff);
