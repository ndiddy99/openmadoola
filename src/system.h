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
 * @brief Should be run by "user code" at the start of each frame
 */
void System_StartFrame(void);

/**
 * @brief Should be run by "user code" at the end of each frame
 */
void System_EndFrame(void);
