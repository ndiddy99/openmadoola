/* buffer.c: Uint8 buffer handler
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

#include <stdio.h>

#include "alloc.h"
#include "buffer.h"
#include "constants.h"
#include "file.h"
#include "platform.h"

Buffer *Buffer_Init(int allocSize) {
    if (allocSize <= 0) { return NULL; }
    Buffer *buf = ommalloc(sizeof(Buffer));
    buf->allocSize = allocSize;
    buf->dataSize = 0;
    buf->data = ommalloc(allocSize);
    return buf;
}

void Buffer_Destroy(Buffer *buf) {
    free(buf->data);
    free(buf);
}

void Buffer_Add(Buffer *buf, Uint8 data) {
    buf->data[buf->dataSize] = data;
    buf->dataSize++;
    if (buf->dataSize >= buf->allocSize) {
        buf->allocSize *= 2;
        buf->data = omrealloc(buf->data, buf->allocSize);
    }
}

void Buffer_AddData(Buffer *buf, Uint8 *data, int len) {
    for (int i = 0; i < len; i++) {
        Buffer_Add(buf, data[i]);
    }
}

void Buffer_AddUint16(Buffer *buf, Uint16 data) {
    Buffer_Add(buf, (Uint8)((data >> 8) & 0xff));
    Buffer_Add(buf, (Uint8)(data & 0xff));
}

Uint16 Buffer_DataReadUint16(Uint8 *data) {
    Uint16 val = ((Uint16)data[0]) << 8;
    val |= (Uint16)data[1];
    return val;
}

Uint16 Buffer_ReadUint16(Buffer *buf, int index) {
    return Buffer_DataReadUint16(buf->data + index);
}

void Buffer_AddUint32(Buffer *buf, Uint32 data) {
    Buffer_Add(buf, (Uint8)((data >> 24) & 0xff));
    Buffer_Add(buf, (Uint8)((data >> 16) & 0xff));
    Buffer_Add(buf, (Uint8)((data >>  8) & 0xff));
    Buffer_Add(buf, (Uint8)(data & 0xff));
}

Uint32 Buffer_DataReadUint32(Uint8 *data) {
    Uint32 val = ((Uint32)data[0]) << 24;
    val |= ((Uint32)data[1]) << 16;
    val |= ((Uint32)data[2]) << 8;
    val |= (Uint32)data[3];
    return val;
}

Uint32 Buffer_ReadUint32(Buffer *buf, int index) {
    return Buffer_DataReadUint32(buf->data + index);
}

void Buffer_AddFromFile(Buffer *buf, FILE *fp, int size) {
    int count = 0;

    while (count < size) {
        int data = fgetc(fp);
        if (data == EOF) { return; }
        Buffer_Add(buf, (Uint8)data);
        count++;
    }
}

void Buffer_WriteToFile(Buffer *buf, char *filename) {
    FILE *fp = File_Open(filename, "wb");
    if (fp) {
        fwrite(buf->data, 1, buf->dataSize, fp);
        fclose(fp);
    }
    else {
        Platform_ShowError("Couldn't open %s for writing.", filename);
    }
}

void Buffer_Append(Buffer *dst, Buffer *src) {
    for (int i = 0; i < src->dataSize; i++) {
        Buffer_Add(dst, src->data[i]);
    }
}