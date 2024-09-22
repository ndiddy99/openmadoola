/* shieldball.c: Shield ball object code
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
#include "lucia.h"
#include "object.h"
#include "shieldball.h"
#include "sprite.h"
#include "weapon.h"

// contains half of 3 different sine curves (other half is mirrored by ShieldBall_Sin)
static Sint8 sinTbl[] = {
    0x00, 0x08, 0x0F, 0x16, 0x1C, 0x21, 0x25, 0x27, 0x28, 0x27, 0x25, 0x21, 0x1C, 0x16, 0x0F, 0x08,
    0x00, 0x06, 0x0C, 0x12, 0x17, 0x1B, 0x1E, 0x1F, 0x20, 0x1F, 0x1E, 0x1B, 0x17, 0x12, 0x0C, 0x06,
    0x00, 0x05, 0x09, 0x0D, 0x11, 0x14, 0x16, 0x18, 0x18, 0x18, 0x16, 0x14, 0x11, 0x0D, 0x09, 0x05,
};

static Sint8 shieldBallOffsetTbl[] = {
    0x00, 0x10, 0x20, 0x10,
};

static Sint8 ShieldBall_GetOffset(Uint8 num) {
    Uint8 index = (gameFrames >> 2) & 3;
    return shieldBallOffsetTbl[index] + num;
}

static Sint8 ShieldBall_Sin(Uint8 index) {
    index &= 31;
    if (index < 15) {
        return sinTbl[ShieldBall_GetOffset(index)];
    }
    else {
        index &= 15;
        return -sinTbl[ShieldBall_GetOffset(index)];
    }
}

static Sint8 ShieldBall_Cos(Uint8 index) {
    if (gameType == GAME_TYPE_ARCADE) {
        return ShieldBall_Sin(index + (gameFrames / 2));
    }
    else {
        return ShieldBall_Sin(index + 8);
    }
}

void ShieldBall_Obj(Object *o) {
    if ((gameFrames & 3) == 0) {
        o->timer--;
        if (o->timer == 0) {
            Weapon_EraseCollisionCoords();
            o->type = OBJ_NONE;
            return;
        }
    }

    Uint8 posOffset = (currObjectIndex << 2) + gameFrames;
    Sprite spr = { 0 };
    spr.size = SPRITE_8X16;

    spr.x = ShieldBall_Cos(posOffset) + luciaSpriteX;
    spr.y = ShieldBall_Sin(posOffset) + luciaSpriteY - 8;
    spr.tile = 0x62;
    spr.palette = ((gameFrames >> 1) + currObjectIndex) & 3;
    Sprite_Draw(&spr, NULL);
    Weapon_SetCollisionCoords(spr.x, spr.y);
}