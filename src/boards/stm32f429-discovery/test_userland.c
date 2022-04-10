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

#include "core/platform.h"


static struct kern_task test_user_task;
static uint8_t kern_test_user_u_stack[512] __attribute__ ((aligned(8))) = { 0 };
static uint8_t kern_test_user_k_stack[512] __attribute__ ((aligned(8))) = { 0 };

/*
 * I'm not yet sure how I return a value here; it's quite possible
 * I need to commit some more naked inline assembly sins to get everything
 * lined up right.
 */
static __attribute__((noinline)) void
syscall_test(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
	asm("svc #0x69");
}

/**
 * Until we can do SVC's, all we can really do here is spin!
 */
static void
kern_test_user_task(void *arg)
{
	while (1) {
		/*
		 * XXX TODO: looks like all six end up in the right spot
		 * in the syscall handler?
		 */
		syscall_test(0x12345678, 0x13579bdf, 0x2468ace0, 0x39647afb);
	}
}

void
setup_test_userland_task(void)
{
	kern_task_user_start(&test_user_task, kern_test_user_task, NULL,
	    "user_task",
	    (stack_addr_t) kern_test_user_k_stack,
	    sizeof(kern_test_user_k_stack),
	    (stack_addr_t) kern_test_user_u_stack,
	    sizeof(kern_test_user_u_stack));
}

