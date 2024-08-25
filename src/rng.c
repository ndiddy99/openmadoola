/* rng.c: Random number generator
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
#include <stdlib.h>
#include <time.h>

#include "constants.h"

Uint8 rngVal;

void RNG_Seed(void) {
    srand((unsigned int)time(NULL));
    rngVal = (Uint8)rand();
}

Uint8 RNG_Get(void) {
    rngVal++;
    rngVal *= 5;
    return rngVal;
}

Uint16 RNG_GetWithCarry(void) {
    rngVal++;
    Uint16 temp = (((Uint16)rngVal << 2) & 0xff) + rngVal;
    rngVal = (Uint8)temp;
    return temp;
}