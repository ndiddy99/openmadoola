/* system.c: Engine management code
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

#include "db.h"
#include "joy.h"
#include "platform.h"
#include "rom.h"
#include "save.h"
#include "sound.h"
#include "system.h"

int System_Init(void) {
    // load assets
    if (!Rom_LoadPrg("madoola.nes", 16, 32768))    { return 0; }
    if (!Rom_LoadChr("madoola.nes", 32784, 32768)) { return 0; }
    if (!Rom_LoadChr("font.bin", 0, 4096))         { return 0; }
    DB_Init();

    // initialize platform code
    if (!Platform_Init()) { return 0; }

    // initialize engine components
    if (!Graphics_Init()) { return 0; }
    if (!Sound_Init()) { return 0; }
    Save_Init();
    Joy_Init();
    Rom_GetMapData(&mapData);
    return 1;
}

void System_StartFrame(void) {
    Platform_StartFrame();
    Graphics_StartFrame();
    Joy_Update();
}

void System_EndFrame(void) {
    Sound_Run();
    Platform_EndFrame();
}
