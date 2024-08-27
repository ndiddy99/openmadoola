/* collision.c: collision handler
 * Copyright (c) 2023, 2024 Nathan Misner
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

#include <stdio.h>

#include "collision.h"
#include "constants.h"
#include "game.h"
#include "lucia.h"
#include "object.h"
#include "palette.h"
#include "sound.h"
#include "sprite.h"
#include "weapon.h"

// collision constants

static Sint8 xHitboxOffsets[] = {
    7, 7, 15,
};

static Sint8 yHitboxOffsets[] = {
    15, 31, 31,
};

static Sint16 xHitboxMasks[] = {
    0xFFF0, 0xFFF0, 0xFFE0,
};

static Sint16 yHitboxMasks[] = {
    0xFFF0, 0xFFE0, 0xFFE0,
};

int Collision_Handle(Object *o, Sprite *s, int size, Uint8 attackPower) {
    Sint16 xCenter = s->x + xHitboxOffsets[size];
    Sint16 yCenter = s->y + yHitboxOffsets[size];

    // flash damages all onscreen objects
    if (flashTimer) {
        goto enemyDamaged;
    }

    // overlay a "hit" graphic on top of the sprite if stunned timer >= 10
    if (o->stunnedTimer >= 10) {
        Sprite hitSpr = { 0 };
        hitSpr.size = SPRITE_16X16;
        hitSpr.x = s->x;
        hitSpr.y = s->y;
        hitSpr.tile = 0x4A;
        hitSpr.palette = 0;
        // use gameFrames to "randomly" set the hit graphic's mirroring
        hitSpr.mirror = (gameFrames >> 4) & 3;
         Sprite_Draw(&hitSpr, o);
    }

    for (int i = (MAX_WEAPONS - 1); i >= 0; i--) {
        if (!weaponCoords[i].spawned) {
            continue;
        }

        // if y coordinate is out of range
        if ((yCenter - weaponCoords[i].y) & yHitboxMasks[size]) {
            continue;
        }

        // if x coordinate is out of range
        if ((xCenter - weaponCoords[i].x) & xHitboxMasks[size]) {
            continue;
        }

        weaponCoords[i].collided = 1;
        goto enemyDamaged;
    }

    // if we're still here, there's no hits so check if the object collided with Lucia
    return Collision_WithLucia(o, s, size, attackPower);

enemyDamaged:
    // NOTE (bug from the original game): When Lucia runs out of MP, weaponDamage
    // gets set to whatever amount of damage the sword currently does. This causes
    // a bug where if the player spams the flash attack and runs out of MP before
    // flashTimer resets, the flash attack will do as much damage as the sword to
    // every object on screen for every frame. I decided not to fix the bug
    // because it's a fun trick to discover and it lets Lucia kill some bosses
    // super fast.
    // Example fix: replace "o->hp -= weaponDamage" with:
    // if (flashTimer) {
    //     // you'll have to add weaponDamageTbl to weapon.h for this to work
    //     o->hp -= weaponDamageTbl[(WEAPON_FLASH * 3) + weaponLevels[WEAPON_FLASH]];
    // }
    // else {
    //     o->hp -= weaponDamage;
    // }
    o->hp -= weaponDamage;
    if (o->hp >= 0) {
        o->stunnedTimer = 16;
        Sound_Play(SFX_ENEMY_HIT);
        return 1;
    }
    else {
        Game_AddScore(attackPower * 10);
        Sound_Play(SFX_ENEMY_KILL);
        if (bossActive) {
            numBossObjs--;
            if (numBossObjs == 0) {
                bossActive = 0;
                bossDefeated[stage] = 0xff;
                // delete all enemy objects
                Object_DeleteRange(9);
                // change music to "boss defeated"
                Game_PlayRoomSong();
                Sound_Play(SFX_BOSS_KILL);
            }
        }
        o->timer = 30;
        o->type = OBJ_EXPLOSION;
        return 0;
    }
}

int Collision_WithLucia(Object *o, Sprite *s, int size, Uint8 attackPower) {
    Sint16 xCenter = s->x + xHitboxOffsets[size];
    Sint16 yCenter = s->y + yHitboxOffsets[size];

    // check that x coordinate is in range
    if (((xCenter - luciaSpriteX) & xHitboxMasks[size]) == 0) {
        Sint16 yDiff = yCenter - luciaSpriteY;
        // check upper body collision
        if (((yDiff - 8) & yHitboxMasks[size]) == 0) {
            luciaHurtPoints = attackPower;
            return 2;
        }
        // check lower body collision
        if (((yDiff + 8) & yHitboxMasks[size]) == 0) {
            luciaHurtPoints = attackPower;
            return 2;
        }
    }

    return 1;
}