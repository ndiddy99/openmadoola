/* input.c: Platform-independent input abstraction
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

#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "input.h"

static const char *buttonNames[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    [INPUT_KEY_A] = "A",
    [INPUT_KEY_B] = "B",
    [INPUT_KEY_C] = "C",
    [INPUT_KEY_D] = "D",
    [INPUT_KEY_E] = "E",
    [INPUT_KEY_F] = "F",
    [INPUT_KEY_G] = "G",
    [INPUT_KEY_H] = "H",
    [INPUT_KEY_I] = "I",
    [INPUT_KEY_J] = "J",
    [INPUT_KEY_K] = "K",
    [INPUT_KEY_L] = "L",
    [INPUT_KEY_M] = "M",
    [INPUT_KEY_N] = "N",
    [INPUT_KEY_O] = "O",
    [INPUT_KEY_P] = "P",
    [INPUT_KEY_Q] = "Q",
    [INPUT_KEY_R] = "R",
    [INPUT_KEY_S] = "S",
    [INPUT_KEY_T] = "T",
    [INPUT_KEY_U] = "U",
    [INPUT_KEY_V] = "V",
    [INPUT_KEY_W] = "W",
    [INPUT_KEY_X] = "X",
    [INPUT_KEY_Y] = "Y",
    [INPUT_KEY_Z] = "Z",
    [INPUT_KEY_1] = "1",
    [INPUT_KEY_2] = "2",
    [INPUT_KEY_3] = "3",
    [INPUT_KEY_4] = "4",
    [INPUT_KEY_5] = "5",
    [INPUT_KEY_6] = "6",
    [INPUT_KEY_7] = "7",
    [INPUT_KEY_8] = "8",
    [INPUT_KEY_9] = "9",
    [INPUT_KEY_0] = "0",
    [INPUT_KEY_RETURN] = "RETURN",
    [INPUT_KEY_ESCAPE] = "ESCAPE",
    [INPUT_KEY_BACKSPACE] = "BACKSP",
    [INPUT_KEY_TAB] = "TAB",
    [INPUT_KEY_SPACE] = "SPACE",
    [INPUT_KEY_MINUS] = "MINUS",
    [INPUT_KEY_EQUALS] = "EQUALS",
    [INPUT_KEY_LEFTBRACKET] = "LBRACKET",
    [INPUT_KEY_RIGHTBRACKET] = "RBRACKET",
    [INPUT_KEY_BACKSLASH] = "BSLASH",
    [INPUT_KEY_NONUSHASH] = "HASH",
    [INPUT_KEY_SEMICOLON] = "SMICOL",
    [INPUT_KEY_APOSTROPHE] = "APO",
    [INPUT_KEY_TILDE] = "TILDE",
    [INPUT_KEY_COMMA] = "COMMA",
    [INPUT_KEY_PERIOD] = "PERIOD",
    [INPUT_KEY_SLASH] = "SLASH",
    [INPUT_KEY_CAPSLOCK] = "CAPS",
    [INPUT_KEY_F1] = "F1",
    [INPUT_KEY_F2] = "F2",
    [INPUT_KEY_F3] = "F3",
    [INPUT_KEY_F4] = "F4",
    [INPUT_KEY_F5] = "F5",
    [INPUT_KEY_F6] = "F6",
    [INPUT_KEY_F7] = "F7",
    [INPUT_KEY_F8] = "F8",
    [INPUT_KEY_F9] = "F9",
    [INPUT_KEY_F10] = "F10",
    [INPUT_KEY_F11] = "F11",
    [INPUT_KEY_F12] = "F12",
    [INPUT_KEY_PRINTSCREEN] = "PRTSC",
    [INPUT_KEY_SCROLLLOCK] = "SCRLK",
    [INPUT_KEY_PAUSE] = "PAUSE",
    [INPUT_KEY_INSERT] = "INSERT",
    [INPUT_KEY_HOME] = "HOME",
    [INPUT_KEY_PAGEUP] = "PG UP",
    [INPUT_KEY_DELETE] = "DELETE",
    [INPUT_KEY_END] = "END",
    [INPUT_KEY_PAGEDOWN] = "PG DOWN",
    [INPUT_KEY_RIGHT] = "RIGHT",
    [INPUT_KEY_LEFT] = "LEFT",
    [INPUT_KEY_DOWN] = "DOWN",
    [INPUT_KEY_UP] = "UP",
    [INPUT_KEY_NUMLOCK] = "NLOCK",
    [INPUT_KEY_KP_DIVIDE] = "KPDIV",
    [INPUT_KEY_KP_MULTIPLY] = "KPMUL",
    [INPUT_KEY_KP_MINUS] = "KPMINUS",
    [INPUT_KEY_KP_PLUS] = "KPPLUS",
    [INPUT_KEY_KP_ENTER] = "KPENTER",
    [INPUT_KEY_KP_1] = "KP1",
    [INPUT_KEY_KP_2] = "KP2",
    [INPUT_KEY_KP_3] = "KP3",
    [INPUT_KEY_KP_4] = "KP4",
    [INPUT_KEY_KP_5] = "KP5",
    [INPUT_KEY_KP_6] = "KP6",
    [INPUT_KEY_KP_7] = "KP7",
    [INPUT_KEY_KP_8] = "KP8",
    [INPUT_KEY_KP_9] = "KP9",
    [INPUT_KEY_KP_0] = "KP0",
    [INPUT_KEY_KP_PERIOD] = "KPPERIOD",
    [INPUT_KEY_LCTRL] = "LCTRL",
    [INPUT_KEY_LSHIFT] = "LSHIFT",
    [INPUT_KEY_LALT] = "LALT",
    [INPUT_KEY_LGUI] = "LGUI",
    [INPUT_KEY_RCTRL] = "RCTRL",
    [INPUT_KEY_RSHIFT] = "RSHIFT",
    [INPUT_KEY_RALT] = "RALT",
    [INPUT_KEY_RGUI] = "RGUI",

    [INPUT_GAMEPAD_A] = "A",
    [INPUT_GAMEPAD_Y] = "Y",
    [INPUT_GAMEPAD_B] = "B",
    [INPUT_GAMEPAD_X] = "X",
    [INPUT_GAMEPAD_L_SHOULDER] = "LSHLDR",
    [INPUT_GAMEPAD_R_SHOULDER] = "RSHLDR",
    [INPUT_GAMEPAD_L_TRIGGER] = "LTRIG",
    [INPUT_GAMEPAD_R_TRIGGER] = "RTRIG",
    [INPUT_GAMEPAD_SELECT] = "SELECT",
    [INPUT_GAMEPAD_HOME] = "HOME",
    [INPUT_GAMEPAD_START] = "START",
    [INPUT_GAMEPAD_L_STICK_PRESS] = "LSTK",
    [INPUT_GAMEPAD_R_STICK_PRESS] = "RSTK",
    [INPUT_GAMEPAD_DPAD_UP] = "DPUP",
    [INPUT_GAMEPAD_DPAD_DOWN] = "DPDOWN",
    [INPUT_GAMEPAD_DPAD_LEFT] = "DPLEFT",
    [INPUT_GAMEPAD_DPAD_RIGHT] = "DPRIGHT",
};

Uint8 inputState[NUM_INPUT_BUTTONS];

static void (*onPress)(int);


void Input_SetState(int button, Uint8 state) {
    if ((button < 0) || (button >= NUM_INPUT_BUTTONS)) {
        printf("Invalid button: %d\n", button);
        return;
    }
    if (state && !inputState[button] && onPress) {
        onPress(button);
    }
    inputState[button] = state;
}

const char *Input_ButtonName(int button) {
    if ((button < 0) || (button >= NUM_INPUT_BUTTONS)) {
        return NULL;
    }
    return buttonNames[button];
}

void Input_SetOnPressFunc(void (*func)(int)) {
    onPress = func;
}
