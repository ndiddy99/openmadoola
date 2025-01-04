/* darutos.c: Darutos object code
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

#include "camera.h"
#include "collision.h"
#include "darutos.h"
#include "fireball.h"
#include "game.h"
#include "rng.h"
#include "sound.h"
#include "sprite.h"
#include "object.h"

static Sint8 darutosOffsets1[] = {
    0xf0, 0x00,
    0x00, 0xf0,
    0x10, 0x00,
    0x0c, 0x00,
};

// standing
static Uint16 darutos1[] = {
    0x3a6, 0x386, 0x384, 0x3a4, 0x00, 0x3c6,
};

// jumping
static Uint16 darutos2[] = {
    0x3be, 0x39e, 0x39c, 0x3bc, 0x00, 0x3dc,
};

static Sint8 darutosOffsets2[] = {
    0x10, 0x00, 
    0x00, 0x10, 
    0x0c, 0x00,
};

static Uint16 darutos3[] = {
    0x390, 0x3b0, 0x3b2, 0x00, 0x3d2, 0x3d0,
};

static Uint16 darutos4[] = {
    0x398, 0x3b8, 0x3ba, 0x00, 0x3da, 0x3d8,
};

// Keeps track of whether Lucia killed Darutos.
// NOTE: The original game used orbCollected for this purpose, but that led to
// a bug where killing Darutos and then dying to his fireball would allow Lucia
// to skip fighting Darutos since orbCollected doesn't get cleared when
// continuing.
Uint8 darutosKilled;

void Darutos_InitObj(Object *o) {
    // darutos's position is hardcoded
    o->x.v = 0x3a80;
    o->y.v = 0x1f80;
    o->hp = 255;
    o->type += 0x20;
    o->timer = 0x2f;
    o->direction = DIR_LEFT;
    o->xSpeed = 0xe0;
    o->ySpeed = 0x38;
}

void Darutos_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }

    if (!(o->timer & 0x20)) {
        o->timer &= 0xbf;
        if (!(o->timer & 0x10)) {
            Object_CalcXPos(o);
            Object_CalcYPos(o);
            if (!(RNG_Get() & 3)) {
                o->timer = (o->timer ^ 0x80) & 0xbf;
            }
            o->timer--;
            if (!(o->timer & 0xf)) {
                if (o->ySpeed >= 0) {
                    o->direction ^= 0x80;
                    o->xSpeed = -o->xSpeed;
                    o->timer |= 0x2f;
                }
                else {
                    o->timer = (o->timer | 0x1f) & 0xdf;
                }
            }
        }
        else {
            Object_CalcXPos(o);
            o->y.f.h -= 2;
            Fireball_Spawn(7, o);
            o->y.f.h += 2;
            goto checkY;
        }
    }
    else {
        o->timer |= 0x40;
    checkY:
        if (!(RNG_Get() & 3)) {
            if (((o->timer & 0xf) < 5) || (o->timer & 0x10)) {
                o->timer ^= 0x80;
            }
        }
        o->timer--;
        if (!(o->timer & 0xf)) {
            o->ySpeed = -o->ySpeed;
            o->timer = (o->timer & 0xc0) | 0xf;
        }
    }

    Sprite spr = { 0 };
    Sint16 dispOffsetX = 0;
    Uint16 *frame;
    spr.size = SPRITE_16X16;
    spr.palette = 2;
    // --- draw darutos's legs ---
    if (!(o->timer & 0x40)) {
        dispOffsetX = (o->direction == DIR_RIGHT) ? -0x10 : 0x10;
        frame = darutos2;
    }
    else {
        frame = darutos1;
    }

    // NOTE: The original game used its equivalent of Sprite_SetDrawLarge here,
    // but that made Darutos not get drawn unless he was at least halfway
    // onscreen, making it harder to judge what side he was on. Instead we
    // calculate the sprite position manually and then use Sprite_SetDrawLargeAbs.
    if (gameType != GAME_TYPE_ORIGINAL) {
        spr.x = ((o->x.v - cameraX.v) >> 4) + dispOffsetX;
        spr.y = ((o->y.v - cameraY.v) >> 4) - 9;
        spr.mirror = (o->direction == DIR_LEFT) ? 0 : H_MIRROR;
        Sprite_SetDrawLargeAbs(&spr, o, frame, darutosOffsets1);
    }
    else {
        if (!Sprite_SetDrawLarge(&spr, o, frame, darutosOffsets1, dispOffsetX, 0)) {
            return;
        }
    }

    // --- draw darutos's tail ---
    // get tile after the "0x00"
    spr.tile = frame[5];
    if (o->timer & 0x40) {
        spr.y += 0x10;
        spr.x += (o->direction == DIR_RIGHT) ? -4 : 4;
        spr.size = SPRITE_16X16;
        Sprite_Draw(&spr, o);
        spr.y -= 0x10;
    }
    else {
        spr.size = SPRITE_8X16;
        Sprite_Draw(&spr, o);
    }
    // --- draw darutos's head and wing ---
    spr.y -= 0x20;
    Sint16 xOffset = (o->timer & 0x40) ? -0x18 : -0x1c;
    if (o->direction == DIR_RIGHT) { xOffset = -xOffset; }
    spr.x += xOffset;
    frame = (!(o->timer & 0x80)) ? darutos3 : darutos4;
    spr.size = SPRITE_16X16;
    Sprite_SetDrawLargeAbs(&spr, o, frame, darutosOffsets2);
    // --- draw the back of darutos's wing ---
    // get tile after the "0x00"
    spr.tile = frame[4];
    spr.size = SPRITE_8X16;
    Sprite_Draw(&spr, o);
    spr.tile = frame[5];
    spr.y -= 0x10;
    Sprite_Draw(&spr, o);
    // --- draw darutos's mouth ---
    spr.tile = (!(o->x.f.h & 1)) ? 0x380 : 0x388;
    spr.x += (o->direction == DIR_RIGHT) ? 0x28 : -0x28;
    Sprite_Draw(&spr, o);
    // --- draw darutos's hand ---
    spr.y += 0x10;
    spr.tile = (!(o->x.f.l & 0x80)) ? 0x382 : 0x38a;
    Sprite_Draw(&spr, o);
    // draw darutos's chest and other hand ---
    spr.tile += 0x10;
    spr.x += (o->direction == DIR_RIGHT) ? -0xc : 0xc;
    spr.size = SPRITE_16X16;
    Sprite_Draw(&spr, o);
    spr.y -= 0x10;

    if (!Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 95)) {
        darutosKilled = 1;
        Game_PlayRoomSong();
        Sound_Play(SFX_BOSS_KILL);
    }
}