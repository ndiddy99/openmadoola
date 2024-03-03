/* eyemon.c: Eyemon object code
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
#include "eyemon.h"
#include "game.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

void Eyemon_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 10;
    o->type += 0x20;
    if (!o->direction) {
        o->xSpeed = 0x10;
    }
    else {
        o->xSpeed = 0xf0;
    }
    o->ySpeed = 0x10;
    o->timer = 0;
}

void HyperEyemon_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 18;
    o->type = OBJ_EYEMON;
    if (!o->direction) {
        o->xSpeed = 0x20;
    }
    else {
        o->xSpeed = 0xe0;
    }
    o->ySpeed = 0x20;
    o->timer = 0;
}

void Eyemon_Obj(Object *o) {
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 0;
    spr.mirror = 0;

    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        // make sure eyemon checks for collision periodically
        if (o->timer & 0x40) {
            o->timer--;
            if (!(o->timer & 0x3f)) {
                o->timer = (o->timer & 0x80) | 0x7;
            }
        }
        else if (o->timer & 0x80) {
            // if eyemon hit floor/ceiling, make it move horizontally
            if (Object_UpdateYPos(o)) {
                o->xSpeed = -o->xSpeed;
                o->timer = 0;
            }
            else if (!(o->timer & 0x7f)) {
                if (!Object_UpdateXPos(o)) {
                    o->timer = 7;
                    o->ySpeed = -o->ySpeed;
                    if (o->ySpeed < 0) {
                        spr.mirror |= V_MIRROR;
                    }
                }
            }
            else {
                o->timer--;
            }
        }
        // if eyemon hit a wall, make it climb the wall
        else if (Object_UpdateXPos(o)) {
            o->timer = 0xdf;
            o->ySpeed = -o->ySpeed;
        }
        else if (o->timer == 0) {
            // if eyemon is over blank space, make it move up/down after a delay
            if ((!Object_TouchingGround(o)) && (!Object_UpdateYPos(o))) {
                o->timer = 0xdf;
                o->xSpeed = -o->xSpeed;
            }
        }
        else {
            o->timer--;
        }
    }

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    spr.palette = (ABS(o->ySpeed) == 0x10) ? 3 : 0;
    Object_SetDirection(o);

    if (o->timer & 0xc0) {
        spr.tile = 0x1ce;
    }
    else {
        spr.tile = 0x1cc;
    }
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }

    Uint8 attackPower = (ABS(o->ySpeed) == 0x10) ? 18 : 10;
    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, attackPower);
}
