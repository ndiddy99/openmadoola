/* rng.c: Random number generator
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

#include "constants.h"

Uint8 rngVal;

Uint8 RNG_Get(void) {
    rngVal++;
    rngVal *= 5;
    return rngVal;
}

Uint8 RNG_GetWithCarry(Uint8 *carry) {
    rngVal++;
    Uint8 oldVal = rngVal;
    rngVal <<= 2;
    rngVal += oldVal;
    *carry = (rngVal < oldVal);
    return rngVal;
}