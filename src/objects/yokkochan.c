/* yokkochan.c: Yokko-chan object code
 * Copyright (c) 2023 Nathan Misner
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

#include "collision.h"
#include "game.h"
#include "map.h"
#include "object.h"
#include "rng.h"
#include "sound.h"
#include "sprite.h"
#include "yokkochan.h"

void YokkoChan_InitObj(Object *o) {
    // don't spawn if keyword has already been displayed
    if (keywordDisplay) {
        o->type = OBJ_NONE;
        return;
    }
    OBJECT_CHECK_SPAWN(o);
    o->hp = 1;
    o->type += 0x20;
    o->timer = 2;
    if (!o->direction) {
        o->xSpeed = 0xc;
    }
    else {
        o->xSpeed = 0xf4;
    }
    o->ySpeed = 0xb0;
}

void YokkoChan_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        if (!(RNG_Get() & 0x1f)) {
            Sound_Play(SFX_YOKKO_CHAN);
        }
        if (o->timer) {
            Object_CheckForDrop(o);
            Object_CheckForWall(o);
            o->timer += 2;
        }
        else {
            Object_ApplyGravity(o);
            Object_CheckForWall(o);
            if (Object_UpdateYPos(o)) {
                if (Object_TouchingGround(o)) {
                    o->y.f.l &= 0x80;
                    o->timer += 2;
                    o->ySpeed = 0xb0;
                }
                else {
                    o->ySpeed = 0;
                }
            }
        }
    }

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 0;
    if ((o->timer) && (o->x.f.l & 0x40)) {
        spr.tile = 0x1ac;
    }
    else {
        spr.tile = 0x18c;
    }
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }
    // disable keyword if lucia kills yokko-chan
    if (!Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 255)) {
        keywordDisplay = -1;
    }
}