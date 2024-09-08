/* task.c: Task manager
 * Copyright (c) 2024 Nathan Misner
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
#include <assert.h>
#include "constants.h"
#include "libco.h"
#include "joy.h"
#include "platform.h"
#include "task.h"

static cothread_t systemTask;
static cothread_t gameTask;
static void (*nextFunction)(void);
static cothread_t childTask;
static int childTimer;
static int childReturn;
static int childSkippable;
static int childSkipped;

#define TASK_STACK_SIZE (sizeof(void *) * 256 * 1024)
static Uint8 gameStack[TASK_STACK_SIZE];
static Uint8 childStack[TASK_STACK_SIZE];
static int canDerive = 0;

void Task_Init(void (*function)(void)) {
    systemTask = co_active();
    // some libco backends don't support using the provided memory instead of allocating new memory
    if ((gameTask = co_derive(gameStack, TASK_STACK_SIZE, function))) {
        canDerive = 1;
    }
    else {
        gameTask = co_create(TASK_STACK_SIZE, function);
    }
    nextFunction = NULL;
    childTask = NULL;
    childTimer = 0;
    childReturn = 0;
    childSkippable = 0;
    childSkipped = 0;
}

static void Task_SwitchCothreadFunction(cothread_t *cothread, Uint8 *stack, void (*function)(void)) {
    if (canDerive) {
        *cothread = co_derive(stack, TASK_STACK_SIZE, function);
    }
    else {
        if (*cothread) { co_delete(*cothread); }
        *cothread = co_create(TASK_STACK_SIZE, function);
    }
}

void Task_Yield(void) {
    assert(co_active() != systemTask);
    co_switch(systemTask);
}

void Task_Switch(void (*function)(void)) {
    assert(co_active() != systemTask);
    nextFunction = function;
    Task_Yield();
}

void Task_Child(void (*function)(void), int timer, int skippable) {
    assert(co_active() == gameTask);
    Task_SwitchCothreadFunction(&childTask, childStack, function);
    childTimer = timer;
    childSkippable = skippable;
    Task_Yield();
}

void Task_Parent(void) {
    assert(co_active() == childTask);
    childReturn = 1;
    Task_Yield();
}

int Task_WasChildSkipped(void) {
    return childSkipped;
}

static void Task_SwitchToParent(int skipped) {
    assert(co_active() == systemTask);
    childSkipped = skipped;
    if (!canDerive) {
        co_delete(childTask);
    }
    childTask = NULL;
}

void Task_Run(void) {
    assert(co_active() == systemTask);
    if (childTask) {
        co_switch(childTask);
        // if task was skipped
        if (childSkippable && (joyRaw & JOY_START)) {
            Task_SwitchToParent(1);
            return;
        }
        // if child wants to return
        if (childReturn) {
            Task_SwitchToParent(0);
            return;
        }
        if (childTimer) {
            childTimer--;
            // if child timed out, switch to parent
            if (childTimer == 0) {
                Task_SwitchToParent(0);
                return;
            }
        }
        if (nextFunction) {
            Task_SwitchCothreadFunction(&childTask, childStack, nextFunction);
            nextFunction = NULL;
            return;
        }
    }
    else {
        co_switch(gameTask);
        if (nextFunction) {
            Task_SwitchCothreadFunction(&gameTask, gameStack, nextFunction);
            nextFunction = NULL;
            return;
        }
    }
}
