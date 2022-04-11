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
#ifndef	__KERN_TIMER_H__
#define	__KERN_TIMER_H__

/* XXX TODO: make these hw/types.h things? */
typedef uint32_t kern_timer_tick_type_t;
typedef int32_t kern_timer_tick_comp_type_t;

extern	kern_timer_tick_type_t kern_timer_tick_msec;

typedef struct kern_timer_event kern_timer_event_t;

typedef	void kern_timer_event_fn_t(kern_timer_event_t *ev, void *arg1,
	    uintptr_t arg2, uint32_t arg3);

/**
 * struct kern_timer_event - a kernel timer.
 *
 * @fn Function to call when timer fires
 * @node timer wheel node
 * @arg1 arg1 to pass to fn
 * @arg2 arg2 to pass to fn
 * @arg3 arg3 to pass to fn
 * @tick absolute tick value in msec to schedule to run
 * @queued true if queued to the timer list
 * @active true if running the timer callback
 * @rearm true if rearm-ed from inside the callback function
 *
 * if rearm is true then the quick hack is that tick temporarily
 * holds the /relative/ value in msec, and will be converted
 * to absolute during re-arm.
 */
struct kern_timer_event {
	kern_timer_event_fn_t *fn;
	struct list_node node;
	void *arg1;
	uintptr_t arg2;
	uint32_t arg3;
	uint32_t tick;
	bool queued;
	bool active;
	bool rearm;
};

extern	void kern_timer_init(void);
extern	void kern_timer_set_tick_interval(uint32_t msec);
extern	void kern_timer_start(void);
extern	void kern_timer_stop(void);
extern	void kern_timer_tick(void);
extern	void kern_timer_idle(void);
extern	void kern_timer_taskcount(uint32_t tasks);

extern	void kern_timer_event_setup(kern_timer_event_t *event,
	    kern_timer_event_fn_t *fn, void *arg1, uintptr_t arg2,
	    uint32_t arg3);
extern	void kern_timer_event_clean(kern_timer_event_t *event);

/**
 * Add a timer event.
 *
 * For now, only callable by the task or context that owns the timer.
 * I'll look at increasing that scope with suitable locking requirements
 * later.
 *
 * Can't be called from within the timer callback!
 */
extern	bool kern_timer_event_add(kern_timer_event_t *event,
	    uint32_t msec);

/**
 * Delete a timer event.
 *
 * For now, only callable by the task or context that owns the timer.
 * I'll look at increasing that scope with suitable locking requirements
 * later.
 *
 * Can't be called from within the timer callback!
 */
extern	bool kern_timer_event_del(kern_timer_event_t *event);

#endif	/* __KERN_TIMER_H__ */
