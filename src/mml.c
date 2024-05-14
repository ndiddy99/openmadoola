/* mml.c: MML compiler
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

#include "alloc.h"
#include "buffer.h"
#include "constants.h"
#include "file.h"
#include "mml.h"
#include "sound.h"

static FILE *infile;

// used for keeping track of where we are in the input file for error messages
static int line;
static int column;
// gets used instead of reading a character from the input file if it's not NO_CH
#define NO_CH -2
static int nextCh;
// which apu we're using (0 or 1), set by the A command
static int apu;
// how many frames (1 frame = 1/60 second) a 16th note (smallest note we support) should take, set by the t command
static int tempo;
// default note length, set by the f command
static int defaultLength;

// we error out when we hit this cursor number (65535 not 655366 because the sound engine uses a cursor value of 65535
// as an "instrument not playing" flag)
#define CURSOR_LIMIT 65535

typedef struct {
    // whether the instrument is used
    int enabled;
    // output binary data for this instrument
    Buffer *outBuff;
    // what command number we're writing to the output file
    int cursor;
    // where the song should loop to after finishing, set by the L command
    int loopPoint;
    // the start of the current loop, set by the [ command
    int loopPos;
    // APU channel (0 = Pulse 1, 1 = Pulse 2, 2 = Triangle, 3 = Noise)
    int channel;
    // initial APU register 1 setting
    int reg1;
    // initial APU register 0 setting
    int reg0;
    // current octave, set by the o command, incremented by the > command, decremented by the < command
    int octave;
    // how many frames the instrument's taken so far
    int frames;
    // how many frames are in the current loop
    int loopFrames;
} InstData;

// number of NES APU sound channels
#define NUM_CHANNELS 4
// maximum number of instruments the sound engine supports
#define NUM_INSTRUMENTS 6
static InstData instruments[NUM_INSTRUMENTS];

static noreturn void errorExit(char *message) {
    printf("Line %d column %d: %s\n", line, column, message);
    exit(-1);
}

static void initInstrument(int num) {
    instruments[num].enabled = 1;
    instruments[num].outBuff = Buffer_Init(256);
    instruments[num].cursor = 0;
    instruments[num].loopPoint = -1;
    instruments[num].loopPos = -1;
    instruments[num].channel = -1;
    instruments[num].reg1 = -1;
    instruments[num].reg0 = -1;
    instruments[num].octave = -1;
    instruments[num].frames = 0;
    instruments[num].loopFrames = 0;
}

static void nextLine(void) {
    int ch;
    while (1) {
        ch = getc(infile);
        if ((ch == EOF) || (ch == '\n')) {
            break;
        }
    }
    column = 0;
    line++;
}

/**
 * @brief Gets the next character out of the file
 * @param whitespace nonzero if you want to get whitespace, 0 if you want to skip whitespace
 * @returns the next character from the file
 */
static int readCh(int whitespace) {
    int ch;

    while (1) {
        if (nextCh != NO_CH) {
            ch = nextCh;
            nextCh = NO_CH;
        }
        else {
            ch = getc(infile);
        }
        if (ch == '\n') {
            column = 0;
            line++;
        }
        else { column++; }
        if ((ch == EOF) || whitespace || !isspace(ch)) { break; }
    }
    return ch;
}

static void pushCh(int ch) {
    nextCh = ch;
    column--;
}

static int readNum(void) {
    char numStr[5] = {0};
    int numCursor = 0;

    // leave space for null terminator
    while (numCursor < (ARRAY_LEN(numStr) - 1)) {
        int ch = readCh(0);
        if (isdigit(ch)) {
            numStr[numCursor++] = (char)ch;
        }
        else {
            pushCh(ch);
            if (numCursor == 0) {
                return EOF;
            }
            else {
                return atoi(numStr);
            }
        }
    }
    errorExit("Number too large");
}

static int readHex(void) {
    char hexStr[3] = {0};
    int hexCursor = 0;

    while (hexCursor < (ARRAY_LEN(hexStr) - 1)) {
        // we care about whitespace because some note commands are also valid hex digits so we want a space to
        // terminate the hex string
        int ch = readCh(1);
        if (isxdigit(ch)) {
            hexStr[hexCursor++] = (char)ch;
        }
        else {
            pushCh(ch);
            break;
        }
    }
    if (hexCursor == 0) {
        return EOF;
    }
    else {
        return (int)strtol(hexStr, NULL, 16);
    }
}

static int getNoteFrames(int noteNum) {
    // valid notes: powers of 2 between 1 and 16
    if ((noteNum >= 1) && (noteNum <= 16) && ((noteNum & (noteNum - 1)) == 0)) {
        int frames = tempo * (16 / noteNum);
        // handle dotted notes
        int originalNote = frames;
        int ch;
        for (int i = 0; i < 3; i++) {
            ch = readCh(0);
            if (ch == '.') {
                originalNote /= 2;
                frames += originalNote;
            }
            else {
                pushCh(ch);
                return frames;
            }
        }
        errorExit("Too many dots on note (max 3)");
    }
    else {
        errorExit("Invalid note duration");
    }
}

static int noteToFrames(void) {
    if (tempo <= 0) { errorExit("Tempo must be set with t command"); }
    int noteNum = readNum();
    if (noteNum < 0) {
        if (defaultLength > 0) {
            return defaultLength;
        }
        else {
            errorExit("Default note length must be set with l command");
        }
    }
    int frames = getNoteFrames(noteNum);
    // handle tie(s)
    int ch = readCh(0);
    while (ch == '^') {
        noteNum = readNum();
        if (noteNum < 0) {
            errorExit("Note not specified for tie");
        }
        else {
            frames += getNoteFrames(noteNum);
        }
	ch = readCh(0);
    }
    pushCh(ch);
    if (frames >= 65536) {
        errorExit("Note duration too long (max: 65535 frames)");
    }
    return frames;
}

static void checkInst(InstData *i) {
    if (!i) { errorExit("InstData number must be defined with the I command."); }
}

static void addInstFrames(InstData *i, int frames) {
    if (i->loopPos >= 0) {
        i->loopFrames += frames;
    }
    else {
        i->frames += frames;
    }
}

static void writeCmd(InstData *i, Uint8 cmd, Uint16 param) {
    checkInst(i);
    if (apu < 0) { errorExit("APU number must be set with A command"); }
    if ((i->reg0 < 0) || (i->reg1 < 0)) { errorExit("APU registers 0 & 1 must be initialized with R0 & R1 commands"); }
    if (i->channel < 0) { errorExit("APU channel must be initialized with C command"); }
    Buffer_Add(i->outBuff, cmd);
    Buffer_AddUint16(i->outBuff, param);
    i->cursor++;
    if (i->cursor >= CURSOR_LIMIT) { errorExit("Song too long"); }
}

int MML_Compile(const char *filename, Sound *sound){
    // input file
    infile = File_OpenResource(filename, "r");
    if (!infile) {
        return 0;
    }
    printf("--- Compiling %s ---\n", filename);

    // initialize compiler state
    line = 1;
    column = 0;
    nextCh = NO_CH;
    apu = -1;
    tempo = -1;
    defaultLength = -1;

    int ch;
    int note;
    int frames;
    int numNoises = 0;
    Uint8 noiseList[256];
    int instNum;
    InstData *inst = NULL;
    while (!feof(infile)) {
        ch = readCh(0);
        switch (ch) {
        case EOF: break;

        // comment
        case ';':
            nextLine();
            break;

        // loop start
        case '[':
            checkInst(inst);
            if (inst->loopPos < 0) {
                inst->loopPos = inst->cursor;
            }
            else {
                errorExit("Nested loops aren't allowed");
            }
            break;

        // loop end
        case ']':
            checkInst(inst);
            if (inst->loopPos >= 0) {
                int loopCount = readNum();
                if ((loopCount >= 2) && (loopCount <= 16)) {
                    // b0 = loop command
                    writeCmd(inst, 0xb0 | (loopCount - 2), inst->loopPos);
                    inst->loopPos = -1;
                    inst->frames += (inst->loopFrames * loopCount);
                    inst->loopFrames = 0;
                }
                else {
                    errorExit("Loop count must be between 2 and 16");
                }
            }
            break;

        // up octave
        case '>':
            checkInst(inst);
            if ((inst->octave >= 1) && (inst->octave <= 8)) {
                inst->octave++;
                if (inst->octave > 8) { errorExit("Octave must be between 1 and 8"); }
            }
            else {
                errorExit("Octave must be initialized with o command");
            }
            break;

        // down octave
        case '<':
            checkInst(inst);
            if ((inst->octave >= 1) && (inst->octave <= 8)) {
                inst->octave--;
                if (inst->octave < 1) { errorExit("Octave must be between 1 and 8"); }
            }
            else {
                errorExit("Octave must be initialized with o command");
            }
            break;

        // note
        case 'c':
            note = 0; goto doneNote;
        case 'd':
            note = 2; goto doneNote;
        case 'e':
            note = 4; goto doneNote;
        case 'f':
            note = 5; goto doneNote;
        case 'g':
            note = 7; goto doneNote;
        case 'a':
            note = 9; goto doneNote;
        case 'b':
            note = 11;
        doneNote:;
            checkInst(inst);
            int sharp = readCh(0);
            if (sharp == '#') {
                sharp = 1;
                if ((ch == 'e') || (ch == 'b')) {
                    errorExit("Only c#, d#, f#, g#, and a# are permitted sharp notes");
                }
            }
            else {
                pushCh(sharp);
                sharp = 0;
            }
            note += sharp;
            frames = noteToFrames();
            writeCmd(inst, (inst->octave - 1) << 4 | note, frames);
            addInstFrames(inst, frames);
            break;

        // APU number
        case 'A':
            if (apu >= 0) { errorExit("APU number can't be changed"); }
            int apuNum = readNum();
            if ((apuNum == 0) || (apuNum == 1)) {
                apu = apuNum;
            }
            else {
                errorExit("APU number must be 0 or 1");
            }
            break;

        // APU channel number
        case 'C':
            checkInst(inst);
            int channelNum = readNum();
            if ((channelNum >= 0) && (channelNum < NUM_CHANNELS)) {
                if (inst->channel >= 0) { errorExit("APU channel can't be changed"); }
                inst->channel = channelNum;
            }
            else {
                errorExit("Channel number must be between 0 and 3");
            }
            break;

        // instrument number
        case 'I':;
            // frame count diagnostic is only accurate when switching instruments outside a loop
            if (inst && inst->frames && (inst->loopPos == -1)) {
                printf("Instrument %d: %d frames\n", instNum, inst->frames);
            }
            instNum = readNum();
            if ((instNum >= 0) && (instNum < NUM_INSTRUMENTS)) {
                inst = &instruments[instNum];
                if (!inst->enabled) {
                    initInstrument(instNum);
                }
            }
            else {
                errorExit("Instrument number must be between 0 and 5");
            }
            break;

        // loop point
        case 'L':
            checkInst(inst);
            inst->loopPoint = inst->cursor;
            break;

        // default note length
        case 'l':
            defaultLength = noteToFrames();
            break;

        // adds noise to list
        case 'N':;
            int noiseToAdd = readHex();
            if ((noiseToAdd <= 0) || (noiseToAdd >= 0xff) || ((noiseToAdd >= 0xa0) && (noiseToAdd < 0xc0))) {
                errorExit("Noise must be 0-a0 or c0-fe");
            }
            int noiseFound = 0;
            for (int i = 0; i < numNoises; i++) {
                if (noiseList[i] == (Uint8)noiseToAdd) {
                    noiseFound = 1;
                    break;
                }
            }
            if (!noiseFound) {
                noiseList[numNoises++] = (Uint8)noiseToAdd;
            }
            break;

        // plays noise
        case 'n':
            checkInst(inst);
            if (inst->channel != 3) { errorExit("Noise can only be played on APU channel 3"); }
            int noiseToPlay = readNum();
            if ((noiseToPlay < 0) || (noiseToPlay >= numNoises)) {
                errorExit("Invalid noise number");
            }
            ch = readCh(0);
            if (ch == ',') {
                frames = noteToFrames();
            }
            else {
                frames = defaultLength;
                pushCh(ch);
            }
            writeCmd(inst, noiseList[noiseToPlay], frames);
            addInstFrames(inst, frames);
            break;

        // octave specifier
        case 'o':
            checkInst(inst);
            inst->octave = readNum();
            if ((inst->octave < 1) || (inst->octave > 8)) {
                errorExit("Octave must be between 1 and 8");
            }
            break;

        // register set
        case 'R':;
            checkInst(inst);
            int regNum = readNum();
            if ((regNum != 0) && (regNum != 1)) {
                errorExit("Register to set must be 0 or 1");
            }
            ch = readCh(0);
            if ((ch != ',') && (ch != ':')) {
                errorExit("Register value not specified");
            }
            int regVal = readHex();
            if ((regVal < 0) || (regVal > 255)) {
                errorExit("Invalid register value");
            }
            if ((regNum == 0) && (inst->reg0 < 0)) {
                inst->reg0 = regVal;
            }
            else if ((regNum == 1) && (inst->reg1 < 0)) {
                inst->reg1 = regVal;
            }
            else {
                writeCmd(inst, 0xa0 + regNum, regVal);
            }
            break;

        // rest
        case 'r':
            frames = noteToFrames();
            writeCmd(inst, 0x6f, frames);
            addInstFrames(inst, frames);
            break;

        // tempo specifier (how many frames a 16th note takes)
        case 't':
            tempo = readNum();
            if (tempo <= 0) {
                errorExit("Tempo must be greater than 0");
            }
            break;

        // we shouldn't end up here
        default:
            errorExit("Syntax error");
        }
    }

    fclose(infile);

    // print frame count for final instrument statement
    if (inst) {
        printf("Instrument %d: %d frames\n", instNum, inst->frames);
    }

    memset(sound, 0, sizeof(Sound));
    sound->isMusic = apu;

    // finish up the instrument data
    for (int i = 0; i < NUM_INSTRUMENTS; i++) {
        inst = &instruments[i];
        if (inst->enabled) {
            if (inst->loopPoint >= 0) {
                writeCmd(inst, 0xbf, inst->loopPoint);
            }
            // "end of track" command
            else {
                writeCmd(inst, 0xff, 0x00);
            }
            sound->count++;
        }
    }
    // write out the instrument data so the sound engine can read it
    sound->data = ommalloc(sound->count * sizeof(Instrument));
    int count = 0;
    for (int i = 0; i < NUM_INSTRUMENTS; i++) {
        inst = &instruments[i];
        if (inst->enabled) {
            sound->data[count].num = i;
            sound->data[count].data = inst->outBuff->data;
            // Buffer_Destroy frees the data as well, which we don't want to do
            free(inst->outBuff);
            sound->data[count].channel = (Uint8)(inst->channel);
            sound->data[count].cursor = 0;
            sound->data[count].reg1 = (Uint8)(inst->reg1);
            sound->data[count].reg0 = (Uint8)(inst->reg0);
            count++;
        }
    }

    memset(instruments, 0, sizeof(instruments));
    return 1;
}
