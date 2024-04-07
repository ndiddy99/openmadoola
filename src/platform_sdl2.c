/* platform_sdl2.c: SDL2 specific code
 * Copyright (c) 2023, 2024 Nathan Misner
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

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "db.h"
#include "file.h"
#include "graphics.h"
#include "input.h"
#include "nanotime.h"
#include "platform.h"

// --- video stuff ---
static Uint8 frameStarted = 0;
static Uint8 scale = 3;
static Uint8 fullscreen = 0;
static Uint32 *framebuffer;
// destination to draw to when drawing in fullscreen
static SDL_Rect fullscreenRect;
static SDL_Window *window;
static SDL_Renderer *renderer;
// texture that the game engine draws to
static SDL_Texture *drawTexture;
static SDL_Rect screenRect = {
    .x = TILE_WIDTH,
    .y = TILE_HEIGHT,
    .w = SCREEN_WIDTH,
    .h = SCREEN_HEIGHT,
};
// texture that gets nearest-neighbor scaled
static SDL_Texture *scaleTexture;
static int vsync;
static nanotime_step_data stepData;

// --- audio stuff ---
static SDL_AudioDeviceID audioDevice;

// --- controller stuff ---
static const int gamepadMap[] = {
    [SDL_CONTROLLER_BUTTON_A] = INPUT_GAMEPAD_A,
    [SDL_CONTROLLER_BUTTON_B] = INPUT_GAMEPAD_B,
    [SDL_CONTROLLER_BUTTON_X] = INPUT_GAMEPAD_X,
    [SDL_CONTROLLER_BUTTON_Y] = INPUT_GAMEPAD_Y,
    [SDL_CONTROLLER_BUTTON_BACK] = INPUT_GAMEPAD_SELECT,
    [SDL_CONTROLLER_BUTTON_GUIDE] = INPUT_GAMEPAD_HOME,
    [SDL_CONTROLLER_BUTTON_START] = INPUT_GAMEPAD_START,
    [SDL_CONTROLLER_BUTTON_LEFTSTICK] = INPUT_GAMEPAD_L_STICK_PRESS,
    [SDL_CONTROLLER_BUTTON_RIGHTSTICK] = INPUT_GAMEPAD_R_STICK_PRESS,
    [SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = INPUT_GAMEPAD_L_SHOULDER,
    [SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = INPUT_GAMEPAD_R_SHOULDER,
    [SDL_CONTROLLER_BUTTON_DPAD_UP] = INPUT_GAMEPAD_DPAD_UP,
    [SDL_CONTROLLER_BUTTON_DPAD_DOWN] = INPUT_GAMEPAD_DPAD_DOWN,
    [SDL_CONTROLLER_BUTTON_DPAD_LEFT] = INPUT_GAMEPAD_DPAD_LEFT,
    [SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = INPUT_GAMEPAD_DPAD_RIGHT,
    [SDL_CONTROLLER_BUTTON_MAX] = INPUT_INVALID,
};
static SDL_GameController *controller;

// static function declarations
static void Platform_PumpEvents(void);

static int Platform_InitVideo(void) {
    // set up window
    int windowWidth = ((int)(SCREEN_WIDTH * scale * WINDOW_PAR));
    int windowHeight = SCREEN_HEIGHT * scale;
    window = SDL_CreateWindow(
        "OpenMadoola",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_SHOWN);
    if (!window) {
        ERROR_MSG(SDL_GetError());
        return 0;
    }

    // set up renderer
    SDL_DisplayMode displayMode;
    SDL_GetWindowDisplayMode(window, &displayMode);
    int refreshRate = displayMode.refresh_rate;
    // round up if refresh rate is 59 or something
    if (refreshRate && (((refreshRate + 1) % 60) == 0)) {
        refreshRate++;
    }
    if (refreshRate && ((refreshRate % 60) == 0)) {
        vsync = refreshRate / 60;
    }
    else {
        vsync = 0;
    }
    renderer = SDL_CreateRenderer(window, -1, vsync ? SDL_RENDERER_PRESENTVSYNC : 0);
    if (!renderer) {
        ERROR_MSG(SDL_GetError());
        return 0;
    }
    nanotime_step_init(&stepData, (uint64_t)(NANOTIME_NSEC_PER_SEC / 60), nanotime_now_max(), nanotime_now, nanotime_sleep);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    drawTexture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    if (!drawTexture) {
        ERROR_MSG(SDL_GetError());
        return 0;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    int scaledWidth = SCREEN_WIDTH * scale;
    int scaledHeight = SCREEN_HEIGHT * scale;
    scaleTexture = SDL_CreateTexture(renderer,
                                     SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_TARGET,
                                     scaledWidth, scaledHeight);
    if (!scaleTexture) {
        ERROR_MSG(SDL_GetError());
        return 0;
    }
    return 1;
}

static void Platform_DestroyVideo(void) {
    SDL_DestroyTexture(drawTexture);
    SDL_DestroyTexture(scaleTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void Platform_StartFrame(void) {
    if (frameStarted) {
        printf("ERROR: Started frame without ending the previous frame!\n");
    }
    frameStarted = 1;
    Platform_PumpEvents();
    int pitch;
    SDL_LockTexture(drawTexture, NULL, (void **)&framebuffer, &pitch);
}

void Platform_EndFrame(void) {
    if (!frameStarted) {
        printf("ERROR: Ended frame without starting it!\n");
    }
    frameStarted = 0;
    SDL_UnlockTexture(drawTexture);
    SDL_SetRenderTarget(renderer, scaleTexture);
    // stretch framebuffer horizontally w/ bilinear so the pixel aspect ratio is correct
    SDL_RenderCopy(renderer, drawTexture, &screenRect, NULL);
    SDL_SetRenderTarget(renderer, NULL);

    // monitor framerate isn't a multiple of 60, so wait in software
    if (!vsync) {
        nanotime_step(&stepData);
    }

    for (int i = 0; i < (vsync ? vsync : 1); i++) {
        SDL_RenderClear(renderer);
        if (fullscreen) {
            SDL_RenderCopy(renderer, scaleTexture, NULL, &fullscreenRect);
        }
        else {
            SDL_RenderCopy(renderer, scaleTexture, NULL, NULL);
        }
        SDL_RenderPresent(renderer);
    }
}

Uint32 *Platform_GetFramebuffer(void) {
    return framebuffer;
}

int Platform_GetVideoScale(void) {
    return scale;
}

int Platform_SetVideoScale(int requested) {
    if ((requested > 0) && !fullscreen) {
        scale = requested;
        // resize window
        int windowWidth = ((int)(SCREEN_WIDTH * scale * WINDOW_PAR));
        int windowHeight = SCREEN_HEIGHT * scale;
        SDL_SetWindowSize(window, windowWidth, windowHeight);
        SDL_SetWindowPosition(window,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED);
        // resize scale framebuffer to fit window
        SDL_DestroyTexture(scaleTexture);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        int scaledWidth = SCREEN_WIDTH * scale;
        int scaledHeight = SCREEN_HEIGHT * scale;
        scaleTexture = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_TARGET,
                                         scaledWidth, scaledHeight);
    }

    DB_Set("scale", &scale, 1);
    DB_Save();
    return scale;
}

int Platform_SetFullscreen(int requested) {
    if (requested) {
        fullscreen = 1;
        // get desktop display resolution
        SDL_DisplayMode displayMode;
        SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(window), &displayMode);
        // make window fullscreen at native res
        SDL_SetWindowSize(window, displayMode.w, displayMode.h);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        // disable cursor
        SDL_ShowCursor(SDL_DISABLE);
        // resize scale framebuffer to fit screen
        SDL_DestroyTexture(scaleTexture);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        int fullscreenScale = displayMode.h / SCREEN_HEIGHT;
        int scaledWidth = SCREEN_WIDTH * fullscreenScale;
        int scaledHeight = SCREEN_HEIGHT * fullscreenScale;
        scaleTexture = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_TARGET,
                                         scaledWidth, scaledHeight);
        // set up destination area
        fullscreenRect.w = (int) ((float) scaledWidth * WINDOW_PAR);
        fullscreenRect.h = scaledHeight;
        fullscreenRect.x = (displayMode.w / 2) - (fullscreenRect.w / 2);
        fullscreenRect.y = (displayMode.h / 2) - (fullscreenRect.h / 2);
    }
    else {
        fullscreen = 0;
        // make window windowed at last scale
        SDL_SetWindowFullscreen(window, 0);
        Platform_SetVideoScale(scale);
    }

    DB_Set("fullscreen", &fullscreen, 1);
    DB_Save();
    return fullscreen;
}

int Platform_GetFullscreen(void) {
    return fullscreen;
}

static int Platform_InitAudio(void) {
    SDL_AudioSpec spec = { 0 };
    spec.freq = 44100;
    spec.format = AUDIO_S16;
    spec.channels = 1;
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (!audioDevice) {
        ERROR_MSG(SDL_GetError());
        return 0;
    }
    SDL_PauseAudioDevice(audioDevice, 0);
    return 1;
}

static void Platform_DestroyAudio(void) {
    SDL_CloseAudioDevice(audioDevice);
}

void Platform_QueueSamples(Sint16 *samples, int count) {
    SDL_QueueAudio(audioDevice, (void *)samples, count * sizeof(Sint16));
}

int Platform_GetQueuedSamples(void) {
    return (int)SDL_GetQueuedAudioSize(audioDevice) / sizeof(Sint16);
}

static SDL_GameController *Platform_FindController(void) {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }

    return NULL;
}

int Platform_Init(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 0;
    }
    if (!Platform_InitVideo()) { return 0; }
    if (!Platform_InitAudio()) { return 0; }
    controller = Platform_FindController();

    DBEntry *entry = DB_Find("scale");
    if (entry) { Platform_SetVideoScale(entry->data[0]); }
    entry = DB_Find("fullscreen");
    if (entry) { Platform_SetFullscreen(entry->data[0]); }

    return 1;
}

void Platform_Quit(void) {
    Platform_DestroyVideo();
    Platform_DestroyAudio();
    SDL_Quit();
    exit(0);
}

static void Platform_PumpEvents(void) {
    SDL_Event event;
    int button;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        // keyboard events
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            // alt+enter = toggle fullscreen
            if ((event.type == SDL_KEYDOWN) &&
                (event.key.keysym.scancode == SDL_SCANCODE_RETURN) &&
                (event.key.keysym.mod & KMOD_ALT)) {
                Platform_SetFullscreen(!Platform_GetFullscreen());
                break;
            }

            button = event.key.keysym.scancode;
            // these are discontiguous with the rest of the scancodes so we make them
            // match the button enum in input.h
            if ((button >= SDL_SCANCODE_LCTRL) && (button <= SDL_SCANCODE_RGUI)) {
                button = (button - SDL_SCANCODE_LCTRL) + INPUT_KEY_LCTRL;
            }
            if ((button >= INPUT_KEY_A) && (button <= INPUT_KEY_RGUI)) {
                Input_SetState(button, event.type == SDL_KEYDOWN);
            }
            break;

        // controller events
        case SDL_CONTROLLERDEVICEADDED:
            controller = SDL_GameControllerOpen(event.cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            controller = Platform_FindController();
            break;

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            if (event.cbutton.button < ARRAY_LEN(gamepadMap)) {
                // match the button to the enum in input.h
                button = gamepadMap[event.cbutton.button];
                if (button != INPUT_INVALID) {
                    Input_SetState(button, event.type == SDL_CONTROLLERBUTTONDOWN);
                }
            }
            break;

        // handle quit
        case SDL_QUIT:
            Platform_Quit();
            break;

        }
    }
}
