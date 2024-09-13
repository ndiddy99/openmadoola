/* camera.c: Camera handling
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

#include "camera.h"
#include "constants.h"
#include "lucia.h"
#include "map.h"
#include "object.h"

Fixed16 cameraX;
Fixed16 cameraY;

#define SCROLL_MODE_FREE (0)
#define SCROLL_MODE_X (1)
#define SCROLL_MODE_LOCKED (2)

#define SCROLL_MAX_X ((MAP_WIDTH_PIXELS - SCREEN_WIDTH) << 4)
#define SCROLL_MAX_Y ((MAP_HEIGHT_PIXELS - SCREEN_HEIGHT) << 4)

Uint8 scrollMode;

void Camera_SetX(Object *o) {
    if (scrollMode == SCROLL_MODE_LOCKED) {
        return;
    }

    // center the camera around the object (0x800 here means 128 pixels)
    cameraX.v = o->x.v - SCROLL_OFFSET_X;

    // min camera x threshold
    if (cameraX.v < 0) {
        cameraX.v = 0;
    }

    // don't scroll past the end of the level (minus a screen width)
    if (cameraX.v > SCROLL_MAX_X) {
        cameraX.v = SCROLL_MAX_X;
    }
}

void Camera_SetY(Object *o) {
    if (scrollMode != SCROLL_MODE_FREE) {
        return;
    }

    cameraY.v = o->y.v - SCROLL_OFFSET_Y;

    // min camera y threshold
    if (cameraY.v < 0) {
        cameraY.v = 0;
    }

    // don't scroll past the end of the level
    if (cameraY.v > SCROLL_MAX_Y) {
        cameraY.v = SCROLL_MAX_Y;
    }
}

void Camera_SetXY(Object *o) {
    Camera_SetX(o);
    Camera_SetY(o);
}

void Camera_LuciaScroll(Object *o) {
    Fixed16 cameraYBound;

    if (scrollMode == SCROLL_MODE_LOCKED) {
        return;
    }

    Camera_SetX(o);

    // only worry about y scrolling if we are able to scroll the y axis
    if (scrollMode == SCROLL_MODE_FREE) {
        // if we should scroll the screen vertically (at a scroll boundary or using the wing of madoola)
        if (((o->y.v - cameraY.v) < 0x300) || 
            ((o->y.v - cameraY.v) >= SCROLL_OFFSET_Y) || 
            usingWing || 
            // NOTE: the arcade version has this condition removed, I keep it because
            // removing it makes the camera movement worse IMO
            !((o->type == OBJ_LUCIA_AIR) || (o->type == OBJ_LUCIA_AIR_LOCKED)))
        {
            cameraYBound.v = o->y.v - SCROLL_OFFSET_Y;

            if (cameraY.v < cameraYBound.v) {
                cameraY.v += 0x40;
                if (cameraY.v > cameraYBound.v) {
                    cameraY.v = cameraYBound.v;
                }
            }

            else {
                cameraY.v -= 0x40;
                if (cameraY.v < cameraYBound.v) {
                    cameraY.v = cameraYBound.v;
                }

            }

            // min camera y threshold
            if (cameraY.v < 0) {
                cameraY.v = 0;
            }

            // don't scroll past the end of the level
            if (cameraY.v > SCROLL_MAX_Y) {
                cameraY.v = SCROLL_MAX_Y;
            }
        }
    }
}