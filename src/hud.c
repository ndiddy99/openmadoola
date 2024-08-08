/* hud.c: HUD display code
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
#include "hud.h"
#include "lucia.h"
#include "sprite.h"
#include "weapon.h"

// the tile number of the "0" tile
#define BASE_NUM 0x6C
#define BORDER_TILE 0x5C
#define DIGIT_WIDTH 8
#define HUD_WIDTH 48
#define WEAPON_WIDTH 16

/**
* @brief Displays the given 4-digit number.
* @param num The number to display
* @param xPos the x position to display the number at
* @param yPos the y position to display the number at
* @param palnum the sprite palette number to use
* @param borders if borders should be shown
*/
static void HUD_ShowNum(Uint16 num, Uint16 xPos, Uint16 yPos, Uint8 palnum, Uint8 borders) {
    Sprite spr = { 0 };
    spr.mirror = 0;
    spr.size = SPRITE_8X16;
    spr.x = xPos + (DIGIT_WIDTH * 4);
    spr.y = yPos;
    spr.palette = palnum;

    if (num > 9999) {
        num = 9999;
    }

    // draw number sprites
    for (int i = 0; i < 4; i++) {
        spr.tile = ((num % 10) * 2) + BASE_NUM;
        Sprite_Draw(&spr, NULL);
        spr.x -= DIGIT_WIDTH;
        num /= 10;
    }

    if (borders) {
        // draw left border
        spr.x = xPos;
        spr.tile = BORDER_TILE;
        Sprite_Draw(&spr, NULL);

        // draw right border
        spr.x = xPos + (5 * DIGIT_WIDTH);
        spr.mirror = H_MIRROR;
        Sprite_Draw(&spr, NULL);
    }
}

static void HUD_ShowScore(Uint32 num, Uint16 xPos, Uint16 yPos, Uint8 palnum) {
    if (num > 99999999) { num = 99999999; }
    HUD_ShowNum((Uint16)(num / 10000), xPos, yPos, palnum, 0);
    HUD_ShowNum((Uint16)(num % 10000), xPos + 32, yPos, palnum, 0);
}

void HUD_Display(void) {
    switch (gameType) {
    case GAME_TYPE_ORIGINAL:
        HUD_ShowNum(health, (SCREEN_WIDTH / 2) - (HUD_WIDTH / 2), 16, 3, 1);
        HUD_ShowNum(magic, (SCREEN_WIDTH / 2) - (HUD_WIDTH / 2), SCREEN_HEIGHT - 32, 0, 1);
        break;

    case GAME_TYPE_PLUS:
        HUD_ShowNum(health, 14, 16, 3, 1);
        HUD_ShowNum(magic, 14, 36, 0, 1);
        HUD_WeaponInit(18, 55);
        break;

    case GAME_TYPE_ARCADE:
        HUD_ShowScore(score, 16, 24, 1);
        HUD_ShowNum(health, SCREEN_WIDTH - 60, SCREEN_HEIGHT - 65, 3, 0);
        HUD_ShowNum(magic, SCREEN_WIDTH - 60, SCREEN_HEIGHT - 41, 0, 0);
        HUD_WeaponInit(SCREEN_WIDTH - 68, SCREEN_HEIGHT - 41);
        break;
    }
}

static Uint8 weaponTiles[] = {
    0x60, 0x60, 0x66, 0x62, 0x64, 0x68, 0x6a,
};

static Uint8 weaponPalettes[] = {
    1, 3, 3, 1, 3, 3, 1,
};

static Sprite *weaponSprite;
static Sprite *weaponBG1;
static Sprite *weaponBG2;

void HUD_WeaponInit(Sint16 x, Sint16 y) {
    weaponBG1 = Sprite_Get();
    weaponBG1->size = SPRITE_8X16;
    weaponBG1->x = x;
    weaponBG1->y = y;
    weaponBG1->tile = 0x4C;
    weaponBG1->mirror = 0;
    weaponBG1->palette = 0;

    weaponBG2 = Sprite_Get();
    weaponBG2->size = SPRITE_8X16;
    weaponBG2->x = weaponBG1->x + 8;
    weaponBG2->y = weaponBG1->y;
    weaponBG2->tile = 0x4C;
    weaponBG2->mirror = H_MIRROR;
    weaponBG2->palette = 0;

    weaponSprite = Sprite_Get();
    weaponSprite->size = SPRITE_8X16;
    weaponSprite->x = weaponBG1->x + 4;
    weaponSprite->y = weaponBG1->y;
    weaponSprite->tile = weaponTiles[currentWeapon];
    weaponSprite->mirror = 0;
    weaponSprite->palette = weaponPalettes[currentWeapon];
}

void HUD_Weapon(void) {
    weaponSprite->tile = weaponTiles[currentWeapon];
    weaponSprite->palette = weaponPalettes[currentWeapon];
}