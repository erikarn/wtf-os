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

volatile uint32_t count = 0;

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
void
_start(void)
{
//	uint32_t count = 0;
	const char *teststr_1 = "test string 1!\r\n";
	const char *teststr_2 = "test string 2!\r\n";

	while (1) {
#if 0
		/*
		 * Test invalid syscall; for now just for doing diagnostics, it doesn't
		 * error out at all.
		 */
		syscall_test(0x12345678, 0x13579bdf, 0x2468ace0, 0x39647afb);

#endif
		/* CONSOLE_WRITE syscall */
		syscall_test(0x00000001, (uintptr_t) ((count & 1) ? teststr_1 : teststr_2), 16, 0);
		/* TIMER_SLEEP syscall, 1 sec */
		syscall_test(0x00000002, 1000, 0, 0);
		count++;
	}

	/* TASK_EXIT syscall */
	syscall_test(0x00000004, 0, 0, 0);
}
