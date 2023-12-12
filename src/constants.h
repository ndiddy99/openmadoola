/* constants.h: Useful constants and macros
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
#include <SDL.h>

#define OPENMADOOLA_VERSION "1.0"

#define SCREEN_WIDTH (256) // original NES aspect ratio
//#define SCREEN_WIDTH (368) // approx. 16:9 aspect ratio
#define SCREEN_HEIGHT (240)

// pixel aspect ratio
#define WINDOW_PAR (8.0f / 7.0f)

#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define STRINGIFY_IMPL(x) #x
#define ERROR_MSG(x) (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, __FILE__ ":" STRINGIFY(__LINE__) ": ", x, NULL))
#define ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define CLAMP(x, low, high) do { if ((x) < (low)) { (x) = (low); } if ((x) > (high)) { (x) = (high); } } while(0)

// this makes it easier to access the low/high bytes of the value
typedef union {
    struct {
        Uint8 l;
        Sint8 h;
    } f;
    Sint16 v;
} Fixed16;
