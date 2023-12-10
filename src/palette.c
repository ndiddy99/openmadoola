/* palette.c: Color palette
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
#include "game.h"
#include "graphics.h"

// which NES colors to use
Uint8 colorPalette[PALETTE_SIZE * 8];
// palette to use when flashTimer is nonzero
static Uint8 flashPalette[PALETTE_SIZE * 8];
Uint8 flashTimer = 0;

void Palette_Run(void) {
    if (flashTimer) {
        flashTimer--;
        for (int i = 0; i < ARRAY_LEN(colorPalette); i++) {
            flashPalette[i] = (((frameCount << 2) & 0x30) + colorPalette[i]) & 0x3f;
        }
    }
}

Uint8 *Palette_Get(void) {
    if (flashTimer) {
        return flashPalette;
    }
    else {
        return colorPalette;
    }
}