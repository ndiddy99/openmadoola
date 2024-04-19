/* palette.c: Color palette
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

#include <stdio.h>

#include "constants.h"
#include "file.h"
#include "game.h"
#include "graphics.h"
#include "palette.h"
#include "platform.h"

// maps each of the 64 NES colors to an ARGB value
Uint32 nesToRGB[64];
// which NES colors to use
Uint8 colorPalette[PALETTE_SIZE * 8];
// palette to use when flashTimer is nonzero
static Uint8 flashPalette[PALETTE_SIZE * 8];
Uint8 flashTimer = 0;

int Palette_Init(void) {
    FILE *fp = File_OpenResource("nes.pal");
    if (!fp) {
        Platform_ShowError("Couldn't open nes.pal");
        return 0;
    }

    for (int i = 0; i < ARRAY_LEN(nesToRGB); i++) {
        Uint8 r = fgetc(fp);
        Uint8 g = fgetc(fp);
        Uint8 b = fgetc(fp);

        nesToRGB[i] = ((Uint32)0xFF << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | (Uint32)b;
    }
    fclose(fp);
    return 1;
}

Uint8 *Palette_Run(void) {
    if (flashTimer) {
        flashTimer--;
        for (int i = 0; i < ARRAY_LEN(colorPalette); i++) {
            flashPalette[i] = (((frameCount << 2) & 0x30) + colorPalette[i]) & 0x3f;
        }
        return flashPalette;
    }
    return colorPalette;
}
