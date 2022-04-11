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
#include <kern/core/timer.h>
#include <kern/console/console.h>

#include <core/platform.h>
#include <core/lock.h>

/*
 * This starts at some negative value so it'll roll over soon.
 * That way we can find and fix timer overflow boundary case conditions.
 */
kern_timer_tick_type_t kern_timer_tick_msec = -32768;

static uint32_t kern_timer_msec = 0;

static platform_spinlock_t timer_lock;
static struct list_head timer_list;

static bool kern_timer_running = false;

/**
 * Return true if a is after b
 */
#define	kern_timer_after(a, b) ((kern_timer_tick_comp_type_t)(b - a) < 0)

/**
 * Return true if a is before b
 */
#define	kern_timer_before(a, b) ((kern_timer_tick_comp_type_t)(b - a) > 0)

/**
 * Return true if a is equal to or after b
 */
#define	kern_timer_after_eq(a, b) ((kern_timer_tick_comp_type_t)(b - a) <= 0)

/**
 * Return true if a is equal to or before b
 */

#define	kern_timer_before_eq(a, b) ((kern_timer_tick_comp_type_t)(b - a) >= 0)

/*
 * Note - until we get dynamic memory going in the kernel, the timer
 * struct needs to live in kern_timer.h
 */

void
kern_timer_init(void)
{
	platform_spinlock_init(&timer_lock);
	list_head_init(&timer_list);
	platform_timer_disable();
}

/**
 * Set the tick interval.  This determines the timer
 * resolution and will control how often we'll attempt
 * to context switch.
 */
void
kern_timer_set_tick_interval(uint32_t msec)
{
	kern_timer_msec = msec;
	platform_timer_set_msec(msec);
}

/**
 * Start the timer from the previously stopped value.
 * If one wishes to reset the timer, just call
 * set_tick_interval() before calling start.
 */
static void
kern_timer_start_locked(void)
{
	if (kern_timer_running == false) {
		kern_timer_running = true;
		platform_timer_enable();
	}
}

/**
 * Stop the timer.  This freezes the timer at the
 * current value.
 */
static void
kern_timer_stop_locked(void)
{
	if (kern_timer_running == true) {
		kern_timer_running = false;
		platform_timer_disable();
	}
}

/**
 * Public function - start the timer.
 */
void
kern_timer_start(void)
{
	platform_spinlock_lock(&timer_lock);
	kern_timer_start_locked();
	platform_spinlock_unlock(&timer_lock);
}

/**
 * Public function - stop the timer.
 */
void
kern_timer_stop(void)
{
	platform_spinlock_lock(&timer_lock);
	kern_timer_stop_locked();
	platform_spinlock_unlock(&timer_lock);
}

/**
 * Called by the interrupt handler for the tick routine.
 *
 * This and any functions it calls are being run in an
 * interrupt context, not a task context, and should be
 * super careful!
 */
void
kern_timer_tick(void)
{
	struct list_head list;
	struct list_node *n, *m;
	kern_timer_event_t *e;

	list_head_init(&list);

	platform_spinlock_lock(&timer_lock);
	kern_timer_tick_msec += kern_timer_msec;

	/* Walk the list, look for events to run */
	n = timer_list.head;

	/* Don't yet have a safe iterator, so just keep checking head */
	while (timer_list.head != NULL) {
		n = timer_list.head;
		e = container_of(n, kern_timer_event_t, node);
		if (kern_timer_before_eq(e->tick, kern_timer_tick_msec)) {
			list_delete(&timer_list, n);
			/* We're not on the queue now, but we're active! */
			e->queued = false;
			e->active = true;
			list_add_tail(&list, n);
		} else {
			break;
		}
	}
	platform_spinlock_unlock(&timer_lock);

	/*
	 * Outside of the timer lock - go run the events.
	 *
	 * This is hairy and one of the dirty places I don't like.
	 *
	 * It's also why I explicitly have stated that the timer
	 * add/delete can only be done by the owning task and not
	 * external tasks.
	 *
	 * Now here's why.
	 *
	 * - I want to ensure that events aren't re-added or deleted
	 *   whilst we're running the above and below loops.
	 *   Now, the above is fixed by having queued/active, but
	 *   we still need to transition active from true->false at
	 *   some point.
	 * - If we set it to false /after/ the callback, then the
	 *   timer won't be re-armable by the function itself.
	 *   Eg, all the timer can do is do a wakeup or a signal
	 *   to the owning task, and then said task has to do the
	 *   rest of the gruntwork with the timer.
	 * - If we set it to false /before/ the callback, then
	 *   the timer /will/ be re-armable by the function itself,
	 *   but there's no nice cleanly atomic/locked state machine
	 *   to ensure it's not re-armed before the callback completes.
	 *   (eg if it's pre-empted by something else.)
	 *
	 * So I'm going with a bit of a hybrid, but documented method.
	 * Here goes.
	 *
	 * - The running state will be updated /after/ the timer fires,
	 *   under a lock.
	 * - The function itself can't call timer_del or timer_add.
	 * - But the function itself /can/ call timer_rearm_in_callback,
	 *   which is a function designed to explicitly be called from
	 *   inside the function callback itself to rearm the timer.
	 *   It'll ignore that it's running because we know it is.
	 */

	/*
	 * Ok, walk the list of timers, don't delete them from the
	 * list or update their active state.
	 */
	for (n = list.head; n != NULL; n = n->next) {
		e = container_of(n, kern_timer_event_t, node);
		e->fn(e, e->arg1, e->arg2, e->arg3);
	}

	/*
	 * Now walk the same list, deleting them this time, but
	 * if re-arm is set then we explicitly re-add them with
	 * the given msec value.
	 *
	 * Since we may delete any entry in the list, we need
	 * to walk it safely.. :-)
	 */
	platform_spinlock_lock(&timer_lock);
	for (n = list.head; n != NULL; n = m) {
		/*
		 * For now I'm going to super cheat and not put it
		 * in one for loop construct because it's late.
		 * Once this works then yes, yes I should.
		 */
		m = n->next;

		e = container_of(n, kern_timer_event_t, node);
		e->queued = false;
		e->active = false;
		list_delete(&list, n);

		if (e->rearm == true) {
			console_printf("[timer] TODO: implement re-arm!\n");
			e->rearm = false;
		}
	}
	platform_spinlock_unlock(&timer_lock);
}

/**
 * Called in the idle task/loop when we don't have any work
 * to schedule.
 *
 * If we don't have work to schedule AND we don't have any
 * timer events (and later, if we don't have a need for
 * timekeeping) then we just disable the timer altogether.

 * The task scheduler and any other eventual kernel code
 * that wishes to use time related stuff will just need
 * to call it again to start things up.
 */
void
kern_timer_idle(void)
{
#if 1
	platform_spinlock_lock(&timer_lock);

	if (list_is_empty(&timer_list)) {
		kern_timer_stop_locked();
	}
	platform_spinlock_unlock(&timer_lock);
#endif
}

/**
 * Called in the task context switch code to update
 * how many tasks there are.
 *
 * If there is only one task then we don't need
 * to re-enable the timer; it'll just keep running.
 * (of course if we eventually need timekeeping and
 * someone has requested it enabled, we'll also
 * start it.)
 *
 * But if we have more than one task then we should
 * re-enable the timers.
 *
 * This likely doesn't belong here as it is being
 * given knowledge about the number of tasks running,
 * but it's easy to push back into the task code
 * if needs be.
 */
void
kern_timer_taskcount(uint32_t tasks)
{
	if (tasks > 1) {
		platform_spinlock_lock(&timer_lock);
		kern_timer_start_locked();
		platform_spinlock_unlock(&timer_lock);
	}
}

void
kern_timer_event_setup(kern_timer_event_t *event,
    kern_timer_event_fn_t *fn, void *arg1, uintptr_t arg2,
    uint32_t arg3)
{
	event->fn = fn;
	event->arg1 = arg1;
	event->arg2 = arg2;
	event->arg3 = arg3;
	event->tick = 0;
	event->queued = false;
	event->active = false;
	event->rearm = false;
	list_node_init(&event->node);
}

/**
 * Clean an event that we're not going to use anymore.
 *
 * The event must be first removed from the timer list.
 *
 * This is a no-op here as we're currently not doing
 * any dynamic memory allocation.
 */
void
kern_timer_event_clean(kern_timer_event_t *event)
{
}

/**
 * Add the given event to fire after msec milliseconds.
 *
 * An event can only be added if it is not queued/active.
 * Otherwise it's likely on a list somewhere and we'll
 * be in trouble.
 *
 * @param[event] timer event to add
 * @param[msec] milliseconds after now before firing
 * @retval true if added, false if not added
 */
bool
kern_timer_event_add(kern_timer_event_t *event, uint32_t msec)
{
	struct list_node *n;
	kern_timer_event_t *e;
	uint32_t abs_msec = msec + kern_timer_tick_msec;
	bool ret = true;

	platform_spinlock_lock(&timer_lock);

	if ((event->active == true) || (event->queued == true)) {
		ret = false;
		goto error;
	}

	/* Quick check - is list empty? add it to head, exit */
	if (list_is_empty(&timer_list)) {
		list_add_head(&timer_list, &event->node);
		goto done;
	}

	/* List has at least one node in it */
	for (n = timer_list.head; n != NULL; n = n->next) {
		e = container_of(n, kern_timer_event_t, node);
		if (kern_timer_after_eq(e->tick, abs_msec))
			break;
	}

	if (n == NULL) {
		list_add_tail(&timer_list, &event->node);
	} else {
		list_add_before(&timer_list, n, &event->node);
	}

done:
	event->tick = abs_msec;
	event->queued = true;
	event->active = false;
	event->rearm = false;

	/*
	 * If we need to start the timer, start the timer.
	 */
	if (kern_timer_running == false) {
		kern_timer_start_locked();
	}

error:
	platform_spinlock_unlock(&timer_lock);
	return (ret);
}

/**
 * Delete the given timer event.
 *
 * An event can only be deleted if it's either not on the list,
 * or if it's queued but not running.  If it's running then
 * it's too late to delete; we can't cancel it.
 *
 * @param[in] event Event to delete
 * @retval true if the event was deleted before run;
 *         false if it wasn't on the list or is about to run.
 */
bool
kern_timer_event_del(kern_timer_event_t *event)
{
	bool ret = false;

	platform_spinlock_lock(&timer_lock);

	/* not on the list, not running, we can "delete" */
	if ((event->active == false) && (event->queued == false)) {
		ret = true;
		goto done;
	}

	/* it's on the list, but not active, we can cancel it here */
	if ((event->active == false) && (event->queued == true)) {
		list_delete(&timer_list, &event->node);
		event->queued = false;
		event->active = false;
		ret = true;
		goto done;
	}

	/* Otherwise at this point it isn't cancelable */

done:
	platform_spinlock_unlock(&timer_lock);

	return (ret);
}
