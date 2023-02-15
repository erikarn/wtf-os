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
#ifndef	__KERN_TASK_H__
#define	__KERN_TASK_H__

#include <kern/libraries/list/list.h>
#include <kern/core/signal.h>
#include <kern/core/timer.h>

#include <kern/core/task_defs.h>
#include <kern/core/task_mem_defs.h>

/*
 * This defines a single task.
 */

#define	KERN_TASK_NAME_SZ		16

/* task struct itself was allocated via kern_malloc() */
#define	TASK_FLAGS_DYNAMIC_STRUCT		BIT_U32(0)

/* kernel stack was allocated via physmem */
#define	TASK_FLAGS_DYNAMIC_KSTACK		BIT_U32(1)

/* user stack was allocated via physmem */
#define	TASK_FLAGS_DYNAMIC_USTACK		BIT_U32(2)

/* enable the MPU for a userland task */
#define	TASK_FLAGS_ENABLE_MPU			BIT_U32(3)

/*
 * The top three entries are very /specifically/ ordered for
 * the assembly routines for task switching and syscalls.
 * They make very specific assumptions about the locations of
 * these through the 'current_task' pointer.
 *
 * stack_top is the top of the stack for the /task/ stack.
 * In kernel tasks that's the kernel stack.  For userland
 * tasks that's the userland stack.
 *
 * For userland tasks, they also have a kernel stack.
 * The top of /that/ stack is kern_stack_top.
 *
 * When we're doing syscalls, we need to store the original
 * return address somewhere before we trampoline into the
 * kernel syscall side.
 */

struct kern_task {
	/* Fields used by the assembly routines */
	volatile stack_addr_t stack_top;
	volatile stack_addr_t kern_stack_top;
	volatile stack_addr_t syscall_return_address;

	uint32_t task_flags;

	/* mpu table, yeah hard coded ew */
	platform_mpu_phys_entry_t mpu_phys_table[PLATFORM_MPU_PHYS_ENTRY_COUNT];

	/* Rest of this isn't used by the assembly routines */
	char task_name[KERN_TASK_NAME_SZ];

	struct task_mem task_mem;

	bool is_user_task;

	void *kern_entry_point;

	struct list_node task_list_node;
	struct list_node task_active_node;

	bool is_on_active_list;
	bool is_on_dying_list;

	kern_task_state_t cur_state;

	/*
	 * Refcount for things that have a reference to this
	 * task.  Tasks can't be deleted until the last
	 * reference is gone.
	 */
	uint16_t refcount;

	/* Current priority.  255 is the highest priority. */
	uint8_t priority;
	uint8_t pad0;

	/* Timer for sleeping */
	struct kern_timer_event sleep_ev;

	/*
	 * Current signal set and mask.
	 *
	 * sig_signals is the set of signals that have been set.
	 * Signals need to be ACKed and cleared or they will allow
	 * a follow-up kern_signal_wait() to return immediately.
	 */
	volatile kern_task_signal_set_t sig_set;
	volatile kern_task_signal_mask_t sig_mask;
};

/**
 * The current task, used by the scheduler, tasks themselves and
 * the context switch routine.
 */
extern	struct kern_task * current_task;

/**
 * Create a kernel / user task.
 *
 * The kern/user stack must either be statically allocated or allocated
 * via kern_physmem_alloc() so it can be appropriately freed.
 */
extern	void kern_task_init(struct kern_task *task, void *entry_point,
	    const char *name, stack_addr_t kern_stack, int kern_stack_size,
	    uint32_t task_flags);
extern	void kern_task_user_init(struct kern_task *task, void *entry_point,
	    void *arg, const char *name,
	    stack_addr_t kern_stack, int kern_stack_size,
	    stack_addr_t user_stack, int user_stack_size,
	    uint32_t task_flags);

extern	void kern_task_setup(void);
extern	void kern_task_start(struct kern_task *task);

/**
 * Select a new task to run.  This is called from the platform specific
 * task switching code.  It'll change current_task if required.
 */
extern	void kern_task_select(void);

/**
 * Lookup a task; if it's found return it with the refcount incremented.
 */
extern	struct kern_task * kern_task_lookup(kern_task_id_t task_id);

/**
 * Turn a task struct into a task id.
 */
extern	kern_task_id_t kern_task_to_id(struct kern_task *task);

/**
 * Get the ID of the current task.
 */
extern	kern_task_id_t kern_task_current_id(void);

/**
 * Task reference increment/decrement.
 */
extern	void kern_task_refcount_inc(struct kern_task *task);
extern	void kern_task_refcount_dec(struct kern_task *task);

/**
 * Exit a task.  This is only callable from the task itself.
 */
extern	void kern_task_exit(void);

/**
 * Kill the given task id.
 *
 * I'm NOT quite sure how to implement this without leaking memory,
 * locks, etc, if I'm a kernel task.  Similar if some other task is
 * waiting for a signal from this task, IPC from this task, etc.
 *
 * Hm will need to think about this.
 */
extern	void kern_task_kill(kern_task_id_t task_id);

/**
 * Set a timer for a task.  Only really callable from the task itself.
 *
 * @retval true if ok, false if couldn't set the timer.
 */
extern	bool kern_task_timer_set(struct kern_task *task, uint32_t msec);

/**
 * Wait for a signal.  Only callable from the task itself.
 *
 * @retval 0 if a signal was set (whether we waited or not).
 * @retval -1 if error.
 */
extern	int kern_task_wait(kern_task_signal_mask_t sig_mask,
	    kern_task_signal_set_t *sig_set);

/**
 * Signal the given task.
 *
 * Multiple signals can be sent at once.
 */
extern	int kern_task_signal(kern_task_id_t task_id,
	    kern_task_signal_set_t signals);

/**
 * Set the state of the current task.
 *
 * Must be called in a critical section.
 */
extern	void kern_task_set_state_locked(kern_task_state_t new_state);

/**
 * Set the state of the given task.
 *
 * Must be called in a critical section.
 */
extern	void kern_task_set_task_state_locked(kern_task_id_t task_id,
	    kern_task_state_t new_state);

extern	void kern_task_set_sigmask(kern_task_signal_mask_t and_sig_mask,
	    kern_task_signal_mask_t or_mask);
extern	kern_task_signal_mask_t kern_task_get_sigmask(void);

extern	void kern_task_tick(void);
extern	void kern_task_ready(void);

#endif	/* __KERN_TASK_H__ */
