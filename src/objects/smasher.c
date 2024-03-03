/* smasher.c: Smasher object code
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
#include "rng.h"
#include "sound.h"
#include "sprite.h"
#include "weapon.h"

static Uint8 smasherLimits[] = {
    0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F,
};

static Sint8 smasherCenters[] = {
    0x08, 0x10, 0x20, 0x40, 0x80, 0x80, 0x40, 0x20, 0x10, 0x08,
};

static Uint8 smasherDelays[] = {
    10, 6, 3, 1,
};

static int Smasher_FindEnemy(void) {
    for (int i = 9; i < MAX_OBJECTS; i++) {
        if (objects[i].type) {
            return i;
        }
    }

    return 0;
}

void Smasher_Obj(Object *o) {
    Sound_Play(SFX_ENEMY_HIT);
    if (o->hp < 0) {
        // have the smasher damage the enemy it homed in to after the expand/contract animation
        o->type = OBJ_SMASHER_DAMAGE;
        o->timer = 1;
        return;
    }

    o->timer--;
    if (o->timer == 0) {
        // higher smasher level = shorter delay
        o->timer = smasherDelays[weaponLevels[WEAPON_SMASHER] & 3];
        o->hp--;
        if (o->hp < 0) { return; }
    }

    // the object to center the smasher around
    int objectCenterNum = 0;

    if (o->hp < 6) {
        objectCenterNum = Smasher_FindEnemy();
        if (!objectCenterNum) {
            // delete smasher object before the "shrink" part if there's no enemies onscreen
            o->type = OBJ_NONE;
            return;
        }
    }

    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.mirror = frameCount & 3;
    spr.tile = 0x44;
    o->x = objects[objectCenterNum].x;
    o->y = objects[objectCenterNum].y;
    Sprite_SetPos(&spr, o, 0, 0);

    Sint16 spriteX = spr.x;
    Sint16 spriteY = spr.y;
    Uint8 limitMask = smasherLimits[o->hp];
    Sint8 center = smasherCenters[o->hp];
    for (int i = 0; i < 8; i++) {
        spr.x = (Sint16)(RNG_Get() & limitMask) - center + spriteX;
        spr.y = (Sint16)(RNG_Get() & limitMask) - center + spriteY;
        Sprite_Draw(&spr, NULL);
    }
}

void Smasher_DamageObj(Object *o) {
    if (o->timer) {
        o->timer--;
        Sprite spr = { 0 };
        Sprite_SetPos(&spr, o, 0, 0);
        Weapon_SetCollisionCoords(spr.x, spr.y);
    }
    else {
        Weapon_EraseCollisionCoords();
        o->type = OBJ_NONE;
    }
}