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
#include <kern/core/timer.h>
#include <kern/core/malloc.h>
#include <kern/core/logging.h>
#include <kern/core/physmem.h>
#include <kern/console/console.h>

#include <core/platform.h>
#include <core/lock.h>

struct kern_task *current_task = NULL;
static platform_spinlock_t kern_task_spinlock;

static struct list_head kern_task_list;
static struct list_head kern_task_active_list;
static struct list_head kern_task_dying_list;
static uint32_t active_task_count = 0;
static uint32_t dying_task_count = 0;

static bool task_switch_ready = false;

LOGGING_DEFINE(LOG_TASK, "task", KERN_LOG_LEVEL_INFO);

/*
 * Idle task
 */
static struct kern_task idle_task;
static uint8_t kern_idle_stack[256] __attribute__ ((aligned(8))) = { 0 };

static struct kern_task test_task;
/* Note: 512 bytes; we need extra for the actual work we do! */
static uint8_t kern_test_stack[512] __attribute__ ((aligned(8))) = { 0 };

static void _kern_task_set_state_locked(struct kern_task *task,
    kern_task_state_t new_state);

static void
kern_task_timer_ev_fn(kern_timer_event_t *ev, void *arg1,
    uintptr_t arg2, uint32_t arg3)
{
	kern_task_id_t t;

	t = kern_task_to_id(arg1);
	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_DEBUG,
	    "[kern] timer fired, task 0x%08x", t);
	kern_task_signal(t, KERN_SIGNAL_TASK_KSLEEP);
}

/**
 * Set the task timer to fire after 'msec' milliseconds.
 *
 * For best behaviour this should be called from the task itself,
 * not from other tasks.
 *
 * @param[in] task kernel task
 * @param[in] msec milliseconds before firing
 * @retval true if set, false if couldn't set (eg is about to run)
 */
bool
kern_task_timer_set(struct kern_task *task, uint32_t msec)
{
	bool ret;

	/*
	 * XXX TODO: another place where it'd be nice if we could
	 * sleep on a non-signal wakeup so we can wait until the
	 * timer actually has fired and we've completed..
	 *
	 * XXX TODO: del/add here isn't atomic; we should extend
	 * the timer API to let us do this cleanly under a single
	 * timer call!
	 */
	ret = kern_timer_event_del(&task->sleep_ev);
	if (ret == false)
		return (false);
	ret = kern_timer_event_add(&task->sleep_ev, msec);
	return (ret);
}

void
kern_task_generic_init(struct kern_task *task, const char *name)
{
	kern_strlcpy(task->task_name, name, KERN_TASK_NAME_SZ);

	list_node_init(&task->task_list_node);
	list_node_init(&task->task_active_node);

	task->is_on_active_list = false;
	task->is_on_dying_list = false;

	/* Timer setup */
	kern_timer_event_setup(&task->sleep_ev, kern_task_timer_ev_fn, task,
	    0, 0);

	/*
	 * Mark this task as idle, we haven't started it running.
	 */
	task->cur_state = KERN_TASK_STATE_IDLE;

	/* Default signal mask */
	task->sig_mask = KERN_SIGNAL_TASK_MASK;
}

void
kern_task_generic_init_finish(struct kern_task *task)
{
	/*
	 * Last, add it to the global list of tasks.
	 */
	platform_spinlock_lock(&kern_task_spinlock);
	list_add_tail(&kern_task_list, &task->task_list_node);
	platform_spinlock_unlock(&kern_task_spinlock);
}

/**
 * Initialise the given task structure for a kernel task.
 *
 * This just sets the generic and the platform side of things up
 * but doesn't add it to the runlist or start the task.
 */
void
kern_task_init(struct kern_task *task, void *entry_point,
    void *arg, const char *name, stack_addr_t kern_stack,
    int kern_stack_size, uint32_t task_flags)
{

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_INFO, "[kern] kern task created 0x%08x", task);

	kern_task_generic_init(task, name);

	kern_task_mem_init(&task->task_mem);

	task->kern_entry_point = entry_point;

	kern_task_mem_set(&task->task_mem, TASK_MEM_ID_TEXT,
	    (paddr_t) entry_point, 0, false);
	kern_task_mem_set(&task->task_mem, TASK_MEM_ID_KERN_STACK,
	    kern_stack, kern_stack_size,
	    !! (task_flags & TASK_FLAGS_DYNAMIC_KSTACK));

	task->is_user_task = 0;

	task->task_flags = task_flags;

	/*
	 * Next we call into the platform code to initialise our
	 * stack with the above parameters so we can context
	 * switch /into/ the task when we're ready to run it.
	 */
	task->stack_top = platform_task_stack_setup(
	    kern_stack + kern_stack_size,
	    entry_point, arg, 0, false);
	/* kern_stack_top isn't used for kernel tasks */
	task->kern_stack_top = 0;

	platform_mpu_table_init(&task->mpu_phys_table[0]);

	kern_task_generic_init_finish(task);
}

/**
 * Initialise the given task structure for a kernel task.
 *
 * This just sets the generic and the platform side of things up
 * but doesn't add it to the runlist or start the task.
 */
void
kern_task_user_init(struct kern_task *task, void *entry_point,
    void *arg, uint32_t r9, const char *name, struct task_mem *task_mem,
    uint32_t task_flags)
{
	paddr_t kern_stack, user_stack;
	paddr_size_t kern_stack_size, user_stack_size;

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_INFO, "[kern] user task created 0x%08x, entry 0x%x, r9=0x%x", task, entry_point, r9);

	kern_task_generic_init(task, name);

	task->kern_entry_point = entry_point;

	/*
	 * Transfer ownership of the memory to us.  At this point if we fail
	 * to set things up it's .. well, our problem to free it.
	 */
	kern_task_mem_transfer(&task->task_mem, task_mem);

#if 0
	kern_task_mem_set(task, TASK_MEM_ID_TEXT, (paddr_t) entry_point, 0, false);
	kern_task_mem_set(task, TASK_MEM_ID_KERN_STACK, kern_stack, kern_stack_size,
	    !! (task_flags & TASK_FLAGS_DYNAMIC_KSTACK));
	kern_task_mem_set(task, TASK_MEM_ID_USER_STACK, user_stack, user_stack_size,
	    !! (task_flags & TASK_FLAGS_DYNAMIC_USTACK));
#endif

	kern_stack = kern_task_mem_get_start(&task->task_mem,
	    TASK_MEM_ID_KERN_STACK);
	kern_stack_size = kern_task_mem_get_size(&task->task_mem,
	    TASK_MEM_ID_KERN_STACK);

	user_stack = kern_task_mem_get_start(&task->task_mem,
	    TASK_MEM_ID_USER_STACK);
	user_stack_size = kern_task_mem_get_size(&task->task_mem,
	    TASK_MEM_ID_USER_STACK);

	task->is_user_task = 1;

	task->task_flags = task_flags;

	/*
	 * Next we call into the platform code to initialise our
	 * stack with the above parameters so we can context
	 * switch /into/ the task when we're ready to run it.
	 *
	 * Here our initial stack is our unprivileged stack.
	 */
	task->stack_top = platform_task_stack_setup(
	    user_stack + user_stack_size,
	    entry_point, arg, r9, true);
	/* And we program in our kernel stack for privileged code */
	task->kern_stack_top = kern_stack + kern_stack_size;

	/* If required, setup the MPU for the user regions */
	(void) kern_task_mem_setup_mpu(task);

	kern_task_generic_init_finish(task);
}

/**
 * Start the given kernel/user task.
 *
 * This makes it ready to be scheduled.
 */
void
kern_task_start(struct kern_task *task)
{
	platform_spinlock_lock(&kern_task_spinlock);
	_kern_task_set_state_locked(task, KERN_TASK_STATE_READY);
	platform_spinlock_unlock(&kern_task_spinlock);
}

/*
 * Select a task to run.
 *
 * This is called by the platform task switching code to select
 * a new task to run.
 */
void
kern_task_select(void)
{
	struct kern_task *task;
	struct list_node *node;

	platform_spinlock_lock(&kern_task_spinlock);
	/*
	 * Special case - handle cleaning up dying tasks.
	 * If we have any dying tasks on the list, then we
	 * schedule the idle task immediately.  It'll then
	 * handle cleanup and then switch again to see
	 * if anything else is needed.
	 */
	if (dying_task_count > 0) {
		task = NULL;
		goto skip;
	}

	/*
	 * Walk the list of active tasks.  For now we just pick the
	 * task at the head of the list and we put it at the tail,
	 * making this purely a round robin scheduler.
	 */
	node = kern_task_active_list.head;
	if (node == NULL) {
		task = NULL;
	} else {
		task = container_of(node, struct kern_task, task_active_node);
	}

	/*
	 * If there isn't a task on the active list then we schedule the
	 * idle task.  If there /is/ a task then we get it ready to run
	 * and move the current task to the end of the active list.
	 */

	/*
	 * Update this task to be ready to run again.
	 *
	 * It's done like this rather than calling set_state because
	 * we don't want to shuffle it around on lists and kick more
	 * re-scheduling.  RUNNING<->READY is fine like this; it's
	 * the sleep transitions we need that set state call for.
	 */
	if (current_task->cur_state == KERN_TASK_STATE_RUNNING)
		current_task->cur_state = KERN_TASK_STATE_READY;

skip:
	/* Update current task */
	if (task != NULL) {
		current_task = task;
		/*
		 * Move to the end of the active list so other tasks
		 * can run.
		 */
		list_delete(&kern_task_active_list, &task->task_active_node);
		list_add_tail(&kern_task_active_list, &task->task_active_node);
	} else {
		current_task = &idle_task;
	}

	/* Mark it as running */
	current_task->cur_state = KERN_TASK_STATE_RUNNING;

	platform_spinlock_unlock(&kern_task_spinlock);

	/*
	 * If we have any other tasks available then we need
	 * to kick start the timer so we can context switch.
	 *
	 * Technically we only need to run the timer if
	 * we have more than one running task.  There's no
	 * need to context switch if we have a /single/ task
	 * running.
	 */
	kern_timer_taskcount(active_task_count);

	/* Program in the MPU table - context switch will re-enable */
	if (current_task->task_flags & TASK_FLAGS_ENABLE_MPU) {
		platform_mpu_table_program(&current_task->mpu_phys_table[0]);
		platform_mpu_enable();
	}
}

/**
 * Clean-up the given task.
 *
 * It has already been removed from the list it is on;
 * so free task related like resources like (upcoming)
 * handles, memory, ports, etc if required.
 *
 * @param[in] task Task to cleanup
 */
static void
kern_task_cleanup(struct kern_task *task)
{
	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_INFO, "cleaning task 0x%08x", task);

	/* Clean up memory regions where required */
	kern_task_mem_cleanup(&task->task_mem);

	/* Free task struct memory if allocated */
	if (task->task_flags & TASK_FLAGS_DYNAMIC_STRUCT) {
		KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_INFO, "freeing struct (0x%x)!", task);
		kern_free(task);
	}
	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_INFO, "finished!");
}

/**
 * Clean up the now reaped tasks - free all the resources
 * that we can't clean up under the kernel lock.
 */
static void
kern_cleanup_dying_tasks(struct list_head *task_dead_list)
{
	struct list_node *node;
	struct kern_task *task;

	while (task_dead_list->head != NULL) {
		node = task_dead_list->head;
		task = container_of(node, struct kern_task,
		    task_list_node);
		KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_NOTICE, "%s: task=0x%x", __func__, task);
		list_delete(task_dead_list, &task->task_list_node);
		kern_task_cleanup(task);
		/* Note: task may be freed/invalid memory at this point */
	}
}

/**
 * Reap dying tasks.
 *
 * This will handle removing dying tasks from the relevant
 * runtime lists and free their memory if required.
 *
 * Called with the kern_task_spinlock held.
 */
static void
kern_reap_dying_tasks_locked(struct list_head *task_dead_list)
{
	struct list_node *node;
	struct kern_task *task;

	while (kern_task_dying_list.head != NULL) {
		node = kern_task_dying_list.head;
		task = container_of(node, struct kern_task,
		    task_active_node);

		list_delete(&kern_task_dying_list, &task->task_active_node);
		list_delete(&kern_task_list, &task->task_list_node);

		/*
		 * Add it to the private list, then do a second pass
		 * outside of this function to do the kern_task_cleanup() call.
		 */
		dying_task_count--;

		list_add_tail(task_dead_list, &task->task_list_node);
		/* Task is invalid here */
	}
}

static void
kern_idle_task_fn(void)
{
	struct list_head task_dead_list;

	list_head_init(&task_dead_list);

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_INFO, "[idle] started!");
	while (1) {
//		console_printf("[idle] entering idle!\n");

		/*
		 * See if we have any idle tasks.
		 * If we do then we finish clean-up here;
		 * then yield immediately so the scheduler
		 * gets another crack at it.
		 *
		 * Yeah, doing this spinlock to check sucks;
		 * it'd be nicer if we had a separate kernel
		 * task to reap dying tasks but that'd require
		 * an extra kernel task + stack.
		 *
		 * Also it would be nice to move them to a private
		 * list and delete them from the global list, and
		 * then do the freeing bit without having the spinlock held.
		 * Maybe do that in the future when we have dynamic
		 * memory allocation working.
		 */
		platform_spinlock_lock(&kern_task_spinlock);
		if (dying_task_count > 0) {
			/* Reap them, ready to clean-up */
			kern_reap_dying_tasks_locked(&task_dead_list);
			platform_spinlock_unlock(&kern_task_spinlock);

			/* Clean-up outside of the lock */
			kern_cleanup_dying_tasks(&task_dead_list);

			platform_kick_context_switch();
			continue;
		}
		platform_spinlock_unlock(&kern_task_spinlock);

		/*
		 * if we get to the idle scheduler loop
		 * then it means nothing else needed to
		 * run.
		 *
		 * Call into the timer to see if it needs
		 * to schedule another tick.  If it doesn't,
		 * it may actually decide to disable the timer
		 * entirely.
		 */
		kern_timer_idle();
		platform_cpu_idle();
	}
}

static void
kern_test_task_fn(void)
{
	kern_task_signal_set_t sig;
	int count = 0;
	bool ret;
	void *alloc = NULL;

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_INFO, "[test] started!");

	/* Enable all task signals for now */
	kern_task_set_sigmask(0xffffffff, KERN_SIGNAL_TASK_MASK);

	while (1) {
		/* Wait every 5 seconds for now */
		ret = kern_task_timer_set(current_task, 5000);
		if (ret == false) {
			KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_CRIT, "[test] failed to add task timer?!");
			continue;
		}
		KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_INFO,
		    "[test] **** (tick=0x%08x), entering wait!",
		    (uint32_t) kern_timer_tick_msec);
		(void) kern_task_wait(KERN_SIGNAL_TASK_KSLEEP, &sig);
		/*
		 * Uncomment this to have the task exit after 10 iterations.
		 */
#if 0
		if (count > 10) {
			break;
		}
#endif
		count++;
		if (alloc != NULL) {
			kern_free(alloc);
		}
		alloc = kern_malloc_nonzero(1024, 8);
	}

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_NOTICE, "[test] Finishing!");
	kern_task_exit();
}

/**
 * Update the given task state and potentially reschedule things
 * to run if needed.
 */
static void
_kern_task_set_state_locked(struct kern_task *task,
    kern_task_state_t new_state)
{
	bool do_ctx = false;

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_DEBUG,
	    "task %s state %d -> %d, active list=%d",
	    task->task_name, task->cur_state, new_state,
	    task->is_on_active_list);

	/* Update state, only do work if the state hasn't changed */
	if (task->cur_state == new_state)
		return;

	/* Update the new state */
	task->cur_state = new_state;

	switch (task->cur_state) {
	case KERN_TASK_STATE_DYING:
		/*
		 * Dying tasks will get moved to the dying list; they won't
		 * be scheduled, and then the task switcher / idle loop
		 * will clean it up.
		 */
		if (task->is_on_active_list) {
			list_delete(&kern_task_active_list,
			    &task->task_active_node);
			task->is_on_active_list = false;
			active_task_count--;
		}

		/*
		 * Handle the case of getting two DYING states;
		 * just be paranoid and ensure we don't double
		 * account it.
		 */
		if (task->is_on_dying_list == false) {
			task->is_on_dying_list = true;
			dying_task_count++;
			list_add_tail(&kern_task_dying_list,
			    &task->task_active_node);
		}
		do_ctx = true;
		break;
	case KERN_TASK_STATE_SLEEPING:
		if (task->is_on_active_list) {
			list_delete(&kern_task_active_list,
			    &task->task_active_node);
			task->is_on_active_list = false;
			active_task_count--;
		}
		do_ctx = true;
		break;
	case KERN_TASK_STATE_READY:
		/*
		 * XXX TODO: we only really need to context switch if
		 * this task has a higher priority than the one currently
		 * running.  For now let's just not and let the scheduler
		 * pick it up.
		 */
		if (task->is_on_active_list == false) {
			list_add_tail(&kern_task_active_list,
			    &task->task_active_node);
			task->is_on_active_list = true;
			active_task_count++;
			do_ctx = true;
		} else {
			do_ctx = false;
		}
		break;
	case KERN_TASK_STATE_RUNNING:
	default:
		KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_CRIT,
		    "[task] taskptr 0x%x unhandled state %d",
		    task, task->cur_state);
		break;
	}

	/*
	 * Remember, this won't DO the context switch until the critical
	 * section / spinlock is finished.  It's just requesting that
	 * we do a context switch soon.
	 */
	if (do_ctx && task_switch_ready) {
		platform_kick_context_switch();
	}
}

void
kern_task_setup(void)
{
	platform_spinlock_init(&kern_task_spinlock);
	list_head_init(&kern_task_list);
	list_head_init(&kern_task_active_list);
	active_task_count = 0;

	/* Idle task will be magically made ready to run */
	kern_task_init(&idle_task, kern_idle_task_fn, NULL, "kidle",
	    (stack_addr_t) kern_idle_stack, sizeof(kern_idle_stack),
	    0);

	/* Test task will be made ready to run as well */
	kern_task_init(&test_task, kern_test_task_fn, NULL, "ktest",
	    (stack_addr_t) kern_test_stack, sizeof(kern_test_stack),
	    0);

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_CRIT,
	    "[setup] idle task=0x%x, test task=0x%x\n",
	    &idle_task, &test_task);

	kern_task_start(&test_task);
}

static struct kern_task *
_kern_task_lookup_locked(kern_task_id_t task_id)
{
	struct kern_task *task;

	task = (struct kern_task *) (uintptr_t) task_id;
	kern_task_refcount_inc(task);
	return (task);
}

struct kern_task *
kern_task_lookup(kern_task_id_t task_id)
{
	struct kern_task *task;

	platform_spinlock_lock(&kern_task_spinlock);
	task = _kern_task_lookup_locked(task_id);
	platform_spinlock_unlock(&kern_task_spinlock);
	return (task);
}

kern_task_id_t
kern_task_to_id(struct kern_task *task)
{

	return (kern_task_id_t) (uintptr_t) task;
}

kern_task_id_t
kern_task_current_id(void)
{
	return (kern_task_to_id(current_task));
}

void
kern_task_refcount_inc(struct kern_task *task)
{
	task->refcount++;
}

void
kern_task_refcount_dec(struct kern_task *task)
{
	task->refcount--;
}

/**
 * Mark the current task as dying.
 *
 * This will move the current task to the dead list; the
 * task switch and idle task will take care of cleaning
 * up said task.
 *
 * This must be called from the task context itself;
 * it must not be called from another context.
 */
void
kern_task_exit(void)
{

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_CRIT,
	    "[task] %s: called, task 0x%x", __func__,
	    current_task);

	/*
	 * XXX TODO: we may need to block here until it's
	 * finished running; remember these calls don't
	 * guarantee success if the timer has actually
	 * just fired.  I'll figure that out once I
	 * have the timer task stuff plumbed in.
	 */
	kern_timer_event_del(&current_task->sleep_ev);
	kern_timer_event_clean(&current_task->sleep_ev);

	platform_spinlock_lock(&kern_task_spinlock);
	kern_task_set_state_locked(KERN_TASK_STATE_DYING);
	platform_spinlock_unlock(&kern_task_spinlock);

	/*
	 * XXX TODO: spin on platform_kick_context_switch()
	 * until we're just cleaned up?
	 */

	return;
}

/**
 * Kill a running kernel task.
 *
 * This can be called from any other kernel task.
 *
 * It's not yet implemented because I haven't yet figured
 * out a sensible flow for how to give a running kernel
 * task a chance to free its resources before it exits.
 *
 * Instead, I may make it so kernel tasks can be signaled
 * to end when they're sleeping in a wait() call and
 * should check for a signal that says "and now you should
 * exit."
 *
 * Anyway, I'll worry about that all of that later.
 */
void
kern_task_kill(kern_task_id_t task_id)
{
	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_CRIT, "[task] %s: TODO!");
	/* XXX TODO */
	return;
}


/**
 * Wait for the given set of signals to be set on the current task.
 *
 * If any of those bits are already set it will return immediately.
 * Otherwise it will mark itself as sleeping, and when it becomes
 * runnable again it will check again.
 *
 * Any signals that were returned in the mask will be cleared.
 *
 * This can not be called in a critical section or with locks held.
 *
 * @param[in] sig_mask mask for signals to check
 * @param[out] sig_set set of signals that were triggered
 */
int
kern_task_wait(kern_task_signal_mask_t sig_mask,
    kern_task_signal_set_t *sig_set)
{
	volatile kern_task_signal_set_t sigs;

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_DEBUG,
	    "[task] wait: task=%x sig_mask=0x%08x",
	    current_task, sig_mask);

	/*
	 * This is a bit messy and I dislike it, but I'll do this
	 * for initial bootstrapping.
	 *
	 * I want to make sure I'm not pre-empted whilst checking
	 * the signal mask in case it gets updated by another task
	 * or interrupt that has pre-empted.
	 *
	 * But, we don't have atomic mutexes here right now and
	 * I can't exactly hold a critical section across yielding.
	 *
	 * So, commit some messy sins for now even though they're not
	 * technically correct, and then we'll fix it all in post.
	 */
	while (1) {
		platform_spinlock_lock(&kern_task_spinlock);

		/* Get the currently set signals */
		sigs = current_task->sig_set;

		if ((sigs & sig_mask & current_task->sig_mask) != 0) {
			_kern_task_set_state_locked(current_task,
			    KERN_TASK_STATE_READY);
			/* Mask out what we've just found */
			current_task->sig_set &= ~(sigs & sig_mask);
			platform_spinlock_unlock(&kern_task_spinlock);
			break;
		}

		/* Still waiting */
		_kern_task_set_state_locked(current_task,
		    KERN_TASK_STATE_SLEEPING);
		platform_spinlock_unlock(&kern_task_spinlock);

		/*
		 * At this point we'll hopefully end up being pre-empted.
		 * I may end up really needing an explicit yield() routine
		 * so I definitely do context switch to something else;
		 * the worst(!) that can happen here is it'll just keep
		 * running through this loop until it ends up context
		 * switching.
		 *
		 * XXX TODO: yeah add a sleep/yield function here.
		 */
	}

	*sig_set = sigs;
	return (0);
}

static int
_kern_task_state_valid_locked(struct kern_task *task)
{
	switch (task->cur_state) {
	case KERN_TASK_STATE_SLEEPING:
	case KERN_TASK_STATE_READY:
	case KERN_TASK_STATE_RUNNING:
		return (1);
	default:
		return (0);
	}
}

static void
_kern_task_wakeup_locked(struct kern_task *task)
{
	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_DEBUG,
	    "task %s waking up, state is %d",
	    task->task_name, task->cur_state);
	if (task->cur_state == KERN_TASK_STATE_SLEEPING) {
		_kern_task_set_state_locked(task, KERN_TASK_STATE_READY);
	}
}

/**
 * Signal the given task.
 *
 * This sets the signals on the given task and if it's ready to
 * be woken up, it'll mark it as READY.  The scheduler will then
 * eventually run the task.
 *
 * @param[in] task_id kernel task ID
 * @param[in] sig_set signal set to set
 * @retval 0 if OK, -1 if error
 */
int
kern_task_signal(kern_task_id_t task_id, kern_task_signal_set_t sig_set)
{
	struct kern_task *task;

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_DEBUG,
	    "[task] task=0x%x sigset=0x%08x",
	    task_id, sig_set);

	platform_spinlock_lock(&kern_task_spinlock);

	/*
	 * This task can be active, running on another core; it may be
	 * sleeping.
	 */
	task = _kern_task_lookup_locked(task_id);

	if (task == NULL) {
		platform_spinlock_unlock(&kern_task_spinlock);
		KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_CRIT,
		    "[task] invalid task id 0x%x", task_id);
		return (-1);
	}

	if (! _kern_task_state_valid_locked(task)) {
		KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_CRIT,
		    "[task] signal: invalid state (%d)",
		    task->cur_state);
		kern_task_refcount_dec(task);
		platform_spinlock_unlock(&kern_task_spinlock);
		return (-1);
	}

	task->sig_set |= sig_set;

	KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_DEBUG,
	    "[task] signal: task id 0x%x sig_set 0x%x task"
	    " sigset 0x%x, task sigmask 0x%x",
	    task_id, sig_set, task->sig_set, task->sig_mask);

	if ((task->sig_set & task->sig_mask) != 0) {
		KERN_LOG(LOG_TASK, KERN_LOG_LEVEL_DEBUG,
		    "[task] signal: task id 0x%x wakeup",
		    task_id);
		/*
		 * Wake up the task if it's not sleeping.
		 *
		 * It may decide that it's not /actually/ ready
		 * to wake up because of the mask set by a call
		 * into kern_task_wait(), but that's not our problem.
		 */
		_kern_task_wakeup_locked(task);
	}
	kern_task_refcount_dec(task);

	platform_spinlock_unlock(&kern_task_spinlock);

	return (0);
}

void
kern_task_set_state_locked(kern_task_state_t new_state)
{
	_kern_task_set_state_locked(current_task, new_state);
}

void
kern_task_set_task_state_locked(kern_task_id_t task_id,
    kern_task_state_t new_state)
{
	struct kern_task *task;

	task = _kern_task_lookup_locked(task_id);

	if (task == NULL)
		return;

	_kern_task_set_state_locked(task, new_state);
	kern_task_refcount_dec(task);
}

/**
 * Update the current task signal mask.
 *
 * The new signal mask for the current task is
 *
 * sig_mask = (sig_mask & and_sig_mask) | or_sig_mask
 *
 * This allows bits to be set AND cleared.
 *
 * @param[in] and_sig_mask Signal mask to AND over the current sigmask
 * @param[in] or_sig_mask Signal mask to then OR over the above
 */
void
kern_task_set_sigmask(kern_task_signal_mask_t and_sig_mask,
    kern_task_signal_mask_t or_sig_mask)
{
	platform_spinlock_lock(&kern_task_spinlock);
	current_task->sig_mask &= and_sig_mask;
	current_task->sig_mask |= or_sig_mask;
	platform_spinlock_unlock(&kern_task_spinlock);
}

kern_task_signal_mask_t
kern_task_get_sigmask(void)
{

	return (current_task->sig_mask);
}

/**
 * Called by the timer to potentially schedule a context switch.
 *
 * This will check to see if we CAN context switch or whether we
 * need to wait until we're fully setup.
 */
void
kern_task_tick(void)
{
	if (task_switch_ready == false) {
		return;
	}

	platform_critical_lock_t s;
	platform_critical_enter(&s);
	platform_kick_context_switch();
	platform_critical_exit(&s);
}

/**
 * Called to actually mark the system as ready to task-switch.
 *
 * This sets the flag that allows the timer and scheduler to
 * start running.
 */
void
kern_task_ready(void)
{
	task_switch_ready = true;
}
