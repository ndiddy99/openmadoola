/* rng.h: Random number generator
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

extern Uint8 rngVal;

/**
 * @brief Seeds the RNG from current time. Necessary because the demos overwrite
 * the RNG seed and I don't want that to make every new game start with the same
 * enemy pattern.
 */
void RNG_Seed(void);

/**
* @brief Updates the RNG value
* @returns the new RNG value
*/
Uint8 RNG_Get(void);

/**
 * @brief Updates the RNG value. The 9th bit in the return value is set to the
 * carry flag.
 * @returns the new rng value (bit 9 = carry)
 */
Uint16 RNG_GetWithCarry(void);
