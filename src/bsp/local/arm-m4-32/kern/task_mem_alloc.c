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
#include <stdint.h>
#include <stdbool.h>

#include "bsp/local/os/reg.h"

#include "hw/types.h"

#include "kern/console/console.h"
#include "kern/libraries/mem/mem.h"

#include "kern/core/task.h"
#include "kern/core/timer.h"
#include "kern/core/physmem.h"
#include "kern/core/malloc.h"
#include "kern/core/task_mem.h"
#include "kern/user/user_exec.h"

#include <kern/core/task_mem_alloc.h>
#include <kern/core/logging.h>


#include "core/platform.h"
#include "core/lock.h"

LOGGING_EXT(LOG_TASKMEM);

/**
 * Allocate RAM for a new user task.
 *
 * This is platform specific for now because I assume (?) I'm going to
 * need to have some platform specific knowledge about how and where
 * to allocate user task RAM so we can meet things like MPU alignment
 * requirements.
 *
 * This assumes the program header has been parsed out and is in hdr.
 * For now we assume that it's an XIP, so text and rodata are both
 * just going to directly reference the location in memory.
 * Those will be setup by the caller after this function runs.
 *
 * All of the other segments are allocated.
 *
 * If require_mpu is set, then an attempt to allocate memory which will
 * meet the alignment requirements for the MPU will be made.  If it can't
 * be made to fit, this will free memory and return false.
 *
 * Returns true if the memory was allocated, false if it couldn't be.
 *
 * Note: the caller must call kern_task_mem_cleanup() to free any
 * memory that we've allocated before we hit an error.
 *
 * XXX TODO: any of the MPU related stuff.
 */
bool
platform_user_task_mem_allocate(const struct user_exec_program_header *hdr,
    struct user_exec_program_addrs *addrs,
    struct task_mem *tm,
    bool require_mpu)
{
	paddr_t kern_stack;

	kern_bzero(addrs, sizeof(*addrs));

	/*
	 * Allocate kernel stack - doesn't require MPU alignment,
	 * only CPU alignment.
	 */
	kern_stack = kern_physmem_alloc(PLATFORM_DEFAULT_KERN_STACK_SIZE,
	    PLATFORM_DEFAULT_KERN_STACK_ALIGNMENT,
	    KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (kern_stack == 0) {
		KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_CRIT,
		     "failed to allocate %s", "kernel stack");
		goto error;
	}

	/* Commented out reminder what we're just assuming is XIP for now */
#if 0
	/* XIP */
	addrs.text_addr = pak.payload_start + hdr.text_offset;
	addrs.start_addr = pak.payload_start + hdr.start_offset;
	/* XIP */
	addrs.rodata_addr = pak.payload_start + hdr.rodata_offset;
#endif

	/*
	 * Note: for MPU, minimium size here is 32 bytes.
	 * So we round smaller entries up.
	 *
	 * For blank segments (eg if we have no data segment for some reason)
	 * we should either allocate a 32 byte fake MPU segment for it, or
	 * add the extra logic to just not allocate RAM for that segment and
	 * then not validate the MPU validity of it.
	 */

	/* Allocate RAM - not MPU aligned for now */
	addrs->got_addr = kern_physmem_alloc(hdr->got_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (addrs->got_addr == 0) {
		KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_CRIT,
		     "failed to allocate %s", "user GOT");
		goto error;
	}

	addrs->bss_addr = kern_physmem_alloc(hdr->bss_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (addrs->bss_addr == 0) {
		KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_CRIT,
		     "failed to allocate %s", "user bss");
		goto error;
	}

	addrs->data_addr = kern_physmem_alloc(hdr->data_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (addrs->data_addr == 0) {
		KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_CRIT,
		     "failed to allocate %s", "user data");
		goto error;
	}

	addrs->heap_addr = kern_physmem_alloc(hdr->heap_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (addrs->heap_addr == 0) {
		KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_CRIT,
		     "failed to allocate %s", "user heap");
		goto error;
	}

	/*
	 * note - this is a stack, so the aligment (min 8 bytes) is for stack
	 * alignment, not general RAM alignment.  It likely should have
	 * a PLATFORM define too.
	 */
	addrs->stack_addr = kern_physmem_alloc(hdr->stack_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (addrs->stack_addr == 0) {
		KERN_LOG(LOG_TASKMEM, KERN_LOG_LEVEL_CRIT,
		     "failed to allocate %s", "user stack");
		goto error;
	}

	/* XIP */
	/*
	 * XXX TODO: this is /terrible/ hard-coded flash for the stm32 discovery
	 * board I'm using, and instead should look at the hdr or something.
	 * (And I should pass in the whole of flash for text if I'm XIP'ing
	 * and want it to be represented in the MPU.)
	 */
	kern_task_mem_set(tm, TASK_MEM_ID_TEXT, 0x08000000, 0x200000, false);

	kern_task_mem_set(tm, TASK_MEM_ID_USER_GOT, addrs->got_addr, hdr->got_size, true);
	kern_task_mem_set(tm, TASK_MEM_ID_USER_BSS, addrs->bss_addr, hdr->bss_size, true);
	kern_task_mem_set(tm, TASK_MEM_ID_USER_DATA, addrs->data_addr, hdr->data_size, true);

	/* XIP */
	kern_task_mem_set(tm, TASK_MEM_ID_USER_RODATA, addrs->rodata_addr, hdr->rodata_size, false);

	kern_task_mem_set(tm, TASK_MEM_ID_USER_HEAP, addrs->heap_addr, hdr->heap_size, true);
	kern_task_mem_set(tm, TASK_MEM_ID_USER_STACK, addrs->stack_addr, hdr->stack_size, true);
	kern_task_mem_set(tm, TASK_MEM_ID_KERN_STACK, kern_stack, PLATFORM_DEFAULT_KERN_STACK_SIZE, true);

	return (true);
error:
	return (false);
}
