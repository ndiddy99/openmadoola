/* buffer.h: Uint8 buffer handler
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
#include <stdio.h>
#include "constants.h"

typedef struct {
    int allocSize;
    int dataSize;
    Uint8 *data;
} Buffer;

/**
 * @brief Creates a new buffer
 * @param allocSize Number of bytes that can go into the buffer without reallocating
 * @return pointer to new buffer
 */
Buffer *Buffer_Init(int allocSize);

/**
 * @brief Creates a new buffer holding the contents of the provided file
 * @param filename File to load from
 * @returns pointer to new buffer, or NULL if we couldn't open the file
 */
Buffer *Buffer_InitFromFile(char *filename);

/**
 * @brief Frees all memory associated with a buffer
 * @param buf The buffer to destroy
 */
void Buffer_Destroy(Buffer *buf);

/**
 * @brief Adds a byte to the end of a buffer
 * @param buf buffer to add to
 * @param data byte to add
 */
void Buffer_Add(Buffer *buf, Uint8 data);

/**
 * @brief Adds a series of bytes to the end of a buffer
 * @param buf buffer to add to
 * @param data bytes to add
 * @param len how many bytes to add
 */
void Buffer_AddData(Buffer *buf, Uint8 *data, int len);

/**
 * @brief Adds a Uint16 to the end of a buffer (big-endian format)
 * @param buf buffer to add to
 * @param data Uint16 to add
 */
void Buffer_AddUint16(Buffer *buf, Uint16 data);

/**
 * @brief Reads a Uint16 out of a buffer
 * @param buf buffer to read out of
 * @param index where to read from in the buffer
 * @return The Uint16
 */
Uint16 Buffer_ReadUint16(Buffer *buf, int index);

/**
 * @brief Adds a Sint16 to the end of a buffer (big-endian format)
 * @param buf buffer to add to
 * @param data Sint16 to add
 */
void Buffer_AddSint16(Buffer *buf, Sint16 data);

/**
 * @brief Reads a Sint16 out of a buffer
 * @param buf buffer to read out of
 * @param index where to read from in the buffer
 * @return The Sint16
 */
Sint16 Buffer_ReadSint16(Buffer *buf, int index);

/**
 * @brief Adds a Uint32 to the end of a buffer (big-endian format)
 * @param buf buffer to add to
 * @param data Uint32 to add
 */
void Buffer_AddUint32(Buffer *buf, Uint32 data);

/**
 * @brief Reads a Uint32 out of a buffer
 * @param buf buffer to read out of
 * @param index where to read from in the buffer
 * @return The Uint32
 */
Uint32 Buffer_ReadUint32(Buffer *buf, int index);

/**
 * @brief Adds data from a file to a buffer
 * @param buf Buffer to add to
 * @param fp file to read from
 * @param size how many bytes to read
 */
void Buffer_AddFromFile(Buffer *buf, FILE *fp, int size);

/**
 * @brief Adds the entire contents of a file to a buffer. fp will be at the end
 * of the file and needs to be rewound if you want to read anything else from it.
 * @param buf Buffer to add to
 * @param filename file to read from
 */
void Buffer_AddFile(Buffer *buf, FILE *fp);

/**
 * @brief Writes a buffer to a file
 * @param buf Buffer to write to
 * @param filename Filename to write the buffer to
 */
void Buffer_WriteToFile(Buffer *buf, char *filename);

/**
 * @brief Appends one buffer to another buffer
 * @param dst Buffer to append src to
 * @param src Buffer to append to dst
 */
void Buffer_Append(Buffer *dst, Buffer *src);