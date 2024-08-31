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
#include "util.h"

Buffer *Buffer_Init(int allocSize) {
    if (allocSize <= 0) { return NULL; }
    Buffer *buf = ommalloc(sizeof(Buffer));
    buf->allocSize = allocSize;
    buf->dataSize = 0;
    buf->data = ommalloc(allocSize);
    return buf;
}

Buffer *Buffer_InitFromFile(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
    Buffer *buf = Buffer_Init(size);
    Buffer_AddFromFile(buf, fp, size);
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
    Uint8 dataArr[sizeof(data)];
    Util_SaveUint16(data, dataArr);
    Buffer_AddData(buf, dataArr, sizeof(dataArr));
}

Uint16 Buffer_ReadUint16(Buffer *buf, int index) {
    return Util_LoadUint16(buf->data + index);
}

void Buffer_AddSint16(Buffer *buf, Sint16 data) {
    Buffer_AddUint16(buf, (Sint16)data);
}

Sint16 Buffer_ReadSint16(Buffer *buf, int index) {
    return Util_LoadSint16(buf->data + index);
}

void Buffer_AddUint32(Buffer *buf, Uint32 data) {
    Uint8 dataArr[sizeof(data)];
    Util_SaveUint32(data, dataArr);
    Buffer_AddData(buf, dataArr, sizeof(dataArr));
}

Uint32 Buffer_ReadUint32(Buffer *buf, int index) {
    return Util_LoadUint32(buf->data + index);
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

void Buffer_AddFile(Buffer *buf, FILE *fp) {
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
    Buffer_AddFromFile(buf, fp, size);
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