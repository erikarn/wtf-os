/*
 * Copyright (C) 2022 Adrian Chadd <adrian@freebsd.org>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-Licence-Identifier: GPL-3.0-or-later
 */
#ifndef	__KERN_TASK_DEFS_H__
#define	__KERN_TASK_DEFS_H__

#include <kern/libraries/list/list.h>
#include <kern/core/signal.h>
#include <kern/core/timer.h>

typedef enum {
	KERN_TASK_STATE_NONE = 0,
	KERN_TASK_STATE_SLEEPING = 1,
	KERN_TASK_STATE_READY = 2,
	KERN_TASK_STATE_RUNNING = 3,
	KERN_TASK_STATE_DYING = 4,
	KERN_TASK_STATE_IDLE = 5,
} kern_task_state_t;

/**
 * In this task implementation, I'm going to super cheat and
 * just have task IDs match their location in memory.
 * However, things can't make that assumption, and so other
 * tasks (kernel and eventually userland) can only store
 * kern_task_id_t's to reference other tasks.
 */
typedef uintptr_t kern_task_id_t;

/*
 * This defines a single task.
 *
 * For now it's not in a collection; I'll worry about that once
 * I actually /have/ two tasks running (an idle task and some
 * running task.  Ok, maybe two running tasks.  We'll see.)
 */

#define	KERN_TASK_NAME_SZ		16

#endif	/* __KERN_TASK_DEFS_H__ */
