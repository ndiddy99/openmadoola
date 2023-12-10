/* game.h: Game related management code and global variables
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
#include <stdnoreturn.h>
#include "constants.h"

#define GAME_TYPE_ORIGINAL 0
#define GAME_TYPE_PLUS 1
extern Uint8 gameType;

extern Uint8 spritePalettes[16];
extern Uint8 stage;
extern Uint8 highestReachedStage;
extern Uint8 orbCollected;
extern Uint8 roomChangeTimer;
extern Uint8 bossActive;
extern Uint8 numBossObjs;
extern Sint8 keywordDisplay;
extern Uint8 bossDefeated[16];
extern Uint8 frameCount;

/**
 * @brief Initializes the title screen, game, etc and runs the game loop.
*/
noreturn void Game_Run(void);

/**
 * @brief Plays the song associated with the current room.
*/
void Game_PlayRoomSong(void);
