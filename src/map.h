/* map.h: handles accessing map data
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

#pragma once
#include "graphics.h"
#include "object.h"

typedef struct {
    Uint16 palnum;
    Uint16 tiles[4];
} Metatile;

typedef struct {
    Uint16 len;
    Metatile *metatiles; // metatiles are 2x2 arrays of tiles
} Tileset;

typedef enum {
    SPAWN_TYPE_ENEMY = 0,
    SPAWN_TYPE_ITEM,
    SPAWN_TYPE_FOUNTAIN,
    SPAWN_TYPE_BOSS,
} SPAWN_TYPE;

typedef struct {
    Uint8 type;
    Uint8 enemy;
    Uint8 count;
} SpawnInfo;

typedef struct {
    // which tileset to use
    Uint16 tileset;
    // which song to play
    Uint8 song;
    // palette data to use
    Uint8 palette[16];
    // rooms are 8x8 arrays of screens (2048x2048px)
    Uint16 screenNums[64];
    // which enemy (object number) should go to each screen
    SpawnInfo spawns[64];
} Room;

typedef struct {
    // chunk aligned x position (divided by 16)
    Uint8 xPos;
    // chunk aligned y position (divided by 16)
    Uint8 yPos;
    // room number
    Uint8 roomNum;
} WarpDoor;

typedef struct {
    // Lucia spawn x
    Fixed16 xPos;
    // Lucia spawn y
    Fixed16 yPos;
    // room number
    Uint8 roomNum;
    // boss object number
    Uint8 bossObj;
    // max number of boss objects to spawn at once
    Uint8 bossSpawnCount;
    // number of boss objects Lucia has to kill before the boss is defeated
    Uint8 bossObjCount;
} StageInfo;

typedef struct {
    Uint16 numTilesets;
    Tileset *tilesets;
    Uint16 numChunks;
    Uint16 (*chunks)[16]; // chunks are 4x4 arrays of metatiles (64x64px)
    Uint16 numScreens;
    Uint16 (*screens)[16]; // screens are 4x4 arrays of chunks (256x256px)
    Room rooms[16];
    StageInfo stages[16];
    Uint16 numWarpDoors;
    WarpDoor *warpDoors;
} MapData;

// map data
extern MapData mapData;

// current room number
extern Uint8 currRoom;

#define METATILE_SIZE (16)
#define MAP_WIDTH_METATILES (128)
#define MAP_WIDTH_PIXELS (MAP_WIDTH_METATILES * METATILE_SIZE)
#define MAP_HEIGHT_METATILES (128)
#define MAP_HEIGHT_PIXELS (MAP_HEIGHT_METATILES * METATILE_SIZE)

// anything below this is solid ground
#define MAP_SOLID (0x1f)
// anything below this is either solid ground or a ladder
#define MAP_LADDER (0x24)
extern Uint16 mapMetatiles[MAP_HEIGHT_METATILES * MAP_WIDTH_METATILES];

/**
 * @brief Loads a room from the map data
 * @param room_num the room number to load
*/
void Map_Init(Uint8 room_num);

/**
 * @brief Loads the current room's palette into palette memory
*/
void Map_LoadPalettes(void);

Uint16 Map_GetMetatile(Object *o);

/**
 * @brief Gets the spawn information for the screen an object is located in
 * @param o The object to get the screen position from
 * @param info The SpawnInfo struct to write the spawn information to
*/
void Map_GetSpawnInfo(Object *o, SpawnInfo *info);

/**
 * @brief Checks if there's a solid tile next to the specified object and
 * handles x-axis map collision
 * @param o the object to check
 * @returns 1 if there is a solid tile, 0 if there's not
*/
Uint16 Map_CheckX(Object *o);

/**
 * @brief Checks if there's a solid tile below the specifed object and
 * handles y-axis map collision
 * @param o the object to check
 * @returns 1 if there is a solid tile, 0 if there's not
 */
Uint16 Map_CheckY(Object *o);

/**
 * @brief Checks if the given map offset is on top of either a solid tile or
 * a ladder
 * @param offset Map offset to look at
 * @returns 1 if there's a solid tile or ladder, 0 if there's not
 */
Uint16 Map_SolidTileBelow(Uint16 offset);

#define DOOR_INVALID -1
#define DOOR_ENDING -2
/**
 * @brief Tries to find the position coordinates associated with the other
 * side of a given door
 * @param o Lucia's object
 * @returns the room number at the other side of the door, DOOR_INVALID if the
 * door doesn't point to another door, or DOOR_ENDING if the door is the ending
 * door from Stage 16.
 */
int Map_Door(Object *o);

/**
 * @brief Sets the map's scroll position
 * @param x The x scroll position
 * @param y The y scroll position
*/
void Map_SetPos(Uint16 x, Uint16 y);

/**
 * @brief Draws the map to the screen
*/
void Map_Draw(void);
