/* joy.h: Input reading code
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
#include "constants.h"

#define JOY_RIGHT   (1 << 0)
#define JOY_LEFT    (1 << 1)
#define JOY_DOWN    (1 << 2)
#define JOY_UP      (1 << 3)
#define JOY_START   (1 << 4)
#define JOY_SELECT  (1 << 5)
#define JOY_B       (1 << 6)
#define JOY_A       (1 << 7)

// currently pressed buttons
extern Uint32 joy;
// buttons that were pressed on this frame
extern Uint32 joyEdge;
// which direction the d-pad is being pressed
// 8   1   2
//     |  
// 7 - 0 - 3
//     |  
// 6   5   4
extern Uint8 joyDir;
// currently pressed buttons on actual controller, even if demo's playing
extern Uint32 joyRaw;
extern Uint32 joyEdgeRaw;

/**
 * @brief Maps a keyboard key to a joypad button
 * @param key see INPUT_BUTTON enum
 * @param joyButton JOY_UP, JOY_DOWN, etc
 */
void Joy_MapKey(int key, Uint32 joyButton);

/**
 * @brief Gives the name of the key currently associated with a given button
 * @param joyButton JOY_UP, JOY_DOWN, etc
 * @returns the name of the key that's been mapped to joyButton
 */
const char *Joy_StrKey(Uint32 joyButton);

/**
 * @brief Maps a physical game controller button to a joypad button
 * @param gamepadButton see INPUT_BUTTON enum
 * @param joyButton JOY_UP, JOY_DOWN, etc
 */
void Joy_MapGamepad(int gamepadButton, Uint32 joyButton);

/**
 * @brief Gives the name of the physical game controller button associated
 * with a given joypad button
 * @param joyButton JOY_UP, JOY_DOWN, etc
 * @returns the name of the game controller button that's been mapped to joyButton
 */
const char *Joy_StrGamepad(Uint32 joyButton);

/**
 * @brief Writes the current joypad button mappings to disk
 */
void Joy_SaveMappings(void);

/**
 * @brief Attempts to load the joypad button mappings from disk
 */
void Joy_Init(void);

/**
 * @brief updates the joy and joyEdge variables. Should be run each frame
*/
void Joy_Update(void);
