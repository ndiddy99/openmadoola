/* demo.c: Demo recording/playback code
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
#include <string.h>

#include "buffer.h"
#include "constants.h"
#include "demo.h"
#include "file.h"
#include "lucia.h"
#include "save.h"
#include "task.h"
#include "util.h"
#include "weapon.h"

static Buffer *demoBuff = NULL;
static Buffer *recordFilename = NULL;
static int recording = 0;
static int playing = 0;
static Uint32 lastInput;
static Uint8 frameCount;
static int first;
static int cursor;

void Demo_Record(char *filename, DemoData *data) {
    if (!demoBuff) {
        demoBuff = Buffer_Init(8192);
    }
    else {
        demoBuff->dataSize = 0;
    }
    if (recordFilename) {
        Buffer_Destroy(recordFilename);
    }
    recordFilename = Buffer_InitFromString(filename);
    first = 1;
    recording = 1;
    playing = 0;
    // record non-input stuff needed to play back the demo
    Buffer_Add(demoBuff, data->rngVal);
    Buffer_Add(demoBuff, data->gameFrames);
    Buffer_Add(demoBuff, data->gameType);
    Buffer_Add(demoBuff, data->stage);
    Buffer_AddSint16(demoBuff, data->health);
    Buffer_AddSint16(demoBuff, data->magic);
    Buffer_Add(demoBuff, data->bootsLevel);
    Buffer_AddData(demoBuff, data->weaponLevels, NUM_WEAPONS);
}

int Demo_Playback(char *filename, DemoData *out) {
    FILE *fp = File_OpenResource(filename, "rb");
    if (!fp) { return 0; }
    if (!demoBuff) {
        demoBuff = Buffer_InitFromFile(fp);
        fclose(fp);
    }
    else {
        demoBuff->dataSize = 0;
        Buffer_AddFile(demoBuff, fp);
        fclose(fp);
    }

    playing = 1;
    recording = 0;
    cursor = 0;

    out->rngVal = demoBuff->data[cursor++];
    out->gameFrames = demoBuff->data[cursor++];
    out->gameType = demoBuff->data[cursor++];
    out->stage = demoBuff->data[cursor++];
    out->health = Util_LoadSint16(demoBuff->data + cursor); cursor += 2;
    out->magic = Util_LoadSint16(demoBuff->data + cursor); cursor += 2;
    out->bootsLevel = demoBuff->data[cursor++];
    memcpy(out->weaponLevels, demoBuff->data + cursor, sizeof(out->weaponLevels));
    cursor += sizeof(weaponLevels);
    frameCount = demoBuff->data[cursor];
    // get first button press ready
    Task_Yield();
    return 1;
}

void Demo_Uninit(void) {
    recording = 0;
    playing = 0;
    // wait a frame so the demo input->joypad input transition doesn't cause any
    // false button presses
    Task_Yield();
}

int Demo_Recording(void) {
    return recording;
}

int Demo_Playing(void) {
    return playing;
}

void Demo_RecordInput(Uint32 input) {
    if (!recording) { return; }

    if (first || (lastInput != input)) {
        first = 0;
        lastInput = input;
        Buffer_Add(demoBuff, 0); // frame counter
        Buffer_AddUint32(demoBuff, input);
    }
    else {
        Uint8 *frameCountPtr = &(demoBuff->data[demoBuff->dataSize - 5]);
        if (*frameCountPtr == 255) {
            Buffer_Add(demoBuff, 0);
            Buffer_AddUint32(demoBuff, lastInput);
        }
        else {
            (*frameCountPtr)++;
        }
    }
}

Uint32 Demo_GetInput(void) {
    if (!playing || (cursor > demoBuff->dataSize)) { return 0; }
    
    Uint32 joy = Util_LoadUint32(demoBuff->data + cursor + 1);
    frameCount--;
    if (frameCount == 255) {
        cursor += 5;
        frameCount = demoBuff->data[cursor];
    }
    return joy;
}

void Demo_Save(void) {
    if (!recording) { return; }
    recording = 0;
    Buffer_WriteToFile(demoBuff, (char *)recordFilename->data);
}
