/* hud.h: HUD display code
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

#pragma once
#include "constants.h"

/**
 * @brief Shows the health and magic counters
 */
void HUD_Display(void);

/**
 * @brief Reserves sprites for the weapon display (necessary to split out because
 * the sprite list doesn't get cleared when the game is paused, but the player
 * can still switch weapons)
 * @param x display x coordinate
 * @param y display y coordinate
 */
void HUD_WeaponInit(Sint16 x, Sint16 y);

/**
 * @brief Updates the HUD weapon display from the currently selected weapon
 */
void HUD_Weapon(void);
