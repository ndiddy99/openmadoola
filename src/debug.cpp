/* debug.cpp: debug GUI
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

#include <SDL.h>
#include <stdio.h>

extern "C" {
#include "constants.h"
#include "debug.h"
#include "game.h"
#include "graphics.h"
#include "lucia.h"
#include "object.h"
#include "weapon.h"
}
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

static SDL_Window *window;
static SDL_Renderer *renderer;

static void Debug_PresentFrame(void);

int Debug_Init(void) {
    window = SDL_CreateWindow("OpenMadoola", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1450, 960, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error creating window", SDL_GetError(), NULL);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error creating renderer", SDL_GetError(), NULL);
    }
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    return 1;
}

SDL_Window *Debug_GetWindow(void) {
    return window;
}

SDL_Renderer *Debug_GetRenderer(void) {
    return renderer;
}

void Debug_StartFrame(void) {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            ImGui_ImplSDLRenderer2_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            exit(0);
        }
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

static char *Debug_GetObjectText(int num) {
    static char buff[80];

    snprintf(buff, sizeof(buff), "Object %d (Type %02X)", num, objects[num].type);
    return buff;
}

static void Debug_ShowObject(int num) {
    char buff[80];

    ImGui::PushID(num);
    snprintf(buff, sizeof(buff), "Object %d", num);
    ImGui::SeparatorText(buff);
    ImGui::InputScalar("Type", ImGuiDataType_U8, &objects[num].type, nullptr, nullptr, "%X");
    if (ImGui::InputScalar("X Pos", ImGuiDataType_S16, &objects[num].x.v, nullptr, nullptr, "%X") ||
        ImGui::InputScalar("Y Pos", ImGuiDataType_S16, &objects[num].y.v, nullptr, nullptr, "%X")) {
            Object_InitCollision(&objects[num]);
    }
    ImGui::LabelText("Collision", "%X", objects[num].collision);
    ImGui::InputScalar("X Speed", ImGuiDataType_S8, &objects[num].xSpeed, nullptr, nullptr, "%02X");
    ImGui::InputScalar("Y Speed", ImGuiDataType_S8, &objects[num].ySpeed, nullptr, nullptr, "%02X");
    ImGui::InputScalar("Dir", ImGuiDataType_U8, &objects[num].direction, nullptr, nullptr, "%u");
    ImGui::InputScalar("HP", ImGuiDataType_S16, &objects[num].hp, nullptr, nullptr, "%d");
    ImGui::InputScalar("Stunned", ImGuiDataType_U8, &objects[num].stunnedTimer, nullptr, nullptr, "%u");
    ImGui::InputScalar("Timer", ImGuiDataType_U8, &objects[num].timer, nullptr, nullptr, "%02X");
    ImGui::PopID();
}

static void Debug_ShowObjectsWindow(bool *display) {
    static bool showAllActive = true;
    static bool listInactive = false;
    static int selectedObject = 0;

    ImGui::Begin("Objects", display);
    ImGui::Checkbox("Show all active", &showAllActive);
    // display all active ojects
    if (showAllActive) {
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (objects[i].type != OBJ_NONE) {
                Debug_ShowObject(i);
            }
        }
    }
    // display a list of objects that the user can pick from
    else {
        ImGui::Checkbox("List inactive", &listInactive);
        if (ImGui::BeginCombo("List", Debug_GetObjectText(selectedObject))) {
            for (int i = 0; i < MAX_OBJECTS; i++) {
                if (listInactive || (objects[i].type != OBJ_NONE)) {
                    bool selected = (selectedObject == i);
                    if (ImGui::Selectable(Debug_GetObjectText(i), &selected)) {
                        selectedObject = i;
                    }

                    if (selected) { ImGui::SetItemDefaultFocus(); }
                }
            }
            ImGui::EndCombo();
        }
        Debug_ShowObject(selectedObject);
    }

    ImGui::End();
}

void Debug_ShowFlagsWindow(bool *display) {
    char buff[80];

    ImGui::Begin("Flags", display);
    ImGui::SeparatorText("Cheats");
    static bool infiniteHealth = false;
    static bool infiniteMagic = false;
    static bool noNyuru = false;
    static Uint8 maxEnemies = MAX_OBJECTS - 9;
    ImGui::Checkbox("Infinite health", &infiniteHealth);
    if (infiniteHealth) { health = 9990; }
    ImGui::Checkbox("Infinite magic", &infiniteMagic);
    if (infiniteMagic) { magic = 9990; }
    if (ImGui::InputScalar("Max Enemies", ImGuiDataType_U8, &maxEnemies)) { CLAMP(maxEnemies, 0, MAX_OBJECTS - 9); }
    Object_DeleteRange(9 + maxEnemies);
    ImGui::Checkbox("No nyuru", &noNyuru);
    if (noNyuru) {
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if ((objects[i].type == OBJ_NYURU_INIT) || (objects[i].type == OBJ_NYURU)) {
                objects[i].type = OBJ_NONE;
            }
        }
    }

    ImGui::SeparatorText("Lucia");
    ImGui::InputScalar("Health", ImGuiDataType_S16, &health);
    ImGui::InputScalar("Max health", ImGuiDataType_S16, &maxHealth);
    ImGui::InputScalar("Magic", ImGuiDataType_S16, &magic);
    ImGui::InputScalar("Max magic", ImGuiDataType_S16, &maxMagic);
    bool wingBool = (hasWing) ? true : false;
    ImGui::Checkbox("Wing of Madoola", &wingBool);
    if (wingBool) { hasWing = 1; }
    else { hasWing = 0; }

    ImGui::SeparatorText("Item Levels");
    if (ImGui::InputScalar("Boots", ImGuiDataType_U8, &bootsLevel)) { CLAMP(bootsLevel, 0, 3); }
    ImGui::LabelText("Weapon Damage", "%d", weaponDamage);
    if (ImGui::InputScalar("Sword", ImGuiDataType_U8, &weaponLevels[0])) { CLAMP(weaponLevels[0], 0, 3); }
    if (ImGui::InputScalar("Flame Sword", ImGuiDataType_U8, &weaponLevels[1])) { CLAMP(weaponLevels[1], 0, 3); }
    if (ImGui::InputScalar("Magic Bomb", ImGuiDataType_U8, &weaponLevels[2])) { CLAMP(weaponLevels[2], 0, 3); }
    if (ImGui::InputScalar("Bound Ball", ImGuiDataType_U8, &weaponLevels[3])) { CLAMP(weaponLevels[3], 0, 3); }
    if (ImGui::InputScalar("Shield Ball", ImGuiDataType_U8, &weaponLevels[4])) { CLAMP(weaponLevels[4], 0, 3); }
    if (ImGui::InputScalar("Smasher", ImGuiDataType_U8, &weaponLevels[5])) { CLAMP(weaponLevels[5], 0, 3); }
    if (ImGui::InputScalar("Flash", ImGuiDataType_U8, &weaponLevels[6])) { CLAMP(weaponLevels[6], 0, 3); }
    
    ImGui::SeparatorText("Stages");
    bool orb = !!orbCollected;
    if (ImGui::Checkbox("Orb collected", &orb)) { orbCollected = orb ? 0xff : 0; }
    if (ImGui::InputScalar("Highest stage", ImGuiDataType_U8, &highestReachedStage)) { CLAMP(highestReachedStage, 0, 15); }

    ImGui::SeparatorText("Bosses");
    bool boss = !!bossActive;
    if (ImGui::Checkbox("Boss active", &boss)) { bossActive = boss ? 1 : 0; }
    ImGui::LabelText("Boss Objects", "%d", numBossObjs);
    for (int i = 0; i < 16; i++) {
        snprintf(buff, sizeof(buff), "Boss %d defeated", i);
        boss = !!bossDefeated[i];
        if (ImGui::Checkbox(buff, &boss)) { bossDefeated[i] = boss ? 0xff : 0; }
    }

    ImGui::End();
}

void Debug_EndFrame(SDL_Texture *texture) {
    static bool halted = false;
    static int focusTimer = 2;
    static bool objectsWindow = true;
    static bool flagsWindow = true;
    do {
        ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        // make game window start focused (SetWindowFocus doesn't work correctly on the first frame)
        if (focusTimer) {
            ImGui::SetWindowFocus();
            focusTimer--;
        }
        // only run the game when its window is focused
        if (ImGui::IsWindowFocused()) {
            halted = false;
        }
        else {
            halted = true;
        }
        SDL_Texture *drawTexture = Graphics_Scale(texture);
        ImGui::Image((ImTextureID)(intptr_t)drawTexture, ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));
        ImGui::End();

        if (objectsWindow) {
            Debug_ShowObjectsWindow(&objectsWindow);
        }

        if (flagsWindow) {
            Debug_ShowFlagsWindow(&flagsWindow);
        }

        ImGui::Begin("Frames", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Frame count: %03d", frameCount);
        // this works because clicking the button selects the frames window, so the main window
        // gets unselected and halted gets set to true next frame
        if (ImGui::Button("Step Frame")) {
            halted = false;
        }
        ImGui::End();

        ImGui::ShowDemoWindow();
        ImGui::Render();

        Debug_PresentFrame();

        // if we've halted, start rendering the new frame and then loop back without letting the game
        // logic run
        if (halted) {
            Debug_StartFrame();
        }
    } while (halted);
}

static void Debug_PresentFrame(void) {
    int vsync = Graphics_GetVsync();

    // frame limiting when we can't use vsync
    if (!vsync) {
        // 60 Hz frame is 16.666 ms
        Uint32 delays[] = { 17, 16, 17 };
        static Uint32 counter;

        static Uint32 prevTicks;
        Uint32 nowTicks = SDL_GetTicks();
        Uint32 nextTicks = prevTicks + delays[counter % 3];

        if (nowTicks >= prevTicks + 100) {
            prevTicks = nowTicks;
        }
        else {
            if (nowTicks < nextTicks) {
                SDL_Delay(nextTicks - nowTicks);
                prevTicks += delays[counter % 3];
            }
        }
        counter++;
    }

    for (int i = 0; i < (vsync ? vsync : 1); i++) {
        SDL_SetRenderDrawColor(renderer, 198, 228, 244, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }
}
