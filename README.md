# OpenMadoola

Free software (GPL v2.0 or later) reimplementation of "The Wing of Madoola"

![OpenMadoola screenshot](screenshot.png)

## Usage

In order to function, OpenMadoola requires a Wing of Madoola ROM image as well as the font.bin and nes.pal data files from this repository. The ROM image can either be a standard NES ROM file named "madoola.nes" or the "sharedassets1.assets" file from [Sunsoft is Back! Retro Game Selection](https://store.steampowered.com/app/2821290/SUNSOFT_is_Back/) on Steam. OpenMadoola reads all the graphics and level data from the ROM image, and will refuse to start if it is not present. If you don't have an original cartridge or don't want to deal with dumping it, I would recommend buying the collection on Steam rather than downloading a ROM off the internet. It's only $13 and has a nice interface with high-quality scans of the manual as well as some concept art. It also shows Sunsoft that there's interest in their legacy IP. To retrieve the "sharedassets1.assets" file, go to your Steam Library, right click the collection, and click "Manage->Browse local files". It will be in the "SUNSOFT is Back! Retro Game Selection_Data" directory.

### UNIX-like platforms (GNU/Linux, Mac OS X, etc)

OpenMadoola will attempt to read the ROM image and data files from these directories:

1. `~/.openmadoola`
2. `/usr/local/share/openmadoola`
3. `/usr/share/openmadoola`
4. Current working directory

It will also write configuration and save data to `~/.openmadoola`.

### Other platforms (Windows, etc)

OpenMadoola will attempt to read the ROM image and data files from the current working directory (typically the directory the executable is in). As a failsafe, it will also try reading the ROM image out of the default Steam install directory (C:\Program Files (x86)\Steam\steamapps\common\SUNSOFT is Back! レトロゲームセレクション) if it doesn't see a ROM image in the current working directory. OpenMadoola will write configuration and save data to the current working directory.

### Default controls

Keyboard | Gamepad (xbox layout) | Equivalent NES button
-------- | --------------------- | ---------------------
W | D-Pad Up | D-Pad Up
S | D-Pad Down | D-Pad Down
A | D-Pad Left | D-Pad Left
D | D-Pad Right | D-Pad Right
J | X | B
K | A | A
Right Shift | Back | Select
Enter | Start | Start

You can change the controls in the options menu. Pressing A+B+Select+Start while in-game will exit to the title screen.

### Game types

Also in the options menu, you'll find a "game type" setting. The "Original" game type tries to simulate the original NES game but with important bugfixes, while the "Plus" game type (the default) includes additional fixes for more minor bugs as well as tweaks that I think improve the game.

#### Changes for both game types
- Game runs without NES hardware limitations (no slowdown, no sprite flicker, no garbage on the edge of the screen)
- Game progress is saved to disk
- Fixed a bug where collecting the Wing of Madoola and then entering a door would make Darutos not spawn
- Fixed a bug where killing Darutos and then dying to one of his fireballs would let you skip fighting him after continuing
- Fixed a bug where sound effects would sometimes get stuck on, glitch out, etc.
- Fixed a bug where pausing and then unpausing on a castle level would cause the noise channel to drop out of the music
- Fixed a bug where the mirroring on the fireball that some enemies throw wasn't initialized

#### Plus-exclusive changes
- Added the opening/ending text from the arcade version of the game
- Added a second emulated sound chip just for music (meaning that channels won't cut out to play sound effects)
- Music doesn't restart when warping within the same stage
- Music doesn't restart when pausing/unpausing the game
- The selected weapon is visible during gameplay, and you can change weapons without pausing
- Darutos gets drawn even when he's only partially visible, making him easier to see
- Fixed a bug where the orb you get at the end of a level wasn't animated correctly
- Fixed a bug where Nipata would get stuck if it spawned in a narrow gap

## Build Instructions

### UNIX-like platforms (GNU/Linux, Mac OS X, etc)

Install GCC or Clang, make, cmake, and the SDL2 development libraries from your package manager.

Generate the build files with
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

Next, compile the executable with this command:
```
cmake --build build --config Release
```

### Windows

Install [Visual Studio](https://visualstudio.microsoft.com/downloads/) and [cmake](https://cmake.org/download/).

Download the latest SDL2 VC development version from [here](https://github.com/libsdl-org/SDL/releases). Unzip the file into libs\\SDL2, making sure that you go down a level so the top-level directories in the folder are docs\\ include\\ libs\\ etc.

Run cmake on the checked out source directory, then open the generated solution in Visual Studio.

Note that I don't check if the project builds with Visual Studio in between releases, so if you check out from master it's possible that the project won't build without changes. If you make a PR with the required changes, that would be appreciated.


## Used Software

### nanotime https://github.com/nightmareci/nanotime
```
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute
this software, either in source code form or as a compiled binary, for any
purpose, commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of
this software dedicate any and all copyright interest in the software to the
public domain. We make this dedication for the benefit of the public at
large and to the detriment of our heirs and successors. We intend this
dedication to be an overt act of relinquishment in perpetuity of all present
and future rights to this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
```

### nes_ntsc http://www.slack.net/~ant/libs/ntsc.html
```
Copyright (C) 2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
```

### Nes_Snd_Emu http://www.slack.net/~ant/libs/
```
Copyright (C) 2003-2005 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
```

### sdl2 http://libsdl.org/
```
Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>
  
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
  
1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required. 
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
```
