/* highscore.h: High score code
 * Copyright (c) 2024 Nathan Misner
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

#pragma once
#include "constants.h"

/**
 * @brief Sets up high score code. Should be run before saving/displaying
 * high scores.
 */
void HighScore_Init(void);

/**
 * @brief Resets high score table to default.
 */
void HighScore_ResetScores(void);

/**
 * @brief Prints high score table to the screen.
 * @param x table x position
 * @param y table y position
 */
void HighScore_Print(int x, int y);

/**
 * @brief Gets the top score from the high scores table.
 * @returns The highest score.
 */
Uint32 HighScore_GetTopScore(void);

/**
 * @returns the score from the last completed game.
 */
Uint32 HighScore_GetLastScore(void);

/**
 * @brief If the user's score is sufficiently high, shows the name entry screen
 * @param score the score to save
 */
void HighScore_NameEntry(Uint32 score);
