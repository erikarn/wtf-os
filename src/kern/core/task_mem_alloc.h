
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
#ifndef	__USER_TASK_MEM_ALLOC_H__
#define	__USER_TASK_MEM_ALLOC_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <hw/types.h>

#include <kern/core/task_mem.h>
#include <kern/user/user_exec.h>

extern	bool platform_user_task_mem_allocate(const struct user_exec_program_header *hdr,
	    struct user_exec_program_addrs *addrs,
	    struct task_mem *mem,
	    bool require_mpu);


#endif	/* __USER_TASK_MEM_ALLOC_H__ */
