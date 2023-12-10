/* file.c: File management utility functions
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

#include <stdio.h>
#include "constants.h"
#include "file.h"

void File_WriteUint16BE(Uint16 data, FILE *fp) {
    fputc(data >> 8, fp);
    fputc(data & 0xff, fp);
}

Uint16 File_ReadUint16BE(FILE *fp) {
    Uint16 i = fgetc(fp) << 8;
    i |= fgetc(fp);
    return i;
}

void File_WriteUint32BE(Uint32 data, FILE *fp) {
    fputc((data >> 24) & 0xff, fp);
    fputc((data >> 16) & 0xff, fp);
    fputc((data >>  8) & 0xff, fp);
    fputc(data & 0xff, fp);
}

Uint32 File_ReadUint32BE(FILE *fp) {
    Uint32 i = fgetc(fp) << 24;
    i |= (fgetc(fp) << 16);
    i |= (fgetc(fp) << 8);
    i |= fgetc(fp);
    return i;
}