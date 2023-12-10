/* collision.c: collision handler
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
#include "object.h"
#include "sprite.h"

typedef enum {
    COLLISION_SIZE_16X16 = 0,
    COLLISION_SIZE_16X32,
    COLLISION_SIZE_32X32,
} COLLISION_SIZE_T;

/**
 * @brief Handles objects colliding with Lucia's weapons and Lucia.
 * @param o The object to check
 * @param sprite The object's sprite
 * @param size from the COLLISION_SIZE_T enum
 * @param attackPower how much damage the object does
 * @returns 0 if the object was killed, 1 if it wasn't, 2 if it hit Lucia
*/
int Collision_Handle(Object *o, Sprite *s, int size, Uint8 attackPower);

/**
 * @brief Handles objects colliding with Lucia.
 * @param o The object to check
 * @param sprite The object's sprite
 * @param size from the COLLISION_SIZE_T enum
 * @param attackPower how much damage the object does
 * @returns 2 if it hit Lucia, 1 if it didn't
*/
int Collision_WithLucia(Object *o, Sprite *s, int size, Uint8 attackPower);
