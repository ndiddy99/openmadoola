/* rng.h: Random number generator
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

extern Uint8 rngVal;

/**
* @brief Updates the RNG value
* @returns the new RNG value
*/
Uint8 RNG_Get(void);

/**
 * @brief Updates the RNG value and provides the code the value that would be
 * in the 6502 carry flag (necessary because some code makes use of that)
 * @param carry pointer to carry flag variable
 * @returns the new rng value, sets carry to the carry flag value
 */
Uint8 RNG_GetWithCarry(Uint8 *carry);
