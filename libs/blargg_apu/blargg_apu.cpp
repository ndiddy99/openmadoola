/* blargg_apu.cpp: C++ -> C shim for Blargg APU code
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

#include "Nes_Apu.h"
#include "Blip_Buffer.h"

#include "blargg_apu.h"
#include "constants.h"

static Nes_Apu apus[2];
static Blip_Buffer bufs[2];
static blip_time_t clock_time;
static blip_time_t frame_length = 29780;

void Blargg_Apu_Init(Uint32 sampleRate) {
    bufs[0].clock_rate(1789773);
    bufs[0].sample_rate(sampleRate);
    bufs[1].clock_rate(1789773);
    bufs[1].sample_rate(sampleRate);

    apus[0].output(&bufs[0]);
    apus[1].output(&bufs[1]);
}

void Blargg_Apu_Volume(int volume) {
    apus[0].volume((double)volume / 100.0);
    apus[1].volume((double)volume / 100.0);
}

static blip_time_t clock_tick(void) {
    clock_time += 4;
    return clock_time;
}

void Blargg_Apu_ClearBuffer(void) {
    bufs[0].clear();
    bufs[1].clear();
}

Sint32 Blargg_Apu_SamplesAvailable(void) {
    return (Sint32)bufs[0].samples_avail();
}

void Blargg_Apu_Write(int num, Uint16 addr, Uint8 data) {
    apus[num].write_register(clock_tick(), addr, data);
}

void Blargg_Apu_EndFrame(void) {
    clock_time = 0;
    frame_length ^= 1;
    apus[0].end_frame(frame_length);
    bufs[0].end_frame(frame_length);
    apus[1].end_frame(frame_length);
    bufs[1].end_frame(frame_length);
}

Sint32 Blargg_Apu_Samples(int num, Sint16 *buff, Sint32 buffSize) {
    return (Sint32)bufs[num].read_samples(buff, buffSize);
}
