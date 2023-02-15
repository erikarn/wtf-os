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
#include <kern/libraries/mem/mem.h>
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

void
kern_task_mem_init(struct kern_task *task)
{
	KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_INFO,
	     "%s: task 0x%08x", __func__, task);

	kern_bzero(&task->task_mem, sizeof(task->task_mem));
}

void
kern_task_mem_set(struct kern_task *task, task_mem_id_t id,
    paddr_t start, paddr_size_t size, bool is_dynamic)
{
	task->task_mem.task_mem_addr[id] = start;
	task->task_mem.task_mem_size[id] = size;
	if (is_dynamic) {
		task->task_mem.dynamic_flags |= (1 << id);
	} else {
		task->task_mem.dynamic_flags &= ~(1 << id);
	}
}

paddr_t
kern_task_mem_get_start(struct kern_task *task, task_mem_id_t id)
{
	return task->task_mem.task_mem_addr[id];
}

paddr_size_t
kern_task_mem_get_size(struct kern_task *task, task_mem_id_t id)
{
	return task->task_mem.task_mem_size[id];
}

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
	paddr_t addr;
	int i;

	KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_INFO,
	     "cleaning task 0x%08x", task);

	for (i = 0; i < TASK_MEM_ID_NUM; i++) {
		if (task->task_mem.dynamic_flags & (1 << i)) {
			addr = kern_task_mem_get_start(task, i);
			KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_INFO,
			     "freeing id %d (0x%x)!", addr);
			kern_physmem_free(addr);
		}
	}

	KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_INFO, "finished!");
}

/*
 * Initialise the MPU table for the given task.
 *
 * This initialises the MPU table based on the given memory regions
 * for the kern/user task, and will return an error if we can't
 * allocate them appropriately (eg the alignment/size are not
 * compatible.)
 *
 * Yes, this is hard-coded and very specific to the cortex-M4;
 * chances are it should be migrated into the platform code.
 */
bool
kern_task_mem_setup_mpu(struct kern_task *task)
{
	paddr_t addr;
	paddr_size_t size;

	/* Initial table setup, no active regions */
	platform_mpu_table_init(&task->mpu_phys_table[0]);

	/* Executable region - all of XIP for now */
	platform_mpu_table_set(&task->mpu_phys_table[0],
	    0x08000000, 0x200000,
	    PLATFORM_PROT_TYPE_EXEC_RO);

	/* User stack */
	addr = kern_task_mem_get_start(task, TASK_MEM_ID_USER_STACK);
	size = kern_task_mem_get_size(task, TASK_MEM_ID_USER_STACK);
	platform_mpu_table_set(&task->mpu_phys_table[1],
	    addr, size, PLATFORM_PROT_TYPE_NOEXEC_RW);

	/* User heap */

	/* task GOT */

	/* task data */

	/* task rodata */

	/* task BSS */

	return (true);
}
