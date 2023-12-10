/* rom.c: Reads from the game ROM file
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

#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "map.h"
#include "rom.h"

Uint8 *prgRom = NULL;
Uint8 *chrRom = NULL;
int prgRomSize = 0;
int chrRomSize = 0;

Uint16 tilesetBases[3] = {
    1024, // Forest
    1280, // Cave
    1536, // Castle
};

static char errorBuff[80];

static int Rom_Load(char *filename, int offset, int size, Uint8 **romBuff, int romBuffSize) {
    // open file
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        snprintf(errorBuff, sizeof(errorBuff), "Couldn't find %s", filename);
        ERROR_MSG(errorBuff);
        return 0;
    }

    // reallocate rom buffer
    if (*romBuff) {
        *romBuff = realloc(*romBuff, romBuffSize + size);
        if (!(*romBuff)) {
            ERROR_MSG("Out of memory");
            return 0;
        }
    }
    else {
        *romBuff = malloc(size);
        if (!(*romBuff)) {
            ERROR_MSG("Out of memory");
            return 0;
        }
    }

    // load data into buffer
    fseek(fp, offset, SEEK_SET);
    fread(*romBuff + romBuffSize, 1, size, fp);
    fclose(fp);

    return romBuffSize + size;
}

int Rom_LoadPrg(char *filename, int offset, int size) {
    prgRomSize = Rom_Load(filename, offset, size, &prgRom, prgRomSize);
    return prgRomSize;
}

int Rom_LoadChr(char *filename, int offset, int size) {
    chrRomSize = Rom_Load(filename, offset, size, &chrRom, chrRomSize);
    return chrRomSize;
}

static int init_tileset(MapData *data, int num, int length, Uint16 base, int pal_offset, int rom_offset) {
    data->tilesets[num].len = length;
    data->tilesets[num].metatiles = malloc(length * sizeof(Metatile));
    for (int i = 0; i < length; i++) {
        data->tilesets[num].metatiles[i].palnum = (Uint16)prgRom[pal_offset++];
        data->tilesets[num].metatiles[i].tiles[0] = (Uint16)prgRom[rom_offset++] + base;
        data->tilesets[num].metatiles[i].tiles[1] = (Uint16)prgRom[rom_offset++] + base;
        data->tilesets[num].metatiles[i].tiles[2] = (Uint16)prgRom[rom_offset++] + base;
        data->tilesets[num].metatiles[i].tiles[3] = (Uint16)prgRom[rom_offset++] + base;
    }

    return rom_offset;
}

void Rom_GetMapData(MapData *data) {
    // position in the PRG ROM
    int cursor = 0;

    // there are 3 tilesets (forest, cave, castle)
    data->numTilesets = 3;
    data->tilesets = malloc(data->numTilesets * sizeof(Tileset));

    // forest tileset has 164 metatiles
    #define FOREST_PALS (0x2790)
    cursor = init_tileset(data, 0, 164, tilesetBases[0], FOREST_PALS, cursor);

    // cave tileset has 164 metatiles
    #define CAVE_PALS (0x2834)
    cursor = init_tileset(data, 1, 164, tilesetBases[1], CAVE_PALS, cursor);

    // castle tileset has 164 metatiles (noticing a pattern?)
    #define CASTLE_PALS (0x28d8)
    cursor = init_tileset(data, 2, 164, tilesetBases[2], CASTLE_PALS, cursor);

    // there are 223 chunks
    data->numChunks = 223;
    data->chunks = malloc(data->numChunks * sizeof(data->chunks[0]));
    for (int i = 0; i < data->numChunks; i++) {
        for (int j = 0; j < ARRAY_LEN(data->chunks[0]); j++) {
            data->chunks[i][j] = (Uint16)prgRom[cursor++];
        }
    }

    // there are 159 screens
    data->numScreens = 159;
    data->screens = malloc(data->numScreens * sizeof(data->screens[0]));
    for (int i = 0; i < data->numScreens; i++) {
        for (int j = 0; j < ARRAY_LEN(data->screens[0]); j++) {
            data->screens[i][j] = (Uint16)prgRom[cursor++];
        }
    }

    // there are 16 rooms
    data->numRooms = 16;
    data->rooms = malloc(data->numRooms * sizeof(data->rooms[0]));
    for (int i = 0; i < data->numRooms; i++) {
        // get tileset
        #define ROOM_BANK_TBL (0x44bf)
        #define BASE_BANK 4
        data->rooms[i].tileset = (Uint16)(prgRom[ROOM_BANK_TBL + i] >> 4) - BASE_BANK;
    
        #define ROOM_PALETTE_TBL (0x35ff)
        for (int j = 0; j < ARRAY_LEN(data->rooms[0].palette); j++) {
            int palette_offset = ROOM_PALETTE_TBL + (i * 16) + j;
            data->rooms[i].palette[j] = prgRom[palette_offset];
        }

        // copy room data
        for (int j = 0; j < ARRAY_LEN(data->rooms[0].screenNums); j++) {
            data->rooms[i].screenNums[j] = (Uint16)prgRom[cursor++];
        }
        // in the original game, enemies were stored as offsets from a base
        // value that depended on the room number
        int room_sprite_bank = prgRom[ROOM_BANK_TBL + i] & 0x3;
        #define ENEMY_BASE_TBL (0x5bc8)
        Uint8 room_obj_base = prgRom[ENEMY_BASE_TBL + room_sprite_bank];
        for (int j = 0; j < ARRAY_LEN(data->rooms[0].spawns); j++) {
            Uint8 enemy_val = prgRom[cursor++];
            if (enemy_val >= 0xA0) {
                // items are A0-B0
                if (enemy_val <= 0xB0) {
                    data->rooms[i].spawns[j].type = SPAWN_TYPE_ITEM;
                }
                // fountains are B1-B7
                else if ((enemy_val & 0xF0) == 0xB0) {
                    data->rooms[i].spawns[j].type = SPAWN_TYPE_FOUNTAIN;
                }
                // bosses are D0-D1
                else {
                    data->rooms[i].spawns[j].type = SPAWN_TYPE_BOSS;
                    // D1 = "spawn boss on this screen", D0 = "don't"
                    data->rooms[i].spawns[j].count = (enemy_val & 0xf);
                }
                data->rooms[i].spawns[j].enemy = enemy_val;
            }
            else {
                data->rooms[i].spawns[j].type = SPAWN_TYPE_ENEMY;
                // low nybble = enemy object number offset
                data->rooms[i].spawns[j].enemy = (enemy_val & 0xf) + room_obj_base;
                // high nybble = enemy count
                data->rooms[i].spawns[j].count = enemy_val >> 4;
            }
        }
    }

    // load warp door data
    #define WARP_DOOR_X_TBL (0x401a)
    #define WARP_DOOR_Y_TBL (0x412a)
    #define WARP_DOOR_ROOM_TBL (0x423a)
    
    data->numWarpDoors = 272;
    data->warpDoors = malloc(data->numWarpDoors * sizeof(WarpDoor));
    for (int i = 0; i < data->numWarpDoors; i++) {
        data->warpDoors[i].xPos = prgRom[WARP_DOOR_X_TBL + i];
        data->warpDoors[i].yPos = prgRom[WARP_DOOR_Y_TBL + i];
        data->warpDoors[i].roomNum = prgRom[WARP_DOOR_ROOM_TBL + i];
    }
}