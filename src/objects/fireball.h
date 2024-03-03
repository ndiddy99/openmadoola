/* fireball.h: Fireball object code
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
#include "object.h"

/**
 * @brief Fireball object code
 * @param o object pointer
*/
void Fireball_Obj(Object *o);

/**
 * @brief Spawns a fireball object
 * @param mask Bitmask to AND with the rng value to determine whether the fireball spawns
 * @param parent The object that's spawning the fireball
*/
void Fireball_Spawn(Uint8 mask, Object *parent);
