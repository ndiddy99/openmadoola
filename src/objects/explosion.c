/* explosion.c: Explosion object code
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

#include "explosion.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

void Explosion_Obj(Object *o) {
    Sprite spr = { 0 };

    // use timer to make explosion graphic "randomly" mirror
    spr.mirror = (o->timer >> 1) & 0x3;
    o->timer--;
    // if the subtraction didn't make the timer variable wrap around
    if (!(o->timer & 0x80)) {
        if (o->timer < 10) {
            spr.tile = 0x58;
            spr.size = SPRITE_8X16;
        }
        else if (o->timer < 20) {
            spr.tile = 0x46;
            spr.size = SPRITE_16X16;
        }
        else {
            spr.tile = 0x4a;
            spr.size = SPRITE_16X16;
        }

        Sprite_SetDraw(&spr, o, 0, 0);
    }
    // randomly spawn a powerup if lucia is lucky
    else if (!(frameCount & 0xF0)) {
        o->type = OBJ_ITEM;
        o->hp = (frameCount & 3) + ITEM_RED_POTION;
        Object_InitCollision(o);
    }
    // better luck next time...
    else {
        o->type = OBJ_NONE;
    }
}
