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

static Nes_Apu apu;
static Blip_Buffer buf;
static blip_time_t clock_time;
static blip_time_t frame_length = 29780;

void Blargg_Apu_Init(Uint32 sampleRate) {
    apu.output(&buf);
    buf.clock_rate(1789773);
    buf.sample_rate(sampleRate);
}

static blip_time_t clock_tick(void) {
    clock_time += 4;
    return clock_time;
}

void Blargg_Apu_ClearBuffer(void) {
    buf.clear();
}

Sint32 Blargg_Apu_SamplesAvailable(void) {
    return (Sint32)buf.samples_avail();
}

void Blargg_Apu_Write(Uint16 addr, Uint8 data) {
    apu.write_register(clock_tick(), addr, data);
}

void Blargg_Apu_EndFrame(void) {
    clock_time = 0;
    frame_length ^= 1;
    apu.end_frame(frame_length);
    buf.end_frame(frame_length);
}

Sint32 Blargg_Apu_Samples(Sint16 *buff, Sint32 buffSize) {
    return (Sint32)buf.read_samples(buff, buffSize);
}
