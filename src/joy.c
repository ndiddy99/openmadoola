/* joy.c: Input reading code
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
#include "input.h"
#include "joy.h"

Uint32 joy;
Uint32 joyEdge;
Uint8 joyDir;

static int keyMappings[] = {
    INPUT_KEY_D,
    INPUT_KEY_A,
    INPUT_KEY_S,
    INPUT_KEY_W,
    INPUT_KEY_RETURN,
    INPUT_KEY_RSHIFT,
    INPUT_KEY_J,
    INPUT_KEY_K,
};

static int gamepadMappings[] = {
    INPUT_GAMEPAD_DPAD_RIGHT,
    INPUT_GAMEPAD_DPAD_LEFT,
    INPUT_GAMEPAD_DPAD_DOWN,
    INPUT_GAMEPAD_DPAD_UP,
    INPUT_GAMEPAD_START,
    INPUT_GAMEPAD_SELECT,
    INPUT_GAMEPAD_X,
    INPUT_GAMEPAD_A,
};

static const Uint8 direction_table[] = {
    0, // no directions
    3, // right
    7, // left
    0, // left & right
    5, // down
    4, // down & right
    6, // down & left
    5, // down, left, & right
    1, // up
    2, // up & right
    8, // up & left
    1, // up, left, & right
    0, // up & down
    3, // up, down, & right
    7, // up, down, & left
    0, // up, down, left, & right
};

static int Joy_ButtonToIndex(Uint32 joyButton) {
    int index = 0;
    while (!(joyButton & 1)) {
        index++;
        if (index >= ARRAY_LEN(keyMappings)) { return -1; }
        joyButton >>= 1;
    }
    return index;
}

void Joy_MapKey(int key, Uint32 joyButton) {
    int index = Joy_ButtonToIndex(joyButton);
    if (index < 0) { return; }
    keyMappings[index] = key;
}

const char *Joy_StrKey(Uint32 joyButton) {
    int index = Joy_ButtonToIndex(joyButton);
    if (index < 0) { return NULL; }
    return Input_ButtonName(keyMappings[index]);
}

void Joy_MapGamepad(int gamepadButton, Uint32 joyButton) {
    int index = Joy_ButtonToIndex(joyButton);
    if (index < 0) { return; }
    gamepadMappings[index] = gamepadButton;
}

const char *Joy_StrGamepad(Uint32 joyButton) {
    int index = Joy_ButtonToIndex(joyButton);
    if (index < 0) { return NULL; }
    return Input_ButtonName(gamepadMappings[index]);
}

void Joy_SaveMappings(void) {
    FILE *fp = fopen("joy.cfg", "wb");
    if (!fp) {
        ERROR_MSG("Couldn't open joy.cfg for writing!");
        return;
    }
    for (int i = 0; i < ARRAY_LEN(keyMappings); i++) {
        File_WriteUint16BE((Uint16)keyMappings[i], fp);
        File_WriteUint16BE((Uint16)gamepadMappings[i], fp);
    }
    fclose(fp);
}

void Joy_Init(void) {
    FILE *fp = fopen("joy.cfg", "rb");
    if (!fp) {
        return;
    }

    for (int i = 0; i < ARRAY_LEN(keyMappings); i++) {
        keyMappings[i] = File_ReadUint16BE(fp);
        gamepadMappings[i] = File_ReadUint16BE(fp);
    }
    fclose(fp);
}

void Joy_Update(void) {
    Uint32 joyLast = joy;
    joy = 0;

    Uint32 mask = 1;
    for (int i = 0; i < ARRAY_LEN(keyMappings); i++) {
        if (inputState[keyMappings[i]])     { joy |= mask; }
        if (inputState[gamepadMappings[i]]) { joy |= mask; }
        mask <<= 1;
    }

    joyEdge = (~joyLast) & joy;
    joyDir = direction_table[joy & 0xf];
}
