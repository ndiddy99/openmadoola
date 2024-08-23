/* demo.h: Demo recording/playback code
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
 * @brief Starts a demo recording.
 */
void Demo_Record(void);

/**
 * @brief Starts demo playback. Runs for a single stage.
 * @param filename demo file to load
 * @returns nonzero if demo played successfully, zero otherwise
 */
int Demo_Playback(char *filename);

/**
 * @returns nonzero if demo is recording, zero otherwise 
 */
int Demo_Recording(void);

/**
 * @returns nonzero if demo is playing, zero otherwise 
 */
int Demo_Playing(void);

/**
 * @brief Records input from a given frame. Should be run every frame you're recording a demo.
 * Does nothing if you're not recording.
 * @param input joypad input
 */
void Demo_RecordInput(Uint32 input);

/**
 * @brief Retrieves input for a given frame. Should be run every frame you're playing back a demo.
 * @returns input for the current frame, or zero if not playing back
 */
Uint32 Demo_GetInput(void);

/**
 * @brief Saves a demo recording to disk. Does nothing unless Demo_Record has previously been run.
 * @param filename what to save the demo as
 */
void Demo_Save(char *filename);
