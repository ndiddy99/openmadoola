/* sound.h: Sound engine and output code
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

#define MUS_TITLE	    0x0
#define MUS_ENDING	    0x1
#define MUS_START	    0x2
#define MUS_CLEAR	    0x3
#define MUS_BOSS	    0x4
#define MUS_ITEM	    0x5
#define MUS_GAME_OVER	0x6
#define MUS_CAVE	    0x7
#define MUS_OVERWORLD	0x8
#define SFX_PERASKULL	0x9
#define SFX_FIREBALL	0xa
#define MUS_CASTLE	    0xb
#define SFX_SWORD	    0xc
#define SFX_MENU	    0xd
#define SFX_LUCIA_HIT	0xe
#define SFX_BOMB	    0xf
#define SFX_JUMP	    0x10
#define SFX_ENEMY_HIT	0x11
#define SFX_BOMB_SPLIT	0x12
#define SFX_SHIELD_BALL	0x13
#define SFX_NOMAJI	    0x14
#define SFX_BOUND_BALL	0x15
#define SFX_YOKKO_CHAN	0x16
#define SFX_ENEMY_KILL	0x17
#define SFX_ITEM	    0x18
#define SFX_BOSS_KILL	0x19
#define SFX_PAUSE	    0x1a
#define SFX_SELECT	    0x1b
#define SFX_FLAME_SWORD	0x1c
#define SFX_NYURU	    0x1d
#define SFX_JOYRAIMA	0x1e

/**
 * @brief Initializes sound output
 * @returns 1 on success, 0 on failure
*/
int Sound_Init(void);

/**
 * @brief Sets the volume.
 * @param vol Percentage (0-100)
 * @returns the new volume value
 */
int Sound_SetVolume(int vol);

/**
 * @returns the current volume.
 */
int Sound_GetVolume(void);

/**
 * @brief Resets the sound engine (so nothing is playing)
*/
void Sound_Reset(void);

/**
 * @brief Plays a sound number
 * @param num the sound to play
*/
void Sound_Play(int num);

/**
 * @brief Saves the sound engine state
*/
void Sound_SaveState(void);

/**
 * @brief Loads the sound engine state
*/
void Sound_LoadState(void);

/**
 * @brief Runs the sound engine and outputs audio, should be called each frame
 * you want audio playing
*/
void Sound_Run(void);
