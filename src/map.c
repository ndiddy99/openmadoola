  /* map.c: handles accessing map data
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

#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "graphics.h"
#include "map.h"
#include "object.h"
#include "palette.h"

MapData mapData;
Uint16 mapMetatiles[MAP_HEIGHT_METATILES * MAP_WIDTH_METATILES];
Uint8 currRoom = 0xff;
static Uint16 scrollX;
static Uint16 scrollY;

void Map_Init(Uint8 roomNum) {
    if (roomNum == currRoom) { return; }

    currRoom = roomNum;

    // decompress the room's metatiles
    for (int screenY = 0; screenY < 8; screenY++) {
        for (int screenX = 0; screenX < 8; screenX++) {
            int screenNum = mapData.rooms[roomNum].screenNums[(screenY) * 8 + screenX];
            for (int chunkY = 0; chunkY < 4; chunkY++) {
                for (int chunkX = 0; chunkX < 4; chunkX++) {
                    int chunkNum = mapData.screens[screenNum][chunkY * 4 + chunkX];
                    for (int metatileY = 0; metatileY < 4; metatileY++) {
                        for (int metatileX = 0; metatileX < 4; metatileX++) {
                            Uint16 metatileNum = mapData.chunks[chunkNum][metatileY * 4 + metatileX];
                            int xPos = (screenX * 16) + (chunkX * 4) + metatileX;
                            int yPos = (screenY * 16) + (chunkY * 4) + metatileY;
                            mapMetatiles[yPos * 128 + xPos] = metatileNum;
                        }
                    }
                }
            }
        }
    }

    // load the room's palettes
    Map_LoadPalettes();
}

void Map_LoadPalettes(void) {
    memcpy(colorPalette, mapData.rooms[currRoom].palette, sizeof(mapData.rooms[currRoom].palette));
}

Uint16 Map_GetMetatile(Object *o) {
    return mapMetatiles[o->collision];
}

void Map_GetSpawnInfo(Object *o, SpawnInfo *info) {
    // lower 3 bits of offset = x coords
    Uint8 offset = (((Uint8)o->x.f.h) >> 4) & 7;
    // upper 3 bits = y coords
    offset |= (((Uint8)o->y.f.h) >> 1) & 0x38;

    *info = mapData.rooms[currRoom].spawns[offset];
}

Uint16 Map_CheckX(Object *o) {
    Uint16 collision = o->collision;

    // less than halfway through the metatile
    if (o->x.f.l < 0x80) {
        // add "walls" around the map border to prevent from going off the map
        if (o->x.f.h <= 0) {
            goto found_tile;
        }
        // look for solid tiles in the previous metatile
        collision--;
    }
    // halfway or more through the metatile
    else {
        // add "walls" around the map border to prevent from going off the map
        if (o->x.f.h >= 0x7f) {
            goto found_tile;
        }
        // look for solid tiles in the next metatile
        collision++;
    }

    if (mapMetatiles[collision] < MAP_SOLID) {
        // solid tile, so make the object snap to the metatile boundary
        goto found_tile;
    }

    // if we didn't find any solid tiles and are in the upper half of the metatile, check up a metatile
    if (o->y.f.l < 0x80) {
        collision -= MAP_WIDTH_METATILES;
    }
    // if we're in the lower ~1/4 of the metatile, check down a metatile
    else if (o->y.f.l >= 0xa0) {
        collision += MAP_WIDTH_METATILES;
    }
    // otherwise, give up
    else {
        return 0;
    }

    if (mapMetatiles[collision] < MAP_SOLID) {
        goto found_tile;
    }
    return 0;

found_tile:
    // snap object to metatile boundary
    o->x.f.l = 0x80;
    return 1;

}

Uint16 Map_CheckY(Object *o) {
    Uint16 collision = o->collision;

    // less than halfway through the metatile
    if (o->y.f.l < 0x80) {
        // add "walls" around the map borders to prevent from going off the map
        if (o->y.f.h == 0) {
            goto found_tile;
        }

        // look in the previous metatile
        collision -= MAP_WIDTH_METATILES;
    }

    else {
        // prevent from going off the bottom of the map
        if (o->y.f.h == 0x7f) {
            goto found_tile;
        }

        // look in the next metatile
        collision += MAP_WIDTH_METATILES;
    }

    if (mapMetatiles[collision] < MAP_SOLID) {
        goto found_tile;
    }

    // if we're in the upper ~1/4 of the tile, check forward a tile
    if (o->x.f.l >= 0xa8) {
        collision++;
    }

    // if we're in the lower ~1/4 of the tile, check backward a tile
    else if (o->x.f.l < 0x58) {
        collision--;
    }

    // otherwise, give up
    else {
        return 0;
    }

    if (mapMetatiles[collision] < MAP_SOLID) {
        goto found_tile;
    }
    return 0;

found_tile:
    // snap object to metatile boundary
    o->y.f.l = 0x80;
    return 1;
}

Uint16 Map_SolidTileBelow(Uint16 offset) {
    Uint16 metatile = mapMetatiles[offset];
    // are we on top of a solid tile or a ladder?
    if (metatile < 0x24) {
        // look down a metatile
        offset += MAP_WIDTH_METATILES;
        metatile = mapMetatiles[offset];
        // are we on top of a solid tile (not a ladder)?
        if (metatile < 0x1f) {
            return 1;
        }
    }
    else {
        offset += MAP_WIDTH_METATILES;
        metatile = mapMetatiles[offset];
        // are we on top of a solid block or ladder? ladders are solid from the
        // top
        if (metatile < 0x24) {
            return 1;
        }

    }

    return 0;
}

int Map_Door(Object *o) {
    Uint8 chunkAlignedX = o->x.f.h & 0xfc;
    Uint8 chunkAlignedY = o->y.f.h & 0xfc;

    for (int i = 0; i < mapData.numWarpDoors; i++) {
        Uint8 chunkAlignedDoorX = mapData.warpDoors[i].xPos & 0xfc;
        Uint8 chunkAlignedDoorY = mapData.warpDoors[i].yPos & 0xfc;
        if ((mapData.warpDoors[i].roomNum == currRoom) &&
            (chunkAlignedDoorX == chunkAlignedX) &&
            (chunkAlignedDoorY == chunkAlignedY)) {
                // ending door
                if (i == 0) {
                    return DOOR_ENDING;
                }
                int doorIndex = i ^ 1;
                o->x.f.l = 0x80;
                o->y.f.l = 0x80;
                o->x.f.h = mapData.warpDoors[doorIndex].xPos;
                o->y.f.h = mapData.warpDoors[doorIndex].yPos;
                return mapData.warpDoors[doorIndex].roomNum;
        }
    }

    return DOOR_INVALID;
}

void Map_SetPos(Uint16 x, Uint16 y) {
    scrollX = x;
    scrollY = y;
}

void Map_Draw(void) {
    Uint16 tileset = mapData.rooms[currRoom].tileset;

    for (int y = 0; y < SCREEN_HEIGHT + METATILE_SIZE; y += METATILE_SIZE) {
        for (int x = 0; x < SCREEN_WIDTH + METATILE_SIZE; x += METATILE_SIZE) {
            int xPos = x - (scrollX % METATILE_SIZE);
            int yPos = y - (scrollY % METATILE_SIZE);

            int xTile = (((x + scrollX) / METATILE_SIZE) % MAP_WIDTH_METATILES);
            int yTile = (((y + scrollY) / METATILE_SIZE) % MAP_WIDTH_METATILES);

            Metatile *metatile = &mapData.tilesets[tileset].metatiles[mapMetatiles[yTile * MAP_WIDTH_METATILES + xTile]];
            Graphics_DrawTile(xPos + 0, yPos + 0, metatile->tiles[0], metatile->palnum, 0);
            Graphics_DrawTile(xPos + 8, yPos + 0, metatile->tiles[1], metatile->palnum, 0);
            Graphics_DrawTile(xPos + 0, yPos + 8, metatile->tiles[2], metatile->palnum, 0);
            Graphics_DrawTile(xPos + 8, yPos + 8, metatile->tiles[3], metatile->palnum, 0);
        }
    }
}
