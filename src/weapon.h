/* weapon.h: Weapon variables and object code
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
#include "constants.h"

// weapon indices (different from weapon object numbers)
typedef enum {
    WEAPON_SWORD = 0,
    WEAPON_FLAME_SWORD,
    WEAPON_MAGIC_BOMB,
    WEAPON_BOUND_BALL,
    WEAPON_SHIELD_BALL,
    WEAPON_SMASHER,
    WEAPON_FLASH,
    NUM_WEAPONS,
} WEAPON_NUMS;

typedef struct {
    Uint8 spawned;
    Uint8 collided;
    Sint16 x;
    Sint16 y;
} WeaponCoords;


#define MAX_WEAPON_LEVEL (3)
extern Uint8 currentWeapon;
extern Uint8 weaponDamage;
// maximum number of weapon objects
#define MAX_WEAPONS (8)
extern WeaponCoords weaponCoords[MAX_WEAPONS];

/**
 * @brief Clears all weapon objects and collision coordinates
*/
void Weapon_Init(void);

/**
 * @brief Handles spawning weapon objects
*/
void Weapon_Process(void);

/**
 * @returns the amount of MP lucia would have left if she used the current weapon 
*/
Sint16 Weapon_MagicAfterUse(void);

/**
 * @brief Sets up the collision coordinates for the current weapon object
 * @param x weapon object's x pos
 * @param y weapon object's y pos
 * @returns nonzero if the object was collided with last frame
*/
int Weapon_SetCollisionCoords(Sint16 x, Sint16 y);

/**
 * @brief erases the collision coordinates for the current weapon object
*/
void Weapon_EraseCollisionCoords(void);
