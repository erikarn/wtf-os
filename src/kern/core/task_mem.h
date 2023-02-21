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
#ifndef	__KERN_TASKMEM_H__
#define	__KERN_TASKMEM_H__

#include <kern/libraries/list/list.h>
#include <kern/core/signal.h>
#include <kern/core/timer.h>

#include <kern/core/task_defs.h>

struct kern_task;

extern	void kern_task_mem_init(struct kern_task *task);
extern	void kern_task_mem_set(struct kern_task *task, task_mem_id_t id,
	    paddr_t start, paddr_size_t size, bool is_dynamic);
extern	paddr_t kern_task_mem_get_start(struct kern_task *task,
	     task_mem_id_t id);
extern	paddr_size_t kern_task_mem_get_size(struct kern_task *task,
	     task_mem_id_t id);
extern	void kern_task_mem_cleanup(struct kern_task *task);
extern	bool kern_task_mem_setup_mpu(struct kern_task *task);

extern	void kern_task_mem_transfer(struct task_mem *dst, struct task_mem *src);

#endif	/* __KERN_TASKMEM_H__ */
