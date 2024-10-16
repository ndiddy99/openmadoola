/* save.h: Save file handler
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
#include "buffer.h"
#include "constants.h"
#include "weapon.h"

typedef struct {
    Sint16 maxHealth;
    Sint16 maxMagic;
    Uint8 highestReachedStage;
    Sint8 keywordDisplay;
    Uint8 orbCollected;
    Uint8 weaponLevels[NUM_WEAPONS];
    Uint8 bootsLevel;
    Uint8 itemsCollected[8];
    Uint8 bossDefeated[16];
} SaveData;

extern SaveData *sd;

/**
 * @brief Loads any save files from disk. Should be run at startup.
 */
void Save_Init(void);

/**
 * @brief Saves game state to disk.
 */
void Save_SaveFile(void);

/**
 * @brief Appends the current game state to the given Buffer.
 * @param buf Buffer to write to (must be initialized)
 */
void Save_Serialize(Buffer *buf);

/**
 * @brief Sets the current game state to the contents of the given byte array
 * @param data Data to set game state to (must have been created by Save_Serialize)
 * @returns number of bytes deserialized
 */
int Save_Deserialize(Uint8 *data);

/**
 * @brief Displays the save file screen.
 */
void Save_Screen(void);