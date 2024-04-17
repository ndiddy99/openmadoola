/* graphics.c: Graphics primitives
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
#include <string.h>

#include "constants.h"
#include "graphics.h"
#include "palette.h"
#include "platform.h"
#include "rom.h"

#define TILE_PACKED_SIZE (16)
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)

// 8bpp chunky version of chrRom
static Uint8 *chrData;
// the palette we're using to draw this frame
static Uint8 *drawPalette;
// where we're drawing to
static Uint8 *screen;

int Graphics_Init(void) {
    // convert planar 2bpp to chunky 8bpp
    chrData = malloc(chrRomSize * 4);
    if (!chrData) {
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

                chrData[chrCursor++] = pixel;
            }
        }
    }
    return 1;

error:
    if (chrData) { free(chrData); }
    return 0;
}

void Graphics_StartFrame(void) {
    screen = Platform_GetFramebuffer();
    drawPalette = Palette_Run();
    memset(screen, colorPalette[0], FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
}

void Graphics_DrawTile(int x, int y, int tilenum, int palnum, int mirror) {
    // don't draw the tile at all if it's entirely offscreen
    if ((x < -TILE_WIDTH) || (x >= SCREEN_WIDTH) || (y < -TILE_HEIGHT) || (y >= SCREEN_HEIGHT)) {
        return;
    }

    // write tile to screen
    int tileOffset = tilenum * TILE_SIZE;
    Uint8 *palette = drawPalette + (palnum * PALETTE_SIZE);
    // the nes framebuffer has an extra tile row/column around it to allow for 
    // drawing to it without checking the tile bounds
    x += TILE_WIDTH;
    y += TILE_HEIGHT;

    switch (mirror) {
    case 0: // no mirror
        for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
            int yDst = y + yOffset;
            for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
                Uint8 palIndex = chrData[tileOffset++];
                if (palIndex) {
                    int xDst = x + xOffset;
                    screen[yDst * FRAMEBUFFER_WIDTH + xDst] = palette[palIndex];
                }
            }
        }
        break;

    case H_MIRROR:
        for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
            int yDst = y + yOffset;
            for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
                Uint8 palIndex = chrData[tileOffset++];
                if (palIndex) {
                    int xDst = ((TILE_WIDTH - 1) - xOffset) + x;
                    screen[yDst * FRAMEBUFFER_WIDTH + xDst] = palette[palIndex];
                }
            }
        }
        break;

    case V_MIRROR:
        for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
            int yDst = ((TILE_HEIGHT - 1) - yOffset) + y;
            for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
                Uint8 palIndex = chrData[tileOffset++];
                if (palIndex) {
                    int xDst = x + xOffset;
                    screen[yDst * FRAMEBUFFER_WIDTH + xDst] = palette[palIndex];
                }
            }
        }
        break;

    case (H_MIRROR | V_MIRROR):
        for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
            int yDst = ((TILE_HEIGHT - 1) - yOffset) + y;
            for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
                Uint8 palIndex = chrData[tileOffset++];
                if (palIndex) {
                    int xDst = ((TILE_WIDTH - 1) - xOffset) + x;
                    screen[yDst * FRAMEBUFFER_WIDTH + xDst] = palette[palIndex];
                }
            }
        }
        break;
    }
}
