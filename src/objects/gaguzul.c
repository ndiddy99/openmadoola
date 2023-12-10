/* gaguzul.c: Gaguzul object code
*
*  This file is part of OpenMadoola.
*
*  OpenMadoola is free software: you can redistribute it and/or modify it
*  under the terms of the GNU General Public License as published by the Free
*  Software Foundation; either version 2 of the License, or (at your option)
*  any later version.
*
*  OpenMadoola is distributed in the hope that it will be useful, but WITHOUT
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
*  more details.
*
*  You should have received a copy of the GNU General Public License
*  along with OpenMadoola. If not, see <https://www.gnu.org/licenses/>.
*/

#include "collision.h"
#include "fireball.h"
#include "gaguzul.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

void Gaguzul_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 192;
    o->type += 0x20;
    o->timer = 0;
    if (!o->direction) {
        o->xSpeed = 0x18;
    }
    else {
        o->xSpeed = 0xe8;
    }
    o->ySpeed = 0;
}

static Sint8 xOffsets[] = {
    0x10, 0xf0, 0xf0, 0x10,
};

void Gaguzul_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }

    Fireball_Spawn(0x1f, o);
    o->timer++;
    // reorient towards lucia every 32 frames
    if (!(o->timer & 0x1f)) {
        Object_MoveTowardsLucia(o);
    }
    
    if (!Object_HandleJump(o)) {
        if (Object_TouchingGround(o)) {
            Object_JumpIfHitWall(o);
        }
        else {
            Object_ApplyGravity(o);
            Object_CheckForWall(o);
            if (Object_UpdateYPos(o)) {
                o->ySpeed = 0;
                o->y.f.l &= 0x80;
                Object_MoveTowardsLucia(o);
            }
        }
    }

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 3;
    // lower left tile
    if (!(o->x.f.h & 1)) {
        spr.tile = 0xc2;
    }
    else {
        spr.tile = 0xc6;
    }
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }
    // upper left tile
    spr.y -= 0x10;
    spr.tile -= 2;
    Sprite_Draw(&spr, o);
    Uint8 offsetCursor = 0;
    // upper right tile
    if (!o->direction) {
        offsetCursor = 2;
    }
    spr.x += xOffsets[offsetCursor];
    offsetCursor++;
    spr.tile += 0x20;
    Sprite_Draw(&spr, o);
    // lower right tile
    spr.y += 0x10;
    spr.tile += 2;
    Sprite_Draw(&spr, o);
    spr.y -= 0x10;
    spr.x += xOffsets[offsetCursor];
    Collision_Handle(o, &spr, COLLISION_SIZE_16X32, 60);
}