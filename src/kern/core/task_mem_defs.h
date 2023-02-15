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
#ifndef	__KERN_TASK_MEM_DEFS_H__
#define	__KERN_TASK_MEM_DEFS_H__


/*
 * This is a somewhat platform specific implementation
 * of task memory.
 *
 * For the cortex m4, we need explicit physical address regions;
 * there's no virtual memory mapping going on.
 *
 * In this design the task struct contains these entries
 * so we can do things like program the MPU with the
 * relevant entries.
 *
 * Some memory regions will be allocated via the physmem
 * API; others will be stack/bss allocated (and still physical
 * addresses!) but they shouldn't be freed via the physmem API.
 * We need to track these during task creation and deletion.
 */

typedef enum {

	/* executable region */
	TASK_MEM_ID_BSS = 0,

	/* kernel stack */
	TASK_MEM_ID_KERN_STACK = 1,

	/* user stack */
	TASK_MEM_ID_USER_STACK = 2,

	/* user heap */
	TASK_MEM_ID_USER_HEAP = 3,

	/* user BSS */
	TASK_MEM_ID_USER_BSS = 4,

	/* user data */
	TASK_MEM_ID_USER_DATA = 5,

	/* user rodata */
	TASK_MEM_ID_USER_RODATA = 6,

	/* user GOT */
	TASK_MEM_ID_USER_GOT = 7,

	TASK_MEM_ID_MAX = 7,
	TASK_MEM_ID_NUM = 8,
} task_mem_id_t;

struct task_mem {
	/* Physical address regions */
	paddr_t task_mem_addr[TASK_MEM_ID_NUM];
	paddr_size_t task_mem_size[TASK_MEM_ID_NUM];
};

#endif	/* __KERN_TASK_MEM_DEFS_H__ */
