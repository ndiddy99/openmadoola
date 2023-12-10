/* graphics.c: Graphics primitives
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

#include <stdio.h>

#include "constants.h"
#include "graphics.h"
#include "palette.h"
#include "platform.h"
#include "rom.h"

#define TILE_PACKED_SIZE (16)
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)

// NES color palette
#define PALETTE_FILE "nes.pal"
#define NES_PALETTE_SIZE 64
// which RGB colors map to each NES color
static Uint32 nesPalette[NES_PALETTE_SIZE];
static Uint8 *dispPalette;

// 8bpp chunky version of chrRom
static Uint8 *chr_data;

static Uint16 *screen;

int Graphics_Init(void) {
    // convert planar 2bpp to chunky 8bpp
    chr_data = malloc(chrRomSize * 4);
    if (!chr_data) {
        ERROR_MSG("Out of memory");
        goto error;
    }
    int chrCursor = 0;
    for (int i = 0; i < (chrRomSize / TILE_PACKED_SIZE); i++) {
        int tilePos = i * TILE_PACKED_SIZE;
        // each tile is 8x8 pixels
        for (int y = 0; y < TILE_HEIGHT; y++) {
            uint8_t lowByte = chrRom[tilePos + y];
            uint8_t highByte = chrRom[tilePos + y + 8];
            for (int x = 0; x < TILE_WIDTH; x++) {
                uint8_t pixel = 0;
                if (lowByte & 0x80) {
                    pixel |= 1;
                }
                lowByte <<= 1;

                if (highByte & 0x80) {
                    pixel |= 2;
                }
                highByte <<= 1;

                chr_data[chrCursor++] = pixel;
            }
        }
    }

    // load palette file
    FILE *fp = fopen(PALETTE_FILE, "rb");
    if (!fp) {
        ERROR_MSG("Couldn't open " PALETTE_FILE);
        goto error;
    }

    for (int i = 0; i < NES_PALETTE_SIZE; i++) {
        Uint8 r = fgetc(fp);
        Uint8 g = fgetc(fp);
        Uint8 b = fgetc(fp);

        nesPalette[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
    fclose(fp);

    return 1;

    error:
    if (chr_data) { free(chr_data); }
    return 0;
}

void Graphics_StartFrame(void) {
    screen = Platform_GetNESBuffer();
    Palette_Run();
    dispPalette = Palette_Get();
    for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) {
        screen[i] = dispPalette[0];
    }
}

void Graphics_DrawTile(int x, int y, int tilenum, int palnum, int mirror) {
    int tileOffset = tilenum * TILE_SIZE;
    Uint8 *palette = dispPalette + (palnum * PALETTE_SIZE);

    // don't draw the tile at all if it's entirely offscreen
    if ((x < -TILE_WIDTH) || (x >= SCREEN_WIDTH) || (y < -TILE_HEIGHT) || (y >= SCREEN_HEIGHT)) {
        return;
    }

    // write tile to screen, making sure we don't go out of bounds
    for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
        int yLoc = y + yOffset;
        if ((yLoc >= 0) && (yLoc < SCREEN_HEIGHT)) {
            for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
                int xLoc = x + xOffset;
                if ((xLoc >= 0) && (xLoc < SCREEN_WIDTH)) {
                    int xSrc, ySrc;
                    if (mirror & H_MIRROR) {
                        xSrc = (TILE_WIDTH - 1) - xOffset;
                    }
                    else {
                        xSrc = xOffset;
                    }

                    if (mirror & V_MIRROR) {
                        ySrc = (TILE_HEIGHT - 1) - yOffset;
                    }
                    else {
                        ySrc = yOffset;
                    }
                    Uint8 palIndex = chr_data[tileOffset + (ySrc * TILE_WIDTH) + xSrc];
                    if (palIndex) {
                        Uint8 color = palette[chr_data[tileOffset + (ySrc * TILE_WIDTH) + xSrc]];
                        screen[yLoc * SCREEN_WIDTH + xLoc] = color;
                    }
                }
            }
        }
    }
}
