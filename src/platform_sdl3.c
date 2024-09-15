/* platform_sdl3.c: SDL3 specific code
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

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "db.h"
#include "graphics.h"
#include "input.h"
#include "nanotime.h"
#include "nes_ntsc.h"
#include "palette.h"
#include "platform.h"

// SDL stopped publishing test versions of SDL3 so I can't test with this until
// SDL3 comes out.
#error Not currently supported, please use SDL2 for now.

// --- video stuff ---
static Uint8 frameStarted = 0;
static Uint8 scale = 3;
static Uint8 fullscreen = 0;
// NES framebuffer
static Uint8 framebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
// destination to draw to when drawing in fullscreen
static SDL_FRect fullscreenRect;
static SDL_Window *window;
static SDL_Renderer *renderer;
// texture that the game engine draws to
static SDL_Texture *drawTexture;
// texture that gets nearest-neighbor scaled
static SDL_Texture *scaleTexture;
static int vsync;
static nanotime_step_data stepData;
static nes_ntsc_t ntsc;
static Uint8 ntscEnabled = 0;

// --- audio stuff ---
static SDL_AudioStream *audioStream;

// --- controller stuff ---
static const int gamepadMap[] = {
    [SDL_GAMEPAD_BUTTON_SOUTH] = INPUT_GAMEPAD_A,
    [SDL_GAMEPAD_BUTTON_EAST] = INPUT_GAMEPAD_B,
    [SDL_GAMEPAD_BUTTON_WEST] = INPUT_GAMEPAD_X,
    [SDL_GAMEPAD_BUTTON_NORTH] = INPUT_GAMEPAD_Y,
    [SDL_GAMEPAD_BUTTON_BACK] = INPUT_GAMEPAD_SELECT,
    [SDL_GAMEPAD_BUTTON_GUIDE] = INPUT_GAMEPAD_HOME,
    [SDL_GAMEPAD_BUTTON_START] = INPUT_GAMEPAD_START,
    [SDL_GAMEPAD_BUTTON_LEFT_STICK] = INPUT_GAMEPAD_L_STICK_PRESS,
    [SDL_GAMEPAD_BUTTON_RIGHT_STICK] = INPUT_GAMEPAD_R_STICK_PRESS,
    [SDL_GAMEPAD_BUTTON_LEFT_SHOULDER] = INPUT_GAMEPAD_L_SHOULDER,
    [SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER] = INPUT_GAMEPAD_R_SHOULDER,
    [SDL_GAMEPAD_BUTTON_DPAD_UP] = INPUT_GAMEPAD_DPAD_UP,
    [SDL_GAMEPAD_BUTTON_DPAD_DOWN] = INPUT_GAMEPAD_DPAD_DOWN,
    [SDL_GAMEPAD_BUTTON_DPAD_LEFT] = INPUT_GAMEPAD_DPAD_LEFT,
    [SDL_GAMEPAD_BUTTON_DPAD_RIGHT] = INPUT_GAMEPAD_DPAD_RIGHT,
    [SDL_GAMEPAD_BUTTON_MAX] = INPUT_INVALID,
};
static SDL_Gamepad *gamepad;

// static function declarations
static void Platform_PumpEvents(void);

static int Platform_InitVideo(void) {
    // set up window
    int windowWidth = ((int)(SCREEN_WIDTH * scale * PIXEL_ASPECT_RATIO));
    int windowHeight = SCREEN_HEIGHT * scale;
    window = SDL_CreateWindow(
        "OpenMadoola",
        windowWidth, windowHeight,
        0);
    if (!window) {
        Platform_ShowError("Error creating window: %s", SDL_GetError());
        return 0;
    }

    // set up renderer
    const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));
    if (!displayMode) {
        Platform_ShowError("Error getting display mode: %s", SDL_GetError());
        return 0;
    }
    int refreshRate = (int)(displayMode->refresh_rate);
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
    renderer = SDL_CreateRenderer(window, NULL, vsync ? SDL_RENDERER_PRESENTVSYNC : 0);
    if (!renderer) {
        Platform_ShowError("Error creating renderer: %s", SDL_GetError());
        return 0;
    }
    nanotime_step_init(&stepData, (uint64_t)(NANOTIME_NSEC_PER_SEC / 60), nanotime_now_max(), nanotime_now, nanotime_sleep);
    nes_ntsc_setup_t setup = nes_ntsc_composite;
    setup.saturation = -0.1;
    // Sony CXA2025AS decoder matrix
    float matrix[6] = { 1.630f, 0.317f, -0.378f, -0.466f, -1.089f, 1.677f };
    setup.decoder_matrix = matrix;
    nes_ntsc_init(&ntsc, &setup);

    int drawWidth;
    if (ntscEnabled) {
        drawWidth = NES_NTSC_OUT_WIDTH(SCREEN_WIDTH);
    }
    else {
        drawWidth = SCREEN_WIDTH;
    }
    drawTexture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    drawWidth, SCREEN_HEIGHT);
    if (!drawTexture) {
        Platform_ShowError("Error creating drawTexture: %s", SDL_GetError());
        return 0;
    }
    SDL_SetTextureScaleMode(drawTexture, SDL_SCALEMODE_NEAREST);
    int scaledWidth = SCREEN_WIDTH * scale;
    int scaledHeight = SCREEN_HEIGHT * scale;
    scaleTexture = SDL_CreateTexture(renderer,
                                     SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_TARGET,
                                     scaledWidth, scaledHeight);
    if (!scaleTexture) {
        Platform_ShowError("Error creating scaleTexture: %s", SDL_GetError());
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
}

void Platform_EndFrame(void) {
    static int burstPhase = 0;

    if (!frameStarted) {
        printf("ERROR: Ended frame without starting it!\n");
    }
    frameStarted = 0;
    // convert framebuffer from nes colors to rgb
    Uint32 *rgbFramebuffer;
    int pitch;
    SDL_LockTexture(drawTexture, NULL, (void **)&rgbFramebuffer, &pitch);
    if (ntscEnabled) {
        nes_ntsc_blit(&ntsc,
            framebuffer + (TILE_HEIGHT * FRAMEBUFFER_WIDTH) + TILE_WIDTH,
            FRAMEBUFFER_WIDTH,
            burstPhase,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            (void *)rgbFramebuffer,
            pitch);
        burstPhase ^= 1;
    }
    else {
        // skip past buffer around framebuffer
        int srcOffset = (TILE_HEIGHT * FRAMEBUFFER_WIDTH) + TILE_WIDTH;
        int dstOffset = 0;
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                rgbFramebuffer[dstOffset++] = nesToRGB[framebuffer[srcOffset++]];
            }
            srcOffset += (TILE_WIDTH * 2);
        }
    }

    SDL_UnlockTexture(drawTexture);
    SDL_SetRenderTarget(renderer, scaleTexture);
    // stretch framebuffer horizontally w/ bilinear so the pixel aspect ratio is correct
    SDL_RenderTexture(renderer, drawTexture, NULL, NULL);
    SDL_SetRenderTarget(renderer, NULL);

    // monitor framerate isn't a multiple of 60, so wait in software
    if (!vsync) {
        nanotime_step(&stepData);
    }

    for (int i = 0; i < (vsync ? vsync : 1); i++) {
        SDL_RenderClear(renderer);
        if (fullscreen) {
            SDL_RenderTexture(renderer, scaleTexture, NULL, &fullscreenRect);
        }
        else {
            SDL_RenderTexture(renderer, scaleTexture, NULL, NULL);
        }
        SDL_RenderPresent(renderer);
    }
}

void Platform_ShowError(char *fmt, ...) {
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    va_end(args);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buff, window);
}

Uint8 *Platform_GetFramebuffer(void) {
    return framebuffer;
}

int Platform_GetVideoScale(void) {
    return scale;
}

int Platform_SetVideoScale(int requested) {
    if ((requested > 0) && !fullscreen) {
        scale = requested;
        // resize window
        int windowWidth = ((int)(SCREEN_WIDTH * scale * PIXEL_ASPECT_RATIO));
        int windowHeight = SCREEN_HEIGHT * scale;
        SDL_SetWindowSize(window, windowWidth, windowHeight);
        SDL_SetWindowPosition(window,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED);
        // resize scale framebuffer to fit window
        SDL_DestroyTexture(scaleTexture);
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
        const SDL_DisplayMode *displayMode = SDL_GetDesktopDisplayMode(SDL_GetDisplayForWindow(window));
        if (!displayMode) {
            Platform_ShowError("Error getting display mode: %s", SDL_GetError());
            return 0;
        }
        // make window fullscreen at native res
        SDL_SetWindowSize(window, displayMode->w, displayMode->h);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        SDL_HideCursor();
        // resize scale framebuffer to fit screen
        SDL_DestroyTexture(scaleTexture);
        int fullscreenScale = displayMode->h / SCREEN_HEIGHT;
        int scaledWidth = SCREEN_WIDTH * fullscreenScale;
        int scaledHeight = SCREEN_HEIGHT * fullscreenScale;
        scaleTexture = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_TARGET,
                                         scaledWidth, scaledHeight);
        // set up destination area
        fullscreenRect.w = scaledWidth * PIXEL_ASPECT_RATIO;
        fullscreenRect.h = (float)scaledHeight;
        fullscreenRect.x = (float)((displayMode->w / 2) - (fullscreenRect.w / 2));
        fullscreenRect.y = (float)((displayMode->h / 2) - (fullscreenRect.h / 2));
    }
    else {
        fullscreen = 0;
        SDL_ShowCursor();
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

int Platform_SetNTSC(int requested) {
    if (requested != ntscEnabled) {
        ntscEnabled = requested;
        SDL_DestroyTexture(drawTexture);
        int drawWidth;
        if (ntscEnabled) {
            drawWidth = NES_NTSC_OUT_WIDTH(SCREEN_WIDTH);
        }
        else {
            drawWidth = SCREEN_WIDTH;
        }
        drawTexture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            drawWidth, SCREEN_HEIGHT);
        if (!drawTexture) {
            Platform_ShowError("Error recreating drawTexture: %s", SDL_GetError());
            abort();
            return requested;
        }
        SDL_SetTextureScaleMode(drawTexture, SDL_SCALEMODE_NEAREST);
        DB_Set("ntsc", &ntscEnabled, 1);
        DB_Save();
    }
    return ntscEnabled;
}

int Platform_GetNTSC(void) {
    return ntscEnabled;
}

static int Platform_InitAudio(void) {
    SDL_AudioSpec spec = { 0 };
    spec.freq = 44100;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 1;
    audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &spec, NULL, NULL);
    if (!audioStream) {
        Platform_ShowError("Error creating audioStream: %s", SDL_GetError());
        return 0;
    }
    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));
    return 1;
}

static void Platform_DestroyAudio(void) {
    SDL_DestroyAudioStream(audioStream);
}

void Platform_QueueSamples(Sint16 *samples, int count) {
    SDL_PutAudioStreamData(audioStream, (void *)samples, count * sizeof(Sint16));
}

int Platform_GetQueuedSamples(void) {
    return (int)SDL_GetAudioStreamQueued(audioStream) / sizeof(Sint16);
}

static SDL_Gamepad *Platform_FindGamepad(void) {
    int numGamepads;
    SDL_JoystickID *gamepadIDs = SDL_GetGamepads(&numGamepads);
    if (gamepadIDs && numGamepads) {
        SDL_Gamepad *newGamepad = SDL_OpenGamepad(gamepadIDs[0]);
        SDL_free(gamepadIDs);
        return newGamepad;
    }
    return NULL;
}

int Platform_Init(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD) < 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 0;
    }
    DBEntry *entry = DB_Find("scale");
    if (entry) { scale = entry->data[0]; }
    entry = DB_Find("ntsc");
    if (entry) { ntscEnabled = entry->data[0]; }
    if (!Platform_InitVideo()) { return 0; }
    if (!Platform_InitAudio()) { return 0; }
    gamepad = Platform_FindGamepad();
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
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            // alt+enter = toggle fullscreen
            if ((event.type == SDL_EVENT_KEY_DOWN) &&
                (event.key.keysym.scancode == SDL_SCANCODE_RETURN) &&
                (event.key.keysym.mod & SDL_KMOD_ALT)) {
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
                Input_SetState(button, event.type == SDL_EVENT_KEY_DOWN);
            }
            break;

        // controller events
        case SDL_EVENT_GAMEPAD_ADDED:
            gamepad = SDL_OpenGamepad(event.gdevice.which);
            break;

        case SDL_EVENT_GAMEPAD_REMOVED:
            gamepad = Platform_FindGamepad();
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            if (event.gbutton.button < ARRAY_LEN(gamepadMap)) {
                // match the button to the enum in input.h
                button = gamepadMap[event.gbutton.button];
                if (button != INPUT_INVALID) {
                    Input_SetState(button, event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
                }
            }
            break;

        // handle quit
        case SDL_EVENT_QUIT:
            Platform_Quit();
            break;

        }
    }
}
