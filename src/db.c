/* db.c: "database" handler (more like a crappy version of RIFF i guess)
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

// db file spec
// Uint32: num entries
// each entry:
// Uint8: name length
// name
// Uint32: data length
// data

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "buffer.h"
#include "constants.h"
#include "db.h"
#include "file.h"
#include "platform.h"

#define DB_FILENAME "config.db"

static int numEntries;
static int allocedEntries;
static DBEntry *entries;

Uint8 *DB_Add(char *name, Uint32 dataLen) {
    if (numEntries >= allocedEntries) {
        allocedEntries *= 2;
        entries = omrealloc(entries, allocedEntries * sizeof(DBEntry));
    }

    entries[numEntries].name = ommalloc(strlen(name) + 1);
    strcpy(entries[numEntries].name, name);
    entries[numEntries].dataLen = dataLen;
    entries[numEntries].data = ommalloc(dataLen);
    return entries[numEntries++].data;
}

DBEntry *DB_Find(char *name) {
    // probably should change this if we ever go above ~50 entries (aka never)
    for (int i = 0; i < numEntries; i++) {
        if (strcmp(entries[i].name, name) == 0) {
            return &entries[i];
        }
    }
    return NULL;
}

void DB_Set(char *name, Uint8 *data, Uint32 dataLen) {
    DBEntry *entry = DB_Find(name);
    if (entry) {
        entry->dataLen = dataLen;
        if (entry->data) { free(entry->data); }
        entry->data = ommalloc(dataLen);
        memcpy(entry->data, data, dataLen);
    }
    else {
        Uint8 *dbData = DB_Add(name, dataLen);
        memcpy(dbData, data, dataLen);
    }
}

Buffer *DB_Serialize(void) {
    Buffer *buf = Buffer_Init(256);
    Buffer_AddUint32(buf, (Uint32)numEntries);
    for (int i = 0; i < numEntries; i++) {
        // add 1 for the NUL terminator
        Uint8 nameLen = (Uint8)strlen(entries[i].name) + 1;
        Buffer_Add(buf, nameLen);
        Buffer_AddData(buf, entries[i].name, nameLen);
        Buffer_AddUint32(buf, entries[i].dataLen);
        Buffer_AddData(buf, entries[i].data, entries[i].dataLen);
    }
    return buf;
}

void DB_Deserialize(Uint8 *data) {
    char *name;
    int cursor = 0;
    Buffer *dataBuff = Buffer_Init(256);
    Uint32 savedEntries = Buffer_DataReadUint32(data);
    cursor += 4;
    for (Uint32 i = 0; i < savedEntries; i++) {
        Uint8 nameLen = data[cursor++];
        name = data + cursor;
        cursor += nameLen; // nameLen includes the NUL terminator
        Uint32 dataLen = Buffer_DataReadUint32(data + cursor);
        cursor += 4;
        dataBuff->dataSize = 0;
        Buffer_AddData(dataBuff, data + cursor, dataLen);
        cursor += dataLen;
        DB_Set(name, dataBuff->data, dataLen);
    }
    Buffer_Destroy(dataBuff);
}

void DB_Save(void) {
    Buffer *buf = DB_Serialize();
    Buffer_WriteToFile(buf, DB_FILENAME);
    Buffer_Destroy(buf);
}

void DB_Init(void) {
    // init vars
    allocedEntries = 10;
    entries = ommalloc(allocedEntries * sizeof(DBEntry));
    numEntries = 0;

    Uint8 *dbData = File_OpenLoad(DB_FILENAME, NULL);
    if (dbData) {
        DB_Deserialize(dbData);
        free(dbData);
    }
}
