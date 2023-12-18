/* game.c: Game related management code and global variables
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

#include <stdnoreturn.h>
#include <string.h>

#include "camera.h"
#include "db.h"
#include "ending.h"
#include "enemy.h"
#include "game.h"
#include "hud.h"
#include "item.h"
#include "joy.h"
#include "lucia.h"
#include "map.h"
#include "mainmenu.h"
#include "object.h"
#include "options.h"
#include "palette.h"
#include "rng.h"
#include "rom.h"
#include "save.h"
#include "screen.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "weapon.h"

#define SOFT_RESET (JOY_A | JOY_B | JOY_START | JOY_SELECT)

Uint8 gameType = GAME_TYPE_PLUS;
Uint8 paused;
Uint8 stage;
Uint8 highestReachedStage;
Uint8 orbCollected;
Uint8 roomChangeTimer;
Uint8 bossActive;
Uint8 numBossObjs;
Uint8 bossDefeated[16];
Uint8 frameCount;
Sint8 keywordDisplay;

Uint8 spritePalettes[16] = {
    0x00, 0x12, 0x16, 0x36,
    0x00, 0x1A, 0x14, 0x30,
    0x00, 0x01, 0x11, 0x26,
    0x00, 0x00, 0x27, 0x37,
};

// start x pos (high), start y pos (high), start room number
static Uint8 stageInitTable[] = {
    0x06, 0x0E, 0x00,
    0x06, 0x0E, 0x01,
    0x05, 0x0A, 0x03,
    0x05, 0x0A, 0x05,
    0x37, 0x3B, 0x0D,
    0x06, 0x6E, 0x02,
    0x05, 0x4A, 0x05,
    0x27, 0x7B, 0x0D,
    0x06, 0x2E, 0x00,
    0x17, 0x0B, 0x0A,
    0x17, 0x7B, 0x0C,
    0x05, 0x7A, 0x04,
    //0x07, 0x0b, 0x06, // stage 1 boss room
    0x67, 0x7B, 0x0B,
    0x27, 0x0B, 0x09,
    0x37, 0x1B, 0x08,
    0x37, 0x7B, 0x0E,
    //0x32, 0x3f, 0x0E, // last stage, near the wing of madoola powerup
};

static Uint8 itemSpawnYOffsets[] = {
    0x09, 0x0B, 0x09, 0x0B,
};

static Uint8 bossObjCounts[16] = {
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    50,
    5,
    1,
    1,
    30,
    1,
    1,
};

static void Game_HandleWeaponSwitch(void);
static void Game_HandlePause(void);
static void Game_SpawnFountain(SpawnInfo *info);
static void Game_HandleRoomChange(void);
static void Game_SetRoom(Uint16 roomNum);
static void Game_HandlePaletteShifting(void);

void Game_InitVars(Object *o) {
    Map_LoadPalettes();
    // load sprite palettes
    Sprite_SetAllPalettes(spritePalettes);
    // clear all weapon objects
    Weapon_Init();

    // set up the camera
    cameraX.f.h = o->x.f.h & 0x70;
    cameraY.f.h = (o->y.f.h & 0x70) + 1;
    cameraX.f.l = 0;
    cameraY.f.l = 0;

    bossActive = 0;
    luciaHurtPoints = 0;
    roomChangeTimer = 0;
    attackTimer = 0;
    o->direction = 0;
    flashTimer = 0;

    SpawnInfo info;
    Map_GetSpawnInfo(o, &info);

    // if we're in an item room and the item hasn't been collected, spawn it
    if ((info.type == SPAWN_TYPE_ITEM) && (!Item_Collected(o))) {
        objects[9].type = OBJ_ITEM;
        objects[9].hp = info.enemy - ITEM_FLAG;
        objects[9].x.f.h = (o->x.f.h & 0x70) | 7;
        objects[9].y.f.h = (o->y.f.h & 0x70) | itemSpawnYOffsets[o->y.f.h >> 5];
        objects[9].x.f.l = 0x80;
        objects[9].y.f.l = 0x80;
        objects[9].ySpeed = 0;
    }
    
    // if we're in the boss room and the boss hasn't been defeated, set up the
    // number of boss objects
    else if (currRoom == 6) {
        if (!bossDefeated[stage]) {
            bossActive = 1;
            numBossObjs = bossObjCounts[stage];
        }
    }

    else if (info.type == SPAWN_TYPE_FOUNTAIN) {
        Game_SpawnFountain(&info);
    }

    // spawn the wing of madoola if lucia hasn't collected it yet
    if (stage == 15) {
        if (!hasWing) {
            objects[MAX_OBJECTS - 1].type = OBJ_WING_OF_MADOOLA;
        }
        else {
            objects[MAX_OBJECTS - 1].type = OBJ_DARUTOS_INIT;
        }
    }

    o->type = OBJ_LUCIA_NORMAL;
    Camera_SetXY(o);
    Object_InitCollision(o);
}

static Sint8 fountainXTbl[] = {
    0x39,
    0x29,
    0x59,
    0x79,
    0x29,
    0x59,
    0x79,
};

static Sint8 fountainYTbl[] = {
    0x0A,
    0x1A,
    0x1A,
    0x1A,
    0x26,
    0x26,
    0x26,
};

static Uint8 fountainPalette[] = {
    0x26, 0x03, 0x31, 0x21,
};

static void Game_SpawnFountain(SpawnInfo *info) {
    Uint8 offset = (info->enemy & 0x7) - 1;
    objects[9].x.f.h = fountainXTbl[offset];
    objects[9].y.f.h = fountainYTbl[offset];
    objects[9].type = OBJ_FOUNTAIN;
    Sprite_SetPalette(2, fountainPalette);
}

// TODO refactor this function into several so i don't have to use gotos
noreturn void Game_Run(void) {
    // the last room number Lucia was in this stage
    Uint16 lastRoom;

    // initialize game type
    DBEntry *gameTypeEntry = DB_Find("gametype");
    if (gameTypeEntry) {
        gameType = gameTypeEntry->data[0];
    }

startGameCode:
    Screen_Title();
    MainMenu_Run();

showSaveScreen:
    if (Save_Screen()) {
        // stage number got set by the save screen so we don't need to set it here
        health = 1000;
    }
    else {
        health = 1000;
        maxHealth = 1000;
        maxMagic = 1000;
        highestReachedStage = 0;

        // initialize boots and weapon levels
        bootsLevel = 0;
        for (int i = NUM_WEAPONS - 1; i >= 0; i--) {
            weaponLevels[i] = 0;
        }
        weaponLevels[WEAPON_SWORD] = 1;
        Item_InitCollected();
        for (int i = 0; i < 16; i++) {
            bossDefeated[i] = 0;
        }
        stage = 0;
        orbCollected = 0;
        keywordDisplay = 0;
    }

    paused = 0;
    currentWeapon = WEAPON_SWORD;

initStage:
    Save_SaveFile();

    magic = maxMagic;
    lastRoom = 0xffff;
    Screen_Status();
    Screen_Stage();

    // set up lucia's position and the room number
    Object *lucia = &objects[0];
    memset(lucia, 0, sizeof(Object));
    lucia->x.f.l = 0x80;
    lucia->y.f.l = 0x80;
    hasWing = 0;
    lucia->x.f.h = stageInitTable[stage * 3];
    lucia->y.f.h = stageInitTable[stage * 3 + 1]; 
    Game_SetRoom(stageInitTable[stage * 3 + 2]);

initRoom:
    Game_InitVars(lucia);
    if ((gameType != GAME_TYPE_PLUS) || (currRoom != lastRoom)) {
        Game_PlayRoomSong();
    }
    lastRoom = currRoom;

mainGameLoop:
    System_StartFrame();
    Game_HandlePause();
    if (paused == 0) {
        Sprite_ClearList();
        if (gameType == GAME_TYPE_PLUS) {
            Game_HandleWeaponSwitch();
        }
        HUD_Display();
        RNG_Get(); // update RNG once per frame
        Weapon_Process();
        Object_ListRun();
        Enemy_Spawn();
        Game_HandleRoomChange();
    }
    else {
        HUD_Weapon();
    }
    Map_Draw();
    Sprite_EndFrame();
    System_EndFrame();
    frameCount++;
    Game_HandlePaletteShifting();

    // --- soft reset code (NOTE: not in original game) ---
    if ((joy & SOFT_RESET) == SOFT_RESET) {
        goto startGameCode;
    }

    // --- handle keyword screen ---
    if (keywordDisplay > 0) {
        Screen_Keyword();
        // mark keyword as shown
        keywordDisplay = -1;
        // despawn yokko-chan
        Object_DeleteRange(9);
        // force music to play
        lastRoom = 0xffff;
        goto initRoom;
    }

    // --- handle doors ---
    if (roomChangeTimer != 1) { goto mainGameLoop; }
    if (objects[0].type == OBJ_LUCIA_WARP_DOOR) {
        int switchRoom = Map_Door(&objects[0]);
        if (switchRoom == DOOR_ENDING) {
            // orbCollected gets set when Lucia kills Darutos
            if (orbCollected) {
                Ending_Run();
                goto startGameCode;
            }
            // put Lucia back where she was if she tried to enter the ending
            // door without killing Darutos
            else {
                Game_SetRoom(currRoom);
            }
        }
        else if (switchRoom == DOOR_INVALID) {
            Game_SetRoom(currRoom);
        }
        else {
            Game_SetRoom(switchRoom);
        }
        goto initRoom;
    }
    else if (objects[0].type == OBJ_LUCIA_LVL_END_DOOR) {
        stage++;
        stage &= 0xf;
        if (stage > highestReachedStage) {
            highestReachedStage = stage;
            orbCollected = 0;
        }
        goto initStage;
    }
    else {
        Save_SaveFile();
        Screen_GameOver();
        goto showSaveScreen;
    }

}

static Uint8 roomSongs[] = {
    MUS_OVERWORLD,
    MUS_OVERWORLD,
    MUS_OVERWORLD,
    MUS_CAVE,
    MUS_CAVE,
    MUS_CAVE,
    MUS_BOSS,
    MUS_ITEM,
    MUS_CASTLE,
    MUS_CASTLE,
    MUS_CASTLE,
    MUS_CASTLE,
    MUS_CASTLE,
    MUS_CASTLE,
    MUS_CASTLE,
    MUS_ITEM,
};

void Game_PlayRoomSong(void) {
    Sound_Reset();
    Uint8 index = currRoom & 0xf;
    // are we in stage 16's room?
    if (index == 14) {
        // don't play any music if darutos has been killed
        if (orbCollected) {
            return;
        }
        // if lucia collected the wing of madoola, play the castle theme
        if (hasWing) {
            Sound_Play(roomSongs[index]);
        }
        // otherwise play the boss room theme
        else {
            Sound_Play(MUS_BOSS);
        }
    }
    // if we're in the boss room, play the boss music if the boss hasn't been
    // killed, and the item room music if it has been
    else if (roomSongs[index] == MUS_BOSS) {
        if (bossActive) {
            Sound_Play(MUS_BOSS);
        }
        else {
            Sound_Play(MUS_ITEM);
        }
    }
    else {
        Sound_Play(roomSongs[index]);
    }
}

static void Game_HandleWeaponSwitch(void) {
    if (joyEdge & JOY_SELECT) {
        Sound_Play(SFX_SELECT);
        Weapon_Init();
        // go to the next weapon that we have and have enough magic to use
        do {
            currentWeapon++;
            if (currentWeapon >= NUM_WEAPONS) { currentWeapon = 0; }
        } while ((weaponLevels[currentWeapon] == 0) || (Weapon_MagicAfterUse() < 0));
        // clamp weapon level to 3
        if (weaponLevels[currentWeapon] > 3) {
            weaponLevels[currentWeapon] = 3;
        }
    }
}

static void Game_HandlePause(void) {
    if (paused) {
        if (joyEdge & JOY_START) {
            if (gameType == GAME_TYPE_PLUS) {
                Sound_LoadState();
            }
            else {
                Game_PlayRoomSong();
            }
            Sound_Play(SFX_PAUSE);
            paused = 0;
            return;
        }
        Game_HandleWeaponSwitch();
    }
    else {
        if (joyEdge & JOY_START) {
            Sound_SaveState();
            Sound_Reset();
            Sound_Play(SFX_PAUSE);
            paused = 1;
            if (gameType == GAME_TYPE_ORIGINAL) {
                HUD_WeaponInit((SCREEN_WIDTH / 2) - (16 / 2), 32);
            }
        }
    }
}

static void Game_SetRoom(Uint16 roomNum) {
    if (roomNum == 15) { scrollMode = SCROLL_MODE_LOCKED; }
    else if ((roomNum == 6) || (roomNum == 7)) { scrollMode = SCROLL_MODE_X; }
    else { scrollMode = SCROLL_MODE_FREE; }
    Map_Init(roomNum);
}

static void Game_HandlePaletteShifting(void) {
    // only palette shift on rooms with waterfalls
    if ((currRoom != 1) && (currRoom != 2)) {
        return;
    }

    if ((frameCount & 3) == 0) {
        Uint8 temp = colorPalette[15];
        colorPalette[15] = colorPalette[14];
        colorPalette[14] = colorPalette[13];
        colorPalette[13] = temp;
    }
}

static Uint16 *Game_GetDoorMetatiles(void) {
    static Uint16 metatiles[6];

    // get Lucia's collision offset
    Uint16 offset = objects[0].collision;
    Uint16 yOffset = offset / MAP_WIDTH_METATILES;
    Uint16 xOffset = offset % MAP_WIDTH_METATILES;

    // chunk align offset numbers
    xOffset &= 0xFC;
    yOffset &= 0xFC;

    // the exit door chunks have the door offset by 1 metatile from the top left of the chunk
    xOffset += 1;
    yOffset += 1;

    // get door's metatile offset
    offset = yOffset * MAP_WIDTH_METATILES + xOffset;

    // 3 left door metatiles
    metatiles[0] = mapMetatiles[offset];
    metatiles[1] = mapMetatiles[offset + MAP_WIDTH_METATILES];
    metatiles[2] = mapMetatiles[offset + MAP_WIDTH_METATILES * 2];

    // 3 right door metatiles
    metatiles[3] = mapMetatiles[offset + 1];
    metatiles[4] = mapMetatiles[offset + MAP_WIDTH_METATILES + 1];
    metatiles[5] = mapMetatiles[offset + MAP_WIDTH_METATILES * 2 + 1];

    return metatiles;
}

static void Game_SetMetatileTiles(Uint16 num, Uint16 tl, Uint16 tr, Uint16 bl, Uint16 br) {
    Uint16 tileset = mapData.rooms[currRoom].tileset;
    Uint16 base = tilesetBases[tileset];

    mapData.tilesets[tileset].metatiles[num].tiles[0] = tl + base;
    mapData.tilesets[tileset].metatiles[num].tiles[1] = tr + base;
    mapData.tilesets[tileset].metatiles[num].tiles[2] = bl + base;
    mapData.tilesets[tileset].metatiles[num].tiles[3] = br + base;
}

static void Game_LeftDoorMidOpen(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[0], 0xd6, 0xff, 0xe6, 0xff);
    Game_SetMetatileTiles(metatiles[1], 0xe8, 0xff, 0xe6, 0xff);
    Game_SetMetatileTiles(metatiles[2], 0xe8, 0xff, 0xf6, 0xff);
}

static void Game_LeftDoorFullOpen(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[0], 0xff, 0xff, 0xff, 0xff);
    Game_SetMetatileTiles(metatiles[1], 0xff, 0xff, 0xff, 0xff);
    Game_SetMetatileTiles(metatiles[2], 0xff, 0xff, 0xff, 0xff);
}

static void Game_LeftDoorClose(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[0], 0xd5, 0xd6, 0xe5, 0xe6);
    Game_SetMetatileTiles(metatiles[1], 0xe7, 0xe8, 0xe5, 0xe6);
    Game_SetMetatileTiles(metatiles[2], 0xe7, 0xe8, 0xf5, 0xf6);
}

static void Game_RightDoorMidOpen(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[3], 0xff, 0xd7, 0xff, 0xe7);
    Game_SetMetatileTiles(metatiles[4], 0xff, 0xe5, 0xff, 0xe7);
    Game_SetMetatileTiles(metatiles[5], 0xff, 0xe5, 0xff, 0xf5);
}

static void Game_RightDoorFullOpen(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[3], 0xff, 0xff, 0xff, 0xff);
    Game_SetMetatileTiles(metatiles[4], 0xff, 0xff, 0xff, 0xff);
    Game_SetMetatileTiles(metatiles[5], 0xff, 0xff, 0xff, 0xff);
}

static void Game_RightDoorClose(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[3], 0xd7, 0xd8, 0xe7, 0xe8);
    Game_SetMetatileTiles(metatiles[4], 0xe5, 0xe6, 0xe7, 0xe8);
    Game_SetMetatileTiles(metatiles[5], 0xe5, 0xe6, 0xf5, 0xf6);
}

static void Game_HandleRoomChange(void) {
    if (roomChangeTimer) {
        // if Lucia's at the end of level door, animate the door opening
        if (objects[0].type == OBJ_LUCIA_LVL_END_DOOR) {
            switch (roomChangeTimer) {
            case 255:
                return;

            case 60:
                Game_LeftDoorMidOpen();
                break;

            case 59:
                Game_RightDoorMidOpen();
                break;

            case 45:
                Game_LeftDoorFullOpen();
                break;

            case 44:
                Game_RightDoorFullOpen();
                break;

            case 30:
                Game_LeftDoorMidOpen();
                break;

            case 29:
                Game_RightDoorMidOpen();
                break;

            case 15:
                Game_LeftDoorClose();
                break;

            case 14:
                Game_RightDoorClose();
                break;
            }
        }

        roomChangeTimer--;
    }
}
