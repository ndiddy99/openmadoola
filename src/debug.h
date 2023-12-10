/* debug.h: debug GUI
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
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL.h>
#include "constants.h"

int Debug_Init(void);
SDL_Window *Debug_GetWindow(void);
SDL_Renderer *Debug_GetRenderer(void);
void Debug_StartFrame(void);
void Debug_EndFrame(SDL_Texture *texture);

#ifdef __cplusplus
}
#endif
