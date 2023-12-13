/* mainmenu.c: Main menu screen
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

#include "bg.h"
#include "constants.h"
#include "menu.h"
#include "joy.h"
#include "options.h"
#include "platform.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"

static Uint8 palette[] = {
    0x0F, 0x36, 0x26, 0x16,
    0x0F, 0x11, 0x21, 0x31,
    0x0F, 0x12, 0x22, 0x32,
    0x0F, 0x13, 0x23, 0x33,
    0x0F, 0x36, 0x26, 0x16,
    0x0F, 0x11, 0x21, 0x31,
    0x0F, 0x12, 0x22, 0x32,
    0x0F, 0x13, 0x23, 0x33,
};

static int exitMenu = 0;
static int redraw = 0;

static int startCB(int num) {
    exitMenu = 1;
    return 0;
}

static int optionsCB(int num) {
    Options_Run();
    redraw = 1;
    return 0;
}

void MainMenu_Run(void) {
    int cursor = 0;

    Sound_Reset();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetAllPalettes(palette);
    Sprite_SetAllPalettes(palette + 16);

start:
    Menu_Init(11, 10);
    Menu_AddLink("Start Game", 0, startCB);
    Menu_AddLink("Options", 0, optionsCB);

    while (!exitMenu) {
        System_StartFrame();
        Sprite_ClearList();
        BG_Clear();

        Menu_Run(4);

        BG_Draw();
        Sprite_EndFrame();
        System_EndFrame();

        if (redraw) {
            redraw = 0;
            goto start;
        }
    }
    exitMenu = 0;
}