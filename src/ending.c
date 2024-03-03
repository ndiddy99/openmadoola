/* ending.c: Ending screen
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

#include "bg.h"
#include "constants.h"
#include "ending.h"
#include "game.h"
#include "palette.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "textscroll.h"

static Sprite *luciaSprites[3];
static Sprite *princeSprites[3];
static int spriteCursor;

static Uint8 endingBGPalette[] = {
    0x0F, 0x39, 0x29, 0x19,
};

static Uint8 endingSpritePalette[] = {
    0x0F, 0x12, 0x16, 0x36,
    0x0F, 0x1A, 0x27, 0x36,
    0x0F, 0x25, 0x16, 0x36,
    0x0F, 0x2C, 0x27, 0x36,
};

static void Ending_WaitFrames(int count);
static void Ending_LuciaRun(void);
void Ending_Sprite(Sprite **spr, int frames);
static void Ending_LoadSprite(Sprite *spr);

static void Ending_Animation(void) {
    Sprite_ClearList();
    spriteCursor = 0;
    for (int i = 0; i < 3; i++) {
        luciaSprites[i] = Sprite_Get();
        princeSprites[i] = Sprite_Get();
    }
    Ending_WaitFrames(1);

    // prince lying on ground
    Ending_Sprite(princeSprites, 30);
    // lucia running over to the prince
    Ending_LuciaRun();
    Ending_WaitFrames(10);
    // lucia ducking down
    Ending_Sprite(luciaSprites, 30);
    // prince getting up
    Ending_Sprite(princeSprites, 50);
    // prince standing up
    Ending_Sprite(princeSprites, 20);
    // lucia standing up
    Ending_Sprite(luciaSprites, 40);
    // palette shift
    flashTimer = 40;
    Ending_WaitFrames(20);
    // lucia & prince transform into cool clothes
    Ending_Sprite(luciaSprites, 0);
    Ending_Sprite(princeSprites, 80);
    // lucia & prince face forwards
    Ending_Sprite(luciaSprites, 0);
    Ending_Sprite(princeSprites, 120);
}

static void Ending_WaitFrames(int count) {
    for (int i = 0; i < count; i++) {
        System_StartFrame();
        BG_Draw();
        Sprite_EndFrame();
        System_EndFrame();
        frameCount++;
    }
}

static Uint16 luciaRunTiles[] = {
    0x04, 0x08, 0x0c, 0x08,
};

static void Ending_LuciaRun(void) {
    Uint16 xPos = 8;
    Sprite *top = luciaSprites[0];
    Sprite *bottom = luciaSprites[1];

    while (xPos < 120) {
        Ending_WaitFrames(1);
        // top sprite
        top->size = SPRITE_16X16;
        top->x = xPos;
        top->y = 152;
        top->tile = luciaRunTiles[(xPos >> 3) & 3];
        top->palette = 0;
        top->mirror = H_MIRROR;

        // bottom sprite
        *bottom = *top;
        bottom->y += 16;
        bottom->tile += 2;

        xPos++;
    }
}

void Ending_Sprite(Sprite **spr, int frames) {
    // workaround for the graphics for one of the prince's head frames being
    // stored on top of each other rather than next to each other like every
    // other sprite
    if (spriteCursor == 60) {
        Ending_LoadSprite(spr[0]);
        spr[0]->size = SPRITE_8X16;
        *spr[1] = *spr[0];
        spr[1]->x -= 8;
        spr[1]->tile += 2;
        Ending_LoadSprite(spr[2]);
    }
    else {
        Ending_LoadSprite(spr[0]);
        Ending_LoadSprite(spr[1]);
        spr[2]->size = SPRITE_NONE;
    }
    if (frames) {
        Ending_WaitFrames(frames);
    }
}

static Uint16 endingSpriteTbl[] = {
//  x     y     tile   pal   mirror
    0x98, 0xA8, 0x6CA, 0x01, H_MIRROR, // prince lying on ground
    0x88, 0xA8, 0x6EA, 0x01, H_MIRROR,
    0x78, 0xA0, 0x300, 0x00, H_MIRROR, // lucia ducking down
    0x78, 0xB0, 0x32C, 0x00, H_MIRROR,
    0x88, 0xA0, 0x6AC, 0x01, H_MIRROR, // prince getting up
    0x88, 0xB0, 0x68E, 0x01, H_MIRROR,
    0x88, 0x98, 0x6AC, 0x01, H_MIRROR, // prince standing up
    0x88, 0xA8, 0x6AE, 0x01, H_MIRROR,
    0x78, 0x98, 0x3E4, 0x00, H_MIRROR, // lucia standing up
    0x78, 0xA8, 0x3E6, 0x00, H_MIRROR,
    0x78, 0x98, 0x3C4, 0x02, H_MIRROR, // lucia fancy clothes
    0x78, 0xA8, 0x3E8, 0x02, H_MIRROR,
    0x8C, 0x98, 0x38C, 0x03, H_MIRROR, // prince fancy clothes
    0x88, 0xA8, 0x3EA, 0x03, H_MIRROR,
    0x78, 0x98, 0x708, 0x02, 0,        // lucia facing forward
    0x78, 0xA8, 0x70A, 0x02, 0,
    0x88, 0x98, 0x70C, 0x03, 0,        // prince facing forward
    0x88, 0xA8, 0x70E, 0x03, 0,
};

static void Ending_LoadSprite(Sprite *spr) {
    spr->size = SPRITE_16X16;
    spr->x = endingSpriteTbl[spriteCursor++];
    spr->y = endingSpriteTbl[spriteCursor++];
    spr->tile = endingSpriteTbl[spriteCursor++];
    spr->palette = (Uint8)endingSpriteTbl[spriteCursor++];
    spr->mirror = (Uint8)endingSpriteTbl[spriteCursor++];
}

static char endingText[] = {
    "   THE EVIL IS DEFEATED NOW.\n\n"
    "    THE WING OF MADOOLA WILL\n\n"
    "     BE BRIGHTING OVER THE\n\n"
    "        WORLD FOR PEACE.\n\n\n\n"
    "   YOU FINISHED THE ADVENTURE.\n\n"
    "    THANK YOU FOR PLAYING AND\n\n"
    "      HELPING LUCIA TO SAVE\n\n"
    "           HER PRINCE.\n\n"
    "\n\n\n\n\n\n\n\n\n\n"
};

static char endingTextArcade[] = {
    "  All the evils have now been\n\n"
    "     defeated, and you have\n\n"
    "  accomplished your adventure.\n\n"
    "  The Wings of Madoola will be\n\n"
    "   here to stay and brighten\n\n"
    "       the world with an\n\n"
    "       everlasting peace.\n\n\n\n"
    "   Thank you for playing the\n\n"
    "  game and helping Lucia save\n\n"
    "           the prince.\n\n"
    "\n\n\n\n\n\n\n\n\n\n"
};

// "The End" graphic
static Uint16 endingGfx[] = {
    0x720, 0x720, 0x720, 0x720, 0x720, 0x720, 0x720, 0x720, 0x720, 0x75B, 0x701, 0x702, 
    0x703, 0x704, 0x705, 0x720, 0x706, 0x707, 0x705, 0x721, 0x722, 0x723, 0x724, 0x00,
    0x720, 0x720, 0x720, 0x720, 0x720, 0x720, 0x720, 0x720, 0x715, 0x710, 0x711, 0x712, 
    0x713, 0x714, 0x720, 0x720, 0x716, 0x714, 0x717, 0x725, 0x726, 0x727, 0x728, 0x00,
    0x00,
};

static void Ending_MoveSpritesUp(void) {
    for (int i = 0; i < 3; i++) {
        luciaSprites[i]->y--;
        princeSprites[i]->y--;
    }
}

void Ending_Run(void) {
    Sound_Reset();
    flashTimer = 0;
    BG_Scroll(0, 0);
    BG_Clear();
    BG_SetPalette(0, endingBGPalette);
    Sprite_SetAllPalettes(endingSpritePalette);
    Ending_Animation();
    Sound_Play(MUS_ENDING);
    TextScroll_Init(0);
    if (gameType == GAME_TYPE_PLUS) {
        TextScroll_DispStr(endingTextArcade, Ending_MoveSpritesUp);
    }
    else {
        TextScroll_DispStr(endingText, Ending_MoveSpritesUp);
    }
    TextScroll_DispTiles(endingGfx, Ending_MoveSpritesUp);
    TextScroll_DispStr("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n", Ending_MoveSpritesUp);
    Ending_WaitFrames(360);
}
