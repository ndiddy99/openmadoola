/* menu.h: Menu display code
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

#pragma once
#include "constants.h"

typedef enum {
    ITEM_TYPE_NONE,
    ITEM_TYPE_BACK,
    ITEM_TYPE_LINK,
    ITEM_TYPE_LIST,
    ITEM_TYPE_NUM,
} MenuItemType;

typedef struct Menu Menu;

typedef struct MenuItem {
    MenuItemType type;
    char *text;
    int num;
    int step;
    char **options;
    int (*init)(void);
    int (*change)(int);
    void (*link)(void);
} MenuItem;

#define MENU_BACK(textParam) \
    {.type = ITEM_TYPE_BACK, \
    .text =  textParam}

#define MENU_LINK(textParam, linkParam) \
    {.type = ITEM_TYPE_LINK, \
    .text = textParam, \
    .link = linkParam}

#define MENU_LIST(textParam, optionsParam, initParam, changeParam) \
    {.type = ITEM_TYPE_LIST, \
    .text = textParam, \
    .options = optionsParam, \
    .init = initParam, \
    .change = changeParam}

#define MENU_NUM(textParam, initParam, changeParam, stepParam) \
    {.type = ITEM_TYPE_NUM, \
    .text = textParam, \
    .init = initParam, \
    .change = changeParam, \
    .step = stepParam}

/**
 * @brief Displays a menu screen
 * @param menuX display x position
 * @param menuY display y position
 * @param spacing how many lines to put between each row of text
 * @param items pointer to MenuItem array (see menu.h for MenuItem struct initialization macros)
 * @param numItems size of MenuItem array
 * @param draw optional function that gets run each frame
 */
void Menu_Run(Uint16 menuX, Uint16 menuY, int spacing, MenuItem *items, int numItems, void (*draw)(void));
