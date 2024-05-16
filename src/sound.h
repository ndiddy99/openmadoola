/* sound.h: Sound engine and output code
 * Copyright (c) 2023, 2024 Nathan Misner
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

typedef enum {
    MUS_TITLE = 0,
    MUS_ENDING,
    MUS_START,
    MUS_CLEAR,
    MUS_BOSS,
    MUS_ITEM,
    MUS_GAME_OVER,
    MUS_CAVE,
    MUS_FOREST,
    SFX_PERASKULL,
    SFX_FIREBALL,
    MUS_CASTLE,
    SFX_SWORD,
    SFX_MENU,
    SFX_LUCIA_HIT,
    SFX_BOMB,
    SFX_JUMP,
    SFX_ENEMY_HIT,
    SFX_BOMB_SPLIT,
    SFX_SHIELD_BALL,
    SFX_NOMAJI,
    SFX_BOUND_BALL,
    SFX_YOKKO_CHAN,
    SFX_ENEMY_KILL,
    SFX_ITEM,
    SFX_BOSS_KILL,
    SFX_PAUSE,
    SFX_SELECT,
    SFX_FLAME_SWORD,
    SFX_NYURU,
    SFX_JOYRAIMA,
    NUM_SOUNDS,
} SOUND_NUM;

// state of a playing instrument
typedef struct {
    Uint8 num;
    Uint8 *data;
    Uint8 channel;
    Uint16 cursor;
    Uint8 reg0;
    Uint8 reg1;
    Uint16 timer;
    Uint8 loop;
    Uint8 ctrlRegsSet;
    Uint8 lastNote;
} Instrument;

typedef struct {
    Uint8 isMusic;
    Uint8 count;
    Instrument *data;
} Sound;

extern Sound sounds[NUM_SOUNDS];

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
 * @param num Sound number we want to inspect (picks the correct APU)
 * @returns informative text about the state of the sound engine (mainly used
 * for taking up space on the sound test screen)
 */
char *Sound_GetDebugText(int num);

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
