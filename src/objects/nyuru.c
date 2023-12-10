/* nyuru.c: Nyuru object code
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
#include "game.h"
#include "lucia.h"
#include "map.h"
#include "nyuru.h"
#include "object.h"
#include "sound.h"
#include "sprite.h"

void Nyuru_InitObj(Object *o) {
    o->hp = 10;
    o->type += 0x20;
    o->timer = 0;
    Sound_Play(SFX_NYURU);
}

static Sint8 Nyuru_SetSpeed(Fixed16 nyuruPos, Fixed16 luciaPos, Uint8 *equalFlag) {
    Sint8 speed;

    if (nyuruPos.f.h == luciaPos.f.h) {
        if (nyuruPos.f.l == luciaPos.f.l) {
            (*equalFlag)--;
            speed = 0;
        }
        // fine movement
        else if (nyuruPos.f.l > luciaPos.f.l) {
            speed =  -((nyuruPos.f.l - luciaPos.f.l) / 2);
        }
        else {
            speed = (luciaPos.f.l - nyuruPos.f.l) / 2;
        }
    }
    // coarse movement
    else if (nyuruPos.f.h > luciaPos.f.h) {
        speed = -((nyuruPos.f.h - luciaPos.f.h) * 8);
    }
    else {
        speed = (luciaPos.f.h - nyuruPos.f.h) * 8;
    }

    return speed;
}

void Nyuru_Obj(Object *o) {
    // gets decremented when the x or y positions are equal to Lucia's
    Uint8 equalFlag = 2;

    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        o->xSpeed = Nyuru_SetSpeed(o->x, luciaXPos, &equalFlag);
        // make nyuru hover over lucia's head when she's crouching
        Fixed16 targetY = luciaYPos;
        targetY.f.h--;
        o->ySpeed = Nyuru_SetSpeed(o->y, targetY, &equalFlag);
        Object_CalcXYPos(o);
        o->timer++;
    }

    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.mirror = 0;
    spr.palette = 1;

    // if x and y pos are equal to lucia's
    if (!equalFlag) {
        if (!o->timer) {
            spr.tile = 0x18e;
        }
        else {
            spr.tile = 0x1ae;
        }
    }
    else {
        if (!(o->timer & 0x10)) {
            spr.tile = 0x18e;
        }
        else if (!(o->timer & 0x20)) {
            spr.tile = 0x1ae;
        }
        else {
            spr.tile = 0x14e;
        }
    }

    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }

    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 20);
}