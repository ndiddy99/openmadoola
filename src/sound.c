/* sound.c: Sound engine and output code
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

#include <stdio.h>
#include <string.h>

#include "blargg_apu.h"
#include "constants.h"
#include "db.h"
#include "game.h"
#include "platform.h"
#include "rom.h"
#include "sound.h"

#define SOUND_FREQ 44100
#define SAMPLES_PER_FRAME (SOUND_FREQ / 60)
// how many frames of audio to store in the sound buffer (increase if your sound skips)
#define BUFFERED_FRAMES 3

#define NUM_INSTRUMENTS 6
// state of a playing instrument
typedef struct {
    Uint8 num;
    Uint8 *data;
    Uint8 channel;
    Uint8 cursor;
    Uint8 reg1;
    Uint8 reg0;
    Uint8 timer;
    Uint8 loop;
    Uint8 ctrlRegsSet;
} Instrument;

typedef struct {
    Uint8 isMusic;
    Uint8 count;
    Instrument *data;
} Sound;

#define NUM_SOUNDS 32
static Sound sounds[NUM_SOUNDS];
// where sound data is stored in CHR ROM
#define CHR_ROM_SOUND (0x7B70)
// where sound data is stored in PRG ROM
#define PRG_ROM_SOUND (0x75DE)

// currently playing instruments
static Instrument instruments[NUM_INSTRUMENTS];
static Instrument savedInstruments[NUM_INSTRUMENTS];
static Instrument musInstruments[NUM_INSTRUMENTS];
static Instrument savedMusInstruments[NUM_INSTRUMENTS];
#define APU_CHANNELS 4
static Uint8 channelsInUse[APU_CHANNELS * 2];
static Uint8 apuStatusCopy[2];
// 0-100
static int volume = 50;

static void Sound_RunInstrument(int apu, Instrument *inst);
static void Sound_DisableChannel(int apu, Uint8 channel);
static void Sound_EnableChannel(int apu, Uint8 channel);

static Uint8 *Sound_LoadData(Uint8 *romData, Sound *out) {
    int cursor = 0;
    out->count = romData[cursor++];
    out->data = malloc(out->count * sizeof(Instrument));
    for (int i = 0; i < out->count; i++) {
        out->data[i].num = romData[cursor++];
        Uint16 addr = romData[cursor] | (romData[cursor + 1] << 8);
        cursor += 2;
        // title screen and ending music are stored in CHR ROM and loaded to
        // RAM during playback, so if we're seeing an address in RAM we know
        // that sound is stored in CHR ROM
        if (addr < 0x8000) {
            // sound data is loaded to 0x400 in memory
            out->data[i].data = chrRom + CHR_ROM_SOUND + addr - 0x400;
        }
        else {
            // PRG ROM is mapped into NES memory at 0x8000-0xFFFF
            out->data[i].data = prgRom + addr - 0x8000;
        }
        out->data[i].channel = romData[cursor++];
        out->data[i].cursor = romData[cursor++];
        out->data[i].reg1 = romData[cursor++];
        out->data[i].reg0 = romData[cursor++];
    }

    return romData + cursor;
}

int Sound_Init(void) {
    Blargg_Apu_Init(SOUND_FREQ);
    DBEntry *entry = DB_Find("volume");
    if (entry) {
        volume = (int)entry->data[0];
    }
    Blargg_Apu_Volume(volume);

    // load sound data from the ROM
    Uint8 *src = chrRom + CHR_ROM_SOUND;
    for (int i = 0; i < NUM_SOUNDS; i++) {
        src = Sound_LoadData(src, &sounds[i]);
        // make sure you change this if you mess with the music order!
        sounds[i].isMusic = ((i < SFX_PERASKULL) || (i == MUS_CASTLE));
        // sound data stored in CHR ROM is padded for some reason
        if (i == 0) { src = chrRom + CHR_ROM_SOUND + 0x20; }
        // after loading title screen and ending music, switch to PRG ROM
        if (i == 1) { src = prgRom + PRG_ROM_SOUND; }
    }
    // patch sound data to fix instrument allocation problems (why certain
    // sound effects would cause issues with other sound effects, music
    // channels dropping out, etc in the original game)
    sounds[SFX_FIREBALL].data[0].num = 5;
    sounds[SFX_PAUSE].data[0].num = 4;
    sounds[SFX_SELECT].data[0].num = 5;
    sounds[SFX_SELECT].data[1].num = 4;

    Sound_Reset();
    return 1;
}

int Sound_SetVolume(int vol) {
    volume = vol;
    CLAMP(volume, 0, 100);
    Blargg_Apu_Volume(volume);
    Uint8 volumeByte = (Uint8)volume;
    DB_Set("volume", &volumeByte, 1);
    DB_Save();
    return volume;
}

int Sound_GetVolume(void) {
    return volume;
}

void Sound_Reset(void) {
    // initialize sound engine state
    for (int i = 0; i < NUM_INSTRUMENTS; i++) {
        instruments[i].cursor = 0xff;
        musInstruments[i].cursor = 0xff;
    }
    apuStatusCopy[0] = 0;
    apuStatusCopy[1] = 0;
    Blargg_Apu_Write(0, 0x4015, apuStatusCopy[0]);
    Blargg_Apu_Write(1, 0x4015, apuStatusCopy[1]);
    Blargg_Apu_ClearBuffer();
}

void Sound_Play(int num) {
    // copy all instruments from a sound into their respective slots
    Instrument *destInsts;
    int apu;
    if ((gameType == GAME_TYPE_PLUS) && sounds[num].isMusic) {
        destInsts = musInstruments;
        apu = 1;
    }
    else {
        destInsts = instruments;
        apu = 0;
    }
    for (int i = 0; i < sounds[num].count; i++) {
        Uint8 instNum = sounds[num].data[i].num;
        destInsts[instNum] = sounds[num].data[i];
        destInsts[instNum].timer = 1;
        destInsts[instNum].loop = 0xff;
        destInsts[instNum].ctrlRegsSet = 0xff;
        // turn off the channel for the previous instrument in this slot
        // Note: The original game didn't do this, which is why sound effects
        // would sometimes stay on, etc.
        if (destInsts[instNum].channel < 4) {
            Sound_DisableChannel(apu, instruments[instNum].channel);
        }
    }
}

void Sound_SaveState(void) {
    memcpy(savedInstruments, instruments, sizeof(instruments));
    memcpy(savedMusInstruments, musInstruments, sizeof(musInstruments));
}

void Sound_LoadState(void) {
    memcpy(instruments, savedInstruments, sizeof(instruments));
    memcpy(musInstruments, savedMusInstruments, sizeof(musInstruments));
    // make sure each instrument sets its APU registers
    for (int i = 0; i < NUM_INSTRUMENTS; i++) {
        instruments[i].ctrlRegsSet = 0xff;
        musInstruments[i].ctrlRegsSet = 0xff;
    }
}

static void Sound_RunEngine(void) {
    memset(channelsInUse, 0, sizeof(channelsInUse));
    for (int i = NUM_INSTRUMENTS - 1; i >= 0; i--) {
        Sound_RunInstrument(0, &instruments[i]);
        Sound_RunInstrument(1, &musInstruments[i]);
    }
}

void Sound_Run(void) {
    static Sint16 buff0[SAMPLES_PER_FRAME * BUFFERED_FRAMES];
    static Sint16 buff1[SAMPLES_PER_FRAME * BUFFERED_FRAMES];

    // find the number of samples we need to fill up the audio buffer
    Sint32 neededSamples = (SAMPLES_PER_FRAME * BUFFERED_FRAMES) - Platform_GetQueuedSamples();

    for (Sint32 i = 0; i < neededSamples; i += SAMPLES_PER_FRAME) {
        Sound_RunEngine();
        Blargg_Apu_EndFrame();
    }
    Blargg_Apu_Samples(0, buff0, SAMPLES_PER_FRAME * BUFFERED_FRAMES);
    Sint32 outputSamples = Blargg_Apu_Samples(1, buff1, SAMPLES_PER_FRAME * BUFFERED_FRAMES);
    // mix output from both APUs together
    for (int i = 0; i < outputSamples; i++) {
        buff0[i] += buff1[i];
    }
    Platform_QueueSamples(buff0, outputSamples);
}

static Uint16 noteTbl[] = {
    0xd5c,
    0xc9c,
    0xbe8,
    0xb3c,
    0xa9a,
    0xa02,
    0x972,
    0x8ea,
    0x86a,
    0x7f2,
    0x780,
    0x714,
};

static void Sound_RunInstrument(int apu, Instrument *inst) {
    Uint8 reg2, reg3;

    // "instrument not playing" flag
    if (inst->cursor == 0xff) {
        return;
    }
    inst->timer--;
    if (inst->timer) {
        // "instrument still playing a note" flag
        reg2 = 0x6F;
    }
    else {
        setReadPtr:;
        Uint8 *read = inst->data + (inst->cursor * 2);
        getCmd:
        if (*read == 0xff) {
            inst->cursor = 0xff;
            inst->loop = 0xff;
            channelsInUse[(apu * APU_CHANNELS) + inst->channel] = 0;
            Sound_DisableChannel(apu, inst->channel);
            goto lostChannel;
        }

        Uint8 cmd = *read++;
        Uint8 param = *read++;

        // a0-af: APU register setting
        if ((cmd >= 0xa0) && (cmd < 0xb0)) {
            if (cmd & 1) {
                inst->reg1 = param;
            }
            else {
                inst->reg0 = param;
            }
            inst->cursor++;
            inst->ctrlRegsSet = 0xff;
            goto getCmd;
        }
        // b0-bf: looping/jumping
        else if ((cmd >= 0xb0) && (cmd < 0xc0)) {
            if (cmd == 0xbf) {
                inst->cursor = param;
                goto setReadPtr;
            }
            // loop over: set to 0xff (no loop) and move on
            if (!inst->loop) {
                inst->loop--;
                inst->cursor++;
                goto getCmd;
            }
            // new loop
            if (inst->loop == 0xff) {
                inst->loop = cmd & 0xf;
            }
            // loop in progress
            else {
                inst->loop--;
            }
            inst->cursor = param;
            goto setReadPtr;
        }
        // >= c0 or < a0: play note
        else {
            inst->cursor++;

            // noise channel: set registers directly
            if (inst->channel == 3) {
                reg2 = cmd;
                reg3 = 0;
            }
            // key off
            else if ((cmd & 0xf) >= 0xc) {
                Sound_DisableChannel(apu, inst->channel);
                reg2 = 0x6f;
                reg3 = 0;
            }
            else {
                Uint16 note = noteTbl[cmd & 0xf];
                Uint8 shifts = (cmd >> 4) + 1;
                note >>= shifts;
                reg2 = note & 0xff;
                reg3 = note >> 8;
            }

            inst->timer = param;
            // length counter load
            reg3 |= 8;
        }
    }
    // set up apu regs
    if (channelsInUse[(apu * APU_CHANNELS) + inst->channel]) {
        goto lostChannel;
    }
    channelsInUse[(apu * APU_CHANNELS) + inst->channel] = 0xff;
    if (reg2 == 0x6f) {
        return;
    }
    Sound_EnableChannel(apu, inst->channel);
    Uint32 regOffset = inst->channel * 4;
    if (inst->ctrlRegsSet) {
        Blargg_Apu_Write(apu, 0x4001 + regOffset, inst->reg1);
        Blargg_Apu_Write(apu, 0x4000 + regOffset, inst->reg0);
    }
    Blargg_Apu_Write(apu, 0x4002 + regOffset, reg2);
    Blargg_Apu_Write(apu, 0x4003 + regOffset, reg3);
    inst->ctrlRegsSet = 0;
    return;

    lostChannel:
    // when an instrument loses access to its channel, it has to reset its
    // registers when it regains access
    inst->ctrlRegsSet = 0xff;
}

static void Sound_DisableChannel(int apu, Uint8 channel) {
    apuStatusCopy[apu] &= ~(1 << channel);
    Blargg_Apu_Write(apu, 0x4015, apuStatusCopy[apu]);
}

static void Sound_EnableChannel(int apu, Uint8 channel) {
    apuStatusCopy[apu] |= (1 << channel);
    Blargg_Apu_Write(apu, 0x4015, apuStatusCopy[apu]);
}