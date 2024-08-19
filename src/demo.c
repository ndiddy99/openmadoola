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

#include "buffer.h"
#include "constants.h"
#include "file.h"
#include "game.h"
#include "lucia.h"
#include "rng.h"
#include "save.h"
#include "util.h"

static Buffer *demoBuff = NULL;
static int recording = 0;
static int playing = 0;
static Uint32 lastInput;
static Uint8 inputFrames;
static Uint32 cursor;

void Demo_Record(void) {
    if (!demoBuff) {
        demoBuff = Buffer_Init(8192);
    }
    else {
        demoBuff->dataSize = 0;
    }
    recording = 1;
    playing = 0;
    // record non-input stuff needed to play back the demo
    Buffer_Add(demoBuff, gameType);
    Buffer_Add(demoBuff, stage);
    Buffer_Add(demoBuff, rngVal);
    Buffer_AddSint16(demoBuff, health);
    Buffer_AddUint32(demoBuff, score);
    Buffer_Add(demoBuff, lives);
    Save_Serialize(demoBuff);
}

int Demo_Playback(char *filename) {
    if (!demoBuff) {
        demoBuff = Buffer_InitFromFile(filename);
        if (!demoBuff) { return 0; }
    }
    else {
        FILE *fp = File_Open(filename, "rb");
        if (!fp) { return 0; }
        demoBuff->dataSize = 0;
        Buffer_AddFile(demoBuff, fp);
        fclose(fp);
    }

    playing = 1;
    recording = 0;
    cursor = 0;

    gameType = demoBuff->data[cursor++];
    stage = demoBuff->data[cursor++];
    rngVal = demoBuff->data[cursor++];
    health = Util_LoadSint16(demoBuff->data + cursor);
    cursor += sizeof(health);
    score = Util_LoadUint32(demoBuff->data + cursor);
    cursor += sizeof(score);
    lives = demoBuff->data[cursor++];
    cursor += Save_Deserialize(demoBuff->data + cursor);

    inputFrames = demoBuff->data[cursor++];
    lastInput = Util_LoadUint32(demoBuff->data + cursor);
    cursor += sizeof(lastInput);
    Game_RunStage();
    return 1;
}

int Demo_Recording(void) {
    return recording;
}

int Demo_Playing(void) {
    return playing;
}

void Demo_RecordInput(Uint32 input) {
    if (!recording) { return; }

    if (!demoBuff->dataSize) {
        lastInput = input;
        inputFrames = 0;
    }
    else {
        if (lastInput == input) {
            inputFrames++;
            if (inputFrames == 0xff) {
                Buffer_Add(demoBuff, inputFrames);
                Buffer_AddUint32(demoBuff, input);
            }
        }
        else {
            Buffer_Add(demoBuff, inputFrames);
            Buffer_AddUint32(demoBuff, lastInput);
            lastInput = input;
            inputFrames = 0;
        }
    }
}

Uint32 Demo_GetInput(void) {
    if (!playing || (cursor >= demoBuff->dataSize)) { return 0; }
    
    Uint32 joy = lastInput;
    if (inputFrames == 0) {
        inputFrames = demoBuff->data[cursor++];
        lastInput = Util_LoadUint32(demoBuff->data + cursor);
        cursor += sizeof(lastInput);
    }
    else {
        inputFrames--;
    }
    return joy;
}

void Demo_Save(void) {
    if (!recording) { return; }
    // finish up the recording
    Buffer_Add(demoBuff, inputFrames);
    Buffer_AddUint32(demoBuff, lastInput);
    recording = 0;
    char filename[20];
    snprintf(filename, sizeof(filename), "stage%d.dem", stage + 1);
    Buffer_WriteToFile(demoBuff, filename);
}
