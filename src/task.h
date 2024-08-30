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
 * @brief Deletes any active tasks and sets the current task.
 * @param init Task initialization function (gets run immediately)
 * @param update Task update function (gets run each frame)
 */
void Task_SetRoot(void (*init)(void), void (*update)(void));

/**
 * @brief Deletes any active tasks and sets the current task.
 * @param init Task initialization function (gets run immediately)
 * @param update Task update function (gets run each frame)
 * @param timer number of frames to run the task before switching to the next task
 */
void Task_SetRootTimed(void (*init)(void), void (*update)(void), int timer);

/**
 * @brief Adds a task to the end of the current task's "next task" queue
 * @param init Task initialization function (gets run when task is switched to)
 * @param update Task update function (gets run each frame)
 */
void Task_AddNext(void (*init)(void), void (*update)(void));

/**
 * @brief Adds a task to the end of the current task's "next task" queue
 * @param init Task initialization function (gets run when task is switched to)
 * @param update Task update function (gets run each frame)
 * @param timer number of frames to run the task before switching to the next task
 */
void Task_AddNextTimed(void (*init)(void), void (*update)(void), int timer);

/**
 * @brief Sets the current task, making the previous task the current task's parent.
 * @param init Task initialization function (gets run immediately)
 * @param update Task update function (gets run each frame)
 */
void Task_AddChild(void (*init)(void), void (*update)(void));

/**
 * @brief Sets the current task, making the previous task the current task's parent.
 * @param init Task initialization function (gets run immediately)
 * @param update Task update function (gets run each frame)
 * @param timer number of frames to run the task before switching to the next task
 */
void Task_AddChildTimed(void (*init)(void), void (*update)(void), int timer);

/**
 * @brief Switches to the next task. The current task's queue gets processed in
 * first-to-last order. If there's no tasks in the queue, control returns to the
 * current task's parent task. If the current task doesn't have a parent task,
 * an error condition occurs.
 */
void Task_Next(void);

/**
 * @brief Runs the current task's update function and modifies timers. Should be
 * run once per frame.
 * @param  
 */
void Task_Run(void);
