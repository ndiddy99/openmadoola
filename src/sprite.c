/* sprite.c: Sprite display code
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

#include <string.h>

#include "camera.h"
#include "constants.h"
#include "game.h"
#include "graphics.h"
#include "object.h"
#include "palette.h"
#include "sprite.h"

#define MAX_SPRITES (200)
static Sprite spriteList[MAX_SPRITES];

static int addOrder = 0;

void Sprite_SetPalette(int palnum, Uint8 *palette) {
    memcpy(&colorPalette[(palnum + 4) * PALETTE_SIZE], palette, PALETTE_SIZE);
}

void Sprite_SetAllPalettes(Uint8 *palette) {
    memcpy(colorPalette + (4 * PALETTE_SIZE), palette, PALETTE_SIZE * 4);
}

static void Sprite_8x8(int x, int y, int tilenum, int palnum, int mirror) {
    Graphics_DrawTile(x, y, tilenum, palnum + 4, mirror);
}

static void Sprite_8x16(int x, int y, int tilenum, int palnum, int mirror) {
    if (mirror & V_MIRROR) {
        Sprite_8x8(x, y, tilenum + 1, palnum, mirror);
        Sprite_8x8(x, y + 8, tilenum, palnum, mirror);
    }
    else {
        Sprite_8x8(x, y, tilenum, palnum, mirror);
        Sprite_8x8(x, y + 8, tilenum + 1, palnum, mirror);
    }
}

static void Sprite_16x16(int x, int y, int tilenum, int palnum, int mirror) {
    if (mirror & H_MIRROR) {
        Sprite_8x16(x - 8, y, tilenum + 16, palnum, mirror);
        Sprite_8x16(x, y, tilenum, palnum, mirror);
    }
    else {
        Sprite_8x16(x - 8, y, tilenum, palnum, mirror);
        Sprite_8x16(x, y, tilenum + 16, palnum, mirror);
    }
}

void Sprite_ClearList(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        spriteList[i].size = SPRITE_NONE;
    }
}

Sprite *Sprite_Get(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (spriteList[i].size == SPRITE_NONE) {
            // set size so sequential Sprite_Get calls don't return pointers to
            // the same sprite
            spriteList[i].size = SPRITE_8X16;
            // initialize the sprite to be offscreen
            spriteList[i].y = 0xffe0;
            return &spriteList[i];
        }
    }

    ERROR_MSG("Not enough sprites, increase MAX_SPRITES define in sprite.c");
    return NULL;
}

int Sprite_SetPos(Sprite *s, Object *o, Sint16 xOffset, Sint16 yOffset) {
    // fixed point to pixels
    s->x = (o->x.v - cameraX.v) >> 4;
    if (s->x < 0) {
        return 0;
    }
    s->x += xOffset;

    if (s->x >= SCREEN_WIDTH) {
        return 0;
    }

    // fixed point to pixels
    s->y = (o->y.v - cameraY.v) >> 4;
    if (s->y < 0) {
        return 0;
    }
    s->y += yOffset - 9;
    if (s->y >= SCREEN_HEIGHT) {
        return 0;
    }

    return 1;
}

void Sprite_Draw(Sprite *s, Object *o) {
    if (o && o->stunnedTimer) {
        if ((frameCount & 2) == 0) {
            return;
        }
    }

    if (addOrder) {
        for (int i = 0; i < MAX_SPRITES; i++) {
            if (spriteList[i].size == SPRITE_NONE) {
                memcpy(&spriteList[i], s, sizeof(Sprite));
                return;
            }
        }
    }

    else {
        for (int i = MAX_SPRITES - 1; i >= 0; i--) {
            if (spriteList[i].size == SPRITE_NONE) {
                memcpy(&spriteList[i], s, sizeof(Sprite));
                return;
            }
        }
    }

    ERROR_MSG("Not enough sprites, increase MAX_SPRITES define in sprite.c");
}

void Sprite_DrawDir(Sprite *s, Object *o) {
    if (!o->direction) { s->mirror |= H_MIRROR; }
    else { s->mirror &= ~H_MIRROR; }
    Sprite_Draw(s, o);
}

int Sprite_SetDraw(Sprite *s, Object *o, Sint16 xOffset, Sint16 yOffset) {

    if (Sprite_SetPos(s, o, xOffset, yOffset)) {
        Sprite_DrawDir(s, o);
        return 1;
    }

    return 0;
}

int Sprite_SetDraw16x32(Sprite *s, Object *o, Uint16 topTile, Sint16 xOffset, Sint16 yOffset) {
    s->size = SPRITE_16X16;
    if (!Sprite_SetDraw(s, o, xOffset, yOffset)) {
        o->type = OBJ_NONE;
        return 1;
    }

    s->tile = topTile;
    s->y -= 16;
    Sprite_Draw(s, o);
    return 0;
}

static void Sprite_SetDrawLargeCommon(Sprite *s, Object *o, Uint16 *tiles, Sint8 *offsets) {
    int cursor = 0;

    while (1) {
        // add the x offset if object is facing left
        if (o->direction) {
            s->x += offsets[cursor * 2];
        }
        // subtract the x offset if object is facing right
        else {
            s->x -= offsets[cursor * 2];
        }
        s->y += offsets[cursor * 2 + 1];
        cursor++;

        // tile array is terminated with a tile number of 0
        if (!tiles[cursor]) {
            break;
        }

        s->tile = tiles[cursor];
        Sprite_Draw(s, o);
    }
}

int Sprite_SetDrawLarge(Sprite *s, Object *o, Uint16 *tiles, Sint8 *offsets, Sint16 xOffset, Sint16 yOffset) {
    s->size = SPRITE_16X16;
    s->tile = *tiles;
    if (!Sprite_SetDraw(s, o, xOffset, yOffset)) {
        return 1;
    }

    Sprite_SetDrawLargeCommon(s, o, tiles, offsets);
    return 0;
}

void Sprite_SetDrawLargeAbs(Sprite *s, Object *o, Uint16 *tiles, Sint8 *offsets) {
    s->size = SPRITE_16X16;
    s->tile = *tiles;
    Sprite_Draw(s, o);
    Sprite_SetDrawLargeCommon(s, o, tiles, offsets);
}


void Sprite_EndFrame() {
    for (int i = 0; i < MAX_SPRITES; i++) {
        // NOTE: I add 1 to the y positions because the NES draws sprites one line lower than the coordinate you provide
        switch (spriteList[i].size) {
            case SPRITE_8X8:
                Sprite_8x8(spriteList[i].x, spriteList[i].y + 1, spriteList[i].tile, spriteList[i].palette, spriteList[i].mirror);
                break;

            case SPRITE_8X16:
                // NOTE: The original code subtracts 4 from the x position when drawing 8x16 sprites. I don't know why but I'll do it too
                Sprite_8x16(spriteList[i].x - 4, spriteList[i].y + 1, spriteList[i].tile, spriteList[i].palette, spriteList[i].mirror);
                break;

            case SPRITE_16X16:
                Sprite_16x16(spriteList[i].x, spriteList[i].y + 1, spriteList[i].tile, spriteList[i].palette, spriteList[i].mirror);
                break;
        }
    }

    // alternate sprite add order every frame (this makes sprites "transparent" if
    // there's sprites on top of each other)
    addOrder ^= 1;
}