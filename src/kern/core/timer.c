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

typedef uint32_t kern_timer_tick_type_t;
typedef int32_t kern_timer_tick_comp_type_t;

kern_timer_tick_type_t kern_timer_tick_msec = 0;

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
	struct list_node *n;
	kern_timer_event_t *e;

	list_head_init(&list);

	/* XXX TODO: do we REALLY need a spinlock here? */
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
			e->added = false;
			list_add_tail(&list, n);
		} else {
			break;
		}
	}
	platform_spinlock_unlock(&timer_lock);

	/* Outside of the timer lock - go run the events */
	for (n = list.head; n != NULL; n = n->next) {
		e = container_of(n, kern_timer_event_t, node);
		list_delete(&list, n);
		e->fn(e, e->arg1, e->arg2, e->arg3);
	}
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
	event->added = false;
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

	platform_spinlock_lock(&timer_lock);

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

	list_add_before(&timer_list, n, &event->node);

done:
	event->tick = abs_msec;
	event->added = true;

	/*
	 * If we need to start the timer, start the timer.
	 */
	if (kern_timer_running == false) {
		kern_timer_start_locked();
	}

	platform_spinlock_unlock(&timer_lock);
	return (true);
}

/**
 * Delete the given timer event.
 *
 * An event can only be deleted if it's pending.
 * If it's active or about to be active (ie, it's deleted
 * from the timer list and either not present or about
 * to be run) then it can't be canceled/deleted here,
 * and 'false' will be returned.
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
	/*
	 * We only delete events that are in the list.
	 * If they're not in the list (eg not added, or
	 * they're running) then we return false;
	 */
	if (event->added == true) {
		list_delete(&timer_list, &event->node);
		event->added = false;
		ret = true;
	}
	platform_spinlock_unlock(&timer_lock);

	return (ret);
}
