/* soundtest.h: Sound test screen
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
#pragma once

/**
 * @brief Displays sound test screen
 */
void SoundTest_Run(void);

/**
 * @brief Compiles the given MML file
 * @param mmlPath MML file to play
 * @returns 1 if the file compiled successfully, 0 otherwise
 */
int SoundTest_RunStandaloneInit(char *mmlPath);

/**
 * @brief Plays an MML file. Needs to be run as a task after SoundTest_RunStandaloneInit
 */
void SoundTest_RunStandaloneTask(void);
