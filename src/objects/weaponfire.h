/* weaponfire.h: Flame sword and magic bomb fire object code
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
#include "object.h"

/**
 * @brief flame sword fire object code
 * @param o pointer to the object
*/
void WeaponFire_FlameSwordObj(Object *o);

/**
 * @brief magic bomb fire object code
 * @param o pointer to the object
*/
void WeaponFire_MagicBombObj(Object *o);
