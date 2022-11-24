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

#include "hw/types.h"

#include "kern/console/console.h"
#include "kern/core/task.h"
#include "kern/core/timer.h"
#include "kern/core/physmem.h"
#include "kern/core/malloc.h"

#include "core/platform.h"


/*
 * I'm not yet sure how I return a value here; it's quite possible
 * I need to commit some more naked inline assembly sins to get everything
 * lined up right.
 */
static __attribute__((noinline)) void
syscall_test(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
	asm("svc #0x01");
}

/**
 * Until we can do SVC's, all we can really do here is spin!
 */
static void
kern_test_user_task(void *arg)
{
	uint32_t count = 0;
	const char *teststr_1 = "test string 1!\r\n";
	const char *teststr_2 = "test string 2!\r\n";

	while (1) {
		/*
		 * Test invalid syscall; for now just for doing diagnostics, it doesn't
		 * error out at all.
		 */
		syscall_test(0x12345678, 0x13579bdf, 0x2468ace0, 0x39647afb);

		/* CONSOLE_WRITE syscall */
		syscall_test(0x00000001, (uintptr_t) ((count & 1) ? teststr_1 : teststr_2), 16, 0);

		/* TIMER_SLEEP syscall, 1 sec */
		syscall_test(0x00000002, 1000, 0, 0);
		count++;
	}

	/* TASK_EXIT syscall */
	syscall_test(0x00000004, 0, 0, 0);
}

void
setup_test_userland_task(void)
{
	paddr_t kern_stack, user_stack;
	struct kern_task *test_user_task;

	/* XXX TODO: check for retval=0 */
	kern_stack = kern_physmem_alloc(512, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	user_stack = kern_physmem_alloc(512, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);

	/*
	 * XXX TODO: this is 512 instead of sizeof() because of limits
	 * of the current physmem allocator.
	 */
	test_user_task = kern_malloc(512, 4);

	/* Create a user task with dynamically allocated RAM */
	kern_task_user_init(test_user_task, kern_test_user_task, NULL,
	    "user_task",
	    (stack_addr_t) kern_stack,
	    512,
	    (stack_addr_t) user_stack,
	    512,
	    TASK_FLAGS_DYNAMIC_KSTACK | TASK_FLAGS_DYNAMIC_USTACK | TASK_FLAGS_DYNAMIC_STRUCT);

	/* And start it */
	kern_task_start(test_user_task);
}
