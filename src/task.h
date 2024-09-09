/* task.h: Task manager
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
#pragma once

/**
 * @brief Sets up the task subsystem. Should be run before any other Task functions.
 * @param function First function to assign a task to.
 */
void Task_Init(void (*function)(void));

/**
 * @brief Switches from the current task to the "system" task. The equivalent of waiting for
 * vblank on the NES.
 */
void Task_Yield(void);

/**
 * @brief Kills the current task and replaces it with the provided function starting next frame.
 * @param function Function to replace the current task with
 */
void Task_Switch(void (*function)(void));

/**
 * @brief Spawns a new child task. This must only be run from the "main game" task. Nested child
 * tasks are not supported. When the child task ends, execution will return to where the game task
 * left off.
 * @param function Function that the task runs
 * @param timer How many frames to run the child task for, 0 = no timer
 * @param skippable 1 = pressing start kills the task and returns to the parent
 */
void Task_Child(void (*function)(void), int timer, int skippable);

/**
 * @brief If a child task calls this, it'll get killed and the game task resumes.
 */
void Task_Parent(void);

/**
 * @returns 1 if the last run child task was skipped, 0 otherwise 
 */
int Task_WasChildSkipped(void);

/**
 * @brief Runs the current active task until it yields. Should be run once per frame.
 */
void Task_Run(void);
