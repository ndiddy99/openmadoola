/* boundball.c: Bound ball object code
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

#include "constants.h"
#include "game.h"
#include "joy.h"
#include "object.h"
#include "rng.h"
#include "sprite.h"
#include "weapon.h"

void BoundBall_Obj(Object *o) {
    o->timer--;
    if (!o->timer) {
        goto eraseObj;
    }

    Object_Bounce(o);
    Sint8 dispOffsetX = (RNG_Get() & 31) - 16;
    Sint8 dispOffsetY = (RNG_Get() & 31) - 16;
    Sprite spr = { 0 };
    spr.palette = (frameCount >> 1) & 3;
    if (weaponLevels[WEAPON_BOUND_BALL] >= 3) {
        spr.tile = 0x4E;
        spr.size = SPRITE_16X16;
        if (!Sprite_SetDraw(&spr, o, dispOffsetX, dispOffsetY)) {
            goto eraseObj;
        }
    }
    else {
        spr.tile = 0x62;
        spr.size = SPRITE_8X16;
        if (!Sprite_SetDraw(&spr, o, dispOffsetX, dispOffsetY)) {
            goto eraseObj;
        }
    }

    if (Weapon_SetCollisionCoords(spr.x, spr.y)) {
        goto eraseObj;
    }
    return;

eraseObj:
    Weapon_EraseCollisionCoords();
    o->type = OBJ_NONE;
}