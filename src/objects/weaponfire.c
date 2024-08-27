/* weaponfire.c: Flame sword and magic bomb fire object code
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
#include "object.h"
#include "sprite.h"
#include "weapon.h"
#include "weaponfire.h"

static void WeaponFire_CommonObj(Object *o) {
    Sprite spr = { 0 };
    if (gameFrames & 4) {
        spr.mirror = V_MIRROR;
    }
    spr.tile = 0x44;
    spr.size = SPRITE_16X16;
    if (Sprite_SetDraw(&spr, o, 0, 0)) {
        if (!Weapon_SetCollisionCoords(spr.x, spr.y)) {
            return;
        }
    }

    o->type = OBJ_NONE;
    Weapon_EraseCollisionCoords();
}

void WeaponFire_FlameSwordObj(Object *o) {
    o->timer--;
    if (o->timer) {
        WeaponFire_CommonObj(o);
    }
    else {
        o->type = OBJ_NONE;
        Weapon_EraseCollisionCoords();
    }
}

void WeaponFire_MagicBombObj(Object *o) {
    Object_CalcXYPos(o);
    WeaponFire_CommonObj(o);
}
