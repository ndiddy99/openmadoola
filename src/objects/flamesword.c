/* flamesword.c: Flame sword object code
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
#include "flamesword.h"
#include "game.h"
#include "object.h"
#include "sprite.h"
#include "weapon.h"

void FlameSword_Obj(Object *o) {
    Object_CalcXYPos(o);
    Sprite spr = { 0 };

    if (frameCount & 0x10) {
        spr.mirror = V_MIRROR;
    }
    spr.tile = 0x44;
    spr.size = SPRITE_16X16;
    Sprite_SetDraw(&spr, o, 0, 0);
    // these got set by Sprite_SetDraw
    Weapon_SetCollisionCoords(spr.x, spr.y);
    
    o->timer--;
    if (o->timer) {
        // spawn a new flame object here every other frame (this makes a
        // "flames sliding out from the sword" animation)
        if (o->timer & 1) {
            Object *fireObj = Object_FindNext(3, 9);
            if (fireObj) {
                fireObj->x = o->x;
                fireObj->y = o->y;
                fireObj->type = OBJ_FLAME_SWORD_FIRE;
                fireObj->timer = o->timer;
            }
        }
    }
    else {
        o->type = OBJ_NONE;
        Weapon_EraseCollisionCoords();
    }
}
