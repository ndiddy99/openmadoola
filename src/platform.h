/* platform.h: Platform specific code
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
#include "constants.h"

/**
 * @brief Initializes platform specific code.
 * @returns nonzero on success, zero on failure.
 */
int Platform_Init(void);

/**
 * @brief Should be run at the start of each frame.
 */
void Platform_StartFrame(void);

/**
 * @brief Should be run at the end of each frame.
 */
void Platform_EndFrame(void);

/**
 * @brief Gets access to the framebuffer.
 * @returns a pointer to the NES framebuffer pixels (9-bit)
 */
Uint16 *Platform_GetNESBuffer(void);

/**
 * @returns the current video scale
 */
int Platform_GetVideoScale(void);

/**
 * @brief Requests changing the video scale. Doesn't work in fullscreen mode.
 * @param requested The requested scale
 * @returns The value the scale was set to
 */
int Platform_SetVideoScale(int requested);

/**
 * @brief Requests setting the video mode
 * @param requested nonzero = fullscreen, zero = windowed
 */
void Platform_SetFullscreen(int requested);

/**
 * @brief Gets the video mode
 * @returns nonzero = fullscreen, zero = windowed
 */
int Platform_GetFullscreen(void);

/**
 * @brief Queues the given audio samples to play
 * @param samples The buffer of samples to queue
 * @param count The number of samples in the buffer
 */
void Platform_QueueSamples(Sint16 *samples, int count);

/**
 * @returns the number of queued audio samples
 */
int Platform_GetQueuedSamples(void);