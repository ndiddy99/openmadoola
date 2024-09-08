/* screen.h: Title, status, stage screens
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

#pragma once

/**
 * @brief Draws the status screen, runs for 3 seconds or until the user presses start.
*/
void Screen_Status(void);

/**
 * @brief Draws the stage screen, runs for 3 seconds or until the user presses start.
*/
void Screen_Stage(void);

/**
 * @brief Draws the game over screen, runs for 4 seconds.
*/
void Screen_GameOver(void);

/**
 * @brief Draws the keyword screen, runs for 3.5 seconds
*/
void Screen_Keyword(void);
