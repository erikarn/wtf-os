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

#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <hw/types.h>

#include <kern/libraries/string/string.h>
#include <kern/libraries/list/list.h>
#include <kern/libraries/container/container.h>

#include <kern/core/exception.h>
#include <kern/core/signal.h>
#include <kern/core/task.h>
#include <kern/core/task_mem.h>
#include <kern/core/malloc.h>
#include <kern/core/logging.h>
#include <kern/core/physmem.h>
#include <kern/console/console.h>

#include <core/platform.h>
#include <core/lock.h>

LOGGING_DEFINE(LOG_TASKMEM, "task_mem", KERN_LOG_LEVEL_INFO);

/**
 * Free the memory allocations for the given task.
 *
 * This checks the pointers and allocation flags.
 * For regions that are set, it will free them appropriately.
 *
 * Note: regions that have been allocated are physical address
 * regions that must be freeable via the physmem API.
 */
void
kern_task_mem_cleanup(struct kern_task *task)
{

	KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_INFO,
	     "cleaning task 0x%08x", task);

	if (task->task_flags & TASK_FLAGS_DYNAMIC_KSTACK) {
		KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_INFO,
		     "freeing stack (0x%x)!", task->kern_stack);
		kern_physmem_free(task->kern_stack);
	}
	if (task->task_flags & TASK_FLAGS_DYNAMIC_USTACK) {
		KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_INFO,
		     "freeing user stack (0x%x)!", task->user_stack);
		kern_physmem_free(task->user_stack);
	}

	/* XXX TODO: optional user heap region */

	/* XXX TODO: optional task executable memory */

	/* XXX TODO: optional GOT */

	/* XXX TODO: optional data */

	/* XXX TODO: optional rodata */

	/* XXX TODO: optional BSS */

	KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_INFO, "finished!");
}
