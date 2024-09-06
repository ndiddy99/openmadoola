/* system.h: Engine management code
 * Copyright (c) 2023 Nathan Misner
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

#pragma once

/**
 * @brief Initializes platform & engine code
 * @returns 0 on failure, nonzero on success
 */
int System_Init(void);

/**
 * @brief Should be run by "user code" at the start of each frame. Tells the
 * platform & graphics code to start the frame and reads input.
 */
void System_StartFrame(void);

/**
 * @brief Should be run by "user code" at the end of each frame. Runs the sound
 * engine and tells the platform code to end the frame.
 */
void System_EndFrame(void);

/**
 * @brief Runs platform code and jumps to the current task.
 */
void System_GameLoop(void);
