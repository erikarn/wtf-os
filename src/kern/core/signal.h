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

#ifndef	__KERN_SIGNAL_H__
#define	__KERN_SIGNAL_H__

/*
 * Although the signal stuff is currently just implemented in the kernel
 * task routines, the types and bit fields that are used are defined here.
 */

#include <os/bit.h>

typedef uint32_t kern_task_signal_mask_t;
typedef uint32_t kern_task_signal_set_t;

/*
 * These are the system defined tasks that are designed to be handled
 * by the task itself.  KERN_SIGNAL_TASK_MASK defines these signals.
 *
 * KSLEEP - Each task has a sleep timer for signals, sleep() calls, etc.
 *          This is the signal that is posted when that timer fires.
 *
 * TERMINATE - This signal is designed to signal a task (user, or kernel)
 *          that it is time to tidy up and exit.  Kernel related resources
 *          for tasks will get cleaned up in kern_task_exit().  If a
 *          task doesn't handle this signal then at some point ("when"
 *          is a great question here!) it will be terminated anyway.
 *
 * WAIT_PORT_RXREADY - The task is waiting for a specific port to recive
 *          a frame or a completion frame.  This is designed to be used
 *          by code that wishes to block on a single port for received
 *          events; later on I'll implement some kind of notification
 *          "thing" that ports can push their notifications into so
 *          tasks (user and kernel) can wait for multiple "things" at once.
 */
#define	KERN_SIGNAL_ALL_MASK			0xffffffff
#define	KERN_SIGNAL_TASK_MASK			0x000000ff
#define	KERN_SIGNAL_TASK_KSLEEP			BIT_U32(0)
#define	KERN_SIGNAL_TASK_TERMINATE		BIT_U32(1)
#define	KERN_SIGNAL_TASK_WAIT_PORT_RXREADY	BIT_U32(2)

#endif	/* __KERN_SIGNAL_H__ */
