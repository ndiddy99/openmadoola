/* nigito.c: Nigito object code
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
#include "constants.h"
#include "fireball.h"
#include "map.h"
#include "nigito.h"
#include "object.h"
#include "rng.h"

void Nigito_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 80;
    o->type += 0x20;
    o->timer = 4;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x10 : -0x10;
    o->ySpeed = 0;
}

void Nigito_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    Fireball_Spawn(0x3f, o);
    // if nigito isn't jumping
    if (!Object_HandleJump(o)) {
        // handle walking off ledges
        if (!Object_TouchingGround(o)) {
            if (o->timer) {
                o->timer--;
                o->xSpeed = -o->xSpeed;
                o->direction ^= 0x80;
                o->ySpeed = 0;
                o->y.f.l &= 0x80;
                Object_MoveTowardsLucia(o);
            }
            else {
                Object_ApplyGravity(o);
                Object_CheckForWall(o);
                if (Object_UpdateYPos(o)) {
                    o->timer = 4;
                    o->ySpeed = 0;
                    o->y.f.l &= 0x80;
                    Object_MoveTowardsLucia(o);
                }
            }
        }
        // walk forward, jump if hit a wall
        else {
            Object_JumpIfHitWall(o);
        }
    }

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 0;
    if (o->x.f.h & 1) {
        spr.tile = 0x1C2;
    }
    else {
        spr.tile = 0x1E2;
    }

    if (!Sprite_SetDraw16x32(&spr, o, spr.tile - 2, 0, 0)) {
        return;
    }

    Collision_Handle(o, &spr, COLLISION_SIZE_16X32, 40);
}