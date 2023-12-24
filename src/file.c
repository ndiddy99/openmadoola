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

#if (defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)))
#define OM_UNIX
#endif

#include <stdio.h>
#include <stdlib.h>
#ifdef OM_UNIX
#include <sys/stat.h>
#endif
#include "constants.h"
#include "file.h"

#ifdef OM_UNIX
#define OM_HOMEDIR ".openmadoola"
static int filenameBuffLen = 256;
static char *filenameBuff = NULL;
#endif

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

static void checkBuffSize(int size) {
    if (!filenameBuff) { filenameBuff = malloc(filenameBuffLen); }
    if (size > filenameBuffLen) {
        filenameBuffLen = size;
        filenameBuff = realloc(filenameBuff, filenameBuffLen);
    }
}

FILE *File_Open(char *filename, const char *mode) {
#ifdef OM_UNIX
    const char dirname[] = OM_HOMEDIR;
    char *homedir = getenv("HOME");
    checkBuffSize(strlen(homedir) + 1 + strlen(dirname) + 1 + strlen(filename) + 1);
    strcpy(filenameBuff, homedir);
    strcat(filenameBuff, "/");
    strcat(filenameBuff, dirname);
    // try to make the directory in case it doesn't exist
    mkdir(filenameBuff, S_IRWXU);
    strcat(filenameBuff, "/");
    // concatenate the directory name with the requested filename
    strcat(filenameBuff, filename);
    return fopen(filenameBuff, mode);
#else
    return fopen(filename, mode);
#endif
}

#ifdef OM_UNIX
static char *resourceDirs[] = {
    NULL, // ~/.openmadoola/ placeholder
    "/usr/local/share/openmadoola/",
    "/usr/share/openmadoola/",
    "", // current working directory
};
#endif

FILE *File_OpenResource(char *filename) {
    // set up home data directory name
#ifdef OM_UNIX
    if (!resourceDirs[0]) {
        char *homedir = getenv("HOME");
        resourceDirs[0] = malloc(strlen(homedir) + 1 + strlen(OM_HOMEDIR) + 2);
        strcpy(resourceDirs[0], homedir);
        strcat(resourceDirs[0], "/");
        strcat(resourceDirs[0], OM_HOMEDIR);
        strcat(resourceDirs[0], "/");
    }

    for (int i = 0; i < ARRAY_LEN(resourceDirs); i++) {
        checkBuffSize(strlen(resourceDirs[i]) + strlen(filename) + 1);
        strcpy(filenameBuff, resourceDirs[i]);
        strcat(filenameBuff, filename);
        FILE *fp = fopen(filenameBuff, "rb");
        if (fp) { return fp; }
    }
    return NULL;
#else
    return fopen(filename, "rb");
#endif
}