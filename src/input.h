/* input.c: Platform-independent input abstraction
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
#include "constants.h"

typedef enum {
    INPUT_INVALID = 0,

    INPUT_KEY_MIN = 4,
    INPUT_KEY_A = 4,
    INPUT_KEY_B = 5,
    INPUT_KEY_C = 6,
    INPUT_KEY_D = 7,
    INPUT_KEY_E = 8,
    INPUT_KEY_F = 9,
    INPUT_KEY_G = 10,
    INPUT_KEY_H = 11,
    INPUT_KEY_I = 12,
    INPUT_KEY_J = 13,
    INPUT_KEY_K = 14,
    INPUT_KEY_L = 15,
    INPUT_KEY_M = 16,
    INPUT_KEY_N = 17,
    INPUT_KEY_O = 18,
    INPUT_KEY_P = 19,
    INPUT_KEY_Q = 20,
    INPUT_KEY_R = 21,
    INPUT_KEY_S = 22,
    INPUT_KEY_T = 23,
    INPUT_KEY_U = 24,
    INPUT_KEY_V = 25,
    INPUT_KEY_W = 26,
    INPUT_KEY_X = 27,
    INPUT_KEY_Y = 28,
    INPUT_KEY_Z = 29,
    INPUT_KEY_1 = 30,
    INPUT_KEY_2 = 31,
    INPUT_KEY_3 = 32,
    INPUT_KEY_4 = 33,
    INPUT_KEY_5 = 34,
    INPUT_KEY_6 = 35,
    INPUT_KEY_7 = 36,
    INPUT_KEY_8 = 37,
    INPUT_KEY_9 = 38,
    INPUT_KEY_0 = 39,
    INPUT_KEY_RETURN = 40,
    INPUT_KEY_ESCAPE = 41,
    INPUT_KEY_BACKSPACE = 42,
    INPUT_KEY_TAB = 43,
    INPUT_KEY_SPACE = 44,
    INPUT_KEY_MINUS = 45,
    INPUT_KEY_EQUALS = 46,
    INPUT_KEY_LEFTBRACKET = 47,
    INPUT_KEY_RIGHTBRACKET = 48,
    INPUT_KEY_BACKSLASH = 49,
    INPUT_KEY_NONUSHASH = 50,
    INPUT_KEY_SEMICOLON = 51,
    INPUT_KEY_APOSTROPHE = 52,
    INPUT_KEY_TILDE = 53,
    INPUT_KEY_COMMA = 54,
    INPUT_KEY_PERIOD = 55,
    INPUT_KEY_SLASH = 56,
    INPUT_KEY_CAPSLOCK = 57,
    INPUT_KEY_F1 = 58,
    INPUT_KEY_F2 = 59,
    INPUT_KEY_F3 = 60,
    INPUT_KEY_F4 = 61,
    INPUT_KEY_F5 = 62,
    INPUT_KEY_F6 = 63,
    INPUT_KEY_F7 = 64,
    INPUT_KEY_F8 = 65,
    INPUT_KEY_F9 = 66,
    INPUT_KEY_F10 = 67,
    INPUT_KEY_F11 = 68,
    INPUT_KEY_F12 = 69,
    INPUT_KEY_PRINTSCREEN = 70,
    INPUT_KEY_SCROLLLOCK = 71,
    INPUT_KEY_PAUSE = 72,
    INPUT_KEY_INSERT = 73,
    INPUT_KEY_HOME = 74,
    INPUT_KEY_PAGEUP = 75,
    INPUT_KEY_DELETE = 76,
    INPUT_KEY_END = 77,
    INPUT_KEY_PAGEDOWN = 78,
    INPUT_KEY_RIGHT = 79,
    INPUT_KEY_LEFT = 80,
    INPUT_KEY_DOWN = 81,
    INPUT_KEY_UP = 82,
    INPUT_KEY_NUMLOCK = 83,
    INPUT_KEY_KP_DIVIDE = 84,
    INPUT_KEY_KP_MULTIPLY = 85,
    INPUT_KEY_KP_MINUS = 86,
    INPUT_KEY_KP_PLUS = 87,
    INPUT_KEY_KP_ENTER = 88,
    INPUT_KEY_KP_1 = 89,
    INPUT_KEY_KP_2 = 90,
    INPUT_KEY_KP_3 = 91,
    INPUT_KEY_KP_4 = 92,
    INPUT_KEY_KP_5 = 93,
    INPUT_KEY_KP_6 = 94,
    INPUT_KEY_KP_7 = 95,
    INPUT_KEY_KP_8 = 96,
    INPUT_KEY_KP_9 = 97,
    INPUT_KEY_KP_0 = 98,
    INPUT_KEY_KP_PERIOD = 99,
    INPUT_KEY_LCTRL = 100,
    INPUT_KEY_LSHIFT = 101,
    INPUT_KEY_LALT = 102,
    INPUT_KEY_LGUI = 103,
    INPUT_KEY_RCTRL = 104,
    INPUT_KEY_RSHIFT = 105,
    INPUT_KEY_RALT = 106,
    INPUT_KEY_RGUI = 107,
    INPUT_KEY_MAX = 107,

    INPUT_GAMEPAD_MIN = 108,
    INPUT_GAMEPAD_A = 108,
    INPUT_GAMEPAD_Y = 109,
    INPUT_GAMEPAD_B = 110,
    INPUT_GAMEPAD_X = 111,
    INPUT_GAMEPAD_L_SHOULDER = 112,
    INPUT_GAMEPAD_R_SHOULDER = 113,
    INPUT_GAMEPAD_L_TRIGGER = 114,
    INPUT_GAMEPAD_R_TRIGGER = 115,
    INPUT_GAMEPAD_SELECT = 116,
    INPUT_GAMEPAD_HOME = 117,
    INPUT_GAMEPAD_START = 118,
    INPUT_GAMEPAD_L_STICK_PRESS = 119,
    INPUT_GAMEPAD_R_STICK_PRESS = 120,
    INPUT_GAMEPAD_DPAD_UP = 121,
    INPUT_GAMEPAD_DPAD_DOWN = 122,
    INPUT_GAMEPAD_DPAD_LEFT = 123,
    INPUT_GAMEPAD_DPAD_RIGHT = 124,
    INPUT_GAMEPAD_MAX = 124,
    NUM_INPUT_BUTTONS,
} INPUT_BUTTON;

extern Uint8 inputState[NUM_INPUT_BUTTONS];

/**
 * @brief Used by the platform code to set the state of a given button
 * @param button see INPUT_BUTTON enum
 * @param state Nonzero for pressed, zero for released
 */
void Input_SetState(int button, Uint8 state);

/**
 * @param button The button to get the name of (see INPUT_BUTTON enum)
 * @returns the name of that button
 */
const char *Input_ButtonName(int button);

/**
 * @brief Sets up a function to be run when a button is pressed
 * @param func press callback
 */
void Input_SetOnPressFunc(void (*func)(int));