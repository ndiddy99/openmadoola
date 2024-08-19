/* util.c: General utility functions
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

#include "constants.h"

void Util_SaveUint16(Uint16 num, Uint8 *out) {
    out[0] = (num >> 8) & 0xff;
    out[1] = num & 0xff;
}

Uint16 Util_LoadUint16(Uint8 *buff) {
    Uint16 num = (Uint16)buff[0] << 8;
    num |= (Uint16)buff[1];
    return num;
}

void Util_SaveSint16(Sint16 num, Uint8 *out) {
    Util_SaveUint16((Uint16)num, out);
}

Sint16 Util_LoadSint16(Uint8 *buff) {
    return (Sint16)Util_LoadUint16(buff);
}

void Util_SaveUint32(Uint32 num, Uint8 *out) {
    out[0] = (num >> 24) & 0xff;
    out[1] = (num >> 16) & 0xff;
    out[2] = (num >> 8) & 0xff;
    out[3] = num & 0xff;
}

Uint32 Util_LoadUint32(Uint8 *buff) {
    Uint32 num = (Uint32)buff[0] << 24;
    num |= ((Uint32)buff[1] << 16);
    num |= ((Uint32)buff[2] << 8);
    num |= (Uint32)buff[3];
    return num;
}

void Util_SaveSint32(Sint32 num, Uint8 *out) {
    Util_SaveUint32((Uint32)num, out);
}

Sint32 Util_LoadSint32(Uint8 *buff) {
    return (Sint32)Util_LoadUint32(buff);
}
