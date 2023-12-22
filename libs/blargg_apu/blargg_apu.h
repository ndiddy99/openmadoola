/* blargg_apu.h: C++ -> C shim for Blargg APU code
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

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "constants.h"

/**
 * @brief Initializes the APU state
 * @param sampleRate hz
 */
void Blargg_Apu_Init(Uint32 sampleRate);

/**
 * @brief Sets the APU volume
 * @param volume percentage (0-100)
 */
void Blargg_Apu_Volume(int volume);

/**
 * @brief Clears the APU sound buffer
 */
void Blargg_Apu_ClearBuffer(void);

/**
 * @returns Number of samples in the APU sound buffer
 */
Sint32 Blargg_Apu_SamplesAvailable(void);

/**
 * @brief Writes to an APU address
 * @param addr address to write to
 * @param data value to write
 */
void Blargg_Apu_Write(Uint16 addr, Uint8 data);

/**
 * @brief Should be run at the end of each frame
 */
void Blargg_Apu_EndFrame(void);

/**
 * @brief Requests to copy a number of APU samples to the given buffer
 * @param buff Pointer to sample buffer (16-bit)
 * @param buffSize Buffer size in Sint16s (not bytes)
 * @returns Number of samples actually written to the buffer
 */
Sint32 Blargg_Apu_Samples(Sint16 *buff, Sint32 buffSize);

#ifdef __cplusplus
}
#endif
