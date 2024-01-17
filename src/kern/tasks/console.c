/*
 * Copyright (C) 2022-2024 Adrian Chadd <adrian@freebsd.org>.
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
#include <kern/core/logging.h>
#include <kern/core/error.h>
#include <kern/console/console.h>

#include <kern/ipc/pipe.h>

#include <core/platform.h>
#include <core/lock.h>

/*
 * Console IO task.
 *
 * For now it's only for console output; will make it for console
 * input later.  This is purely to start stress testing the pipe IPC
 * implementation.
 *
 * This isn't designed for kernel debug logging per se, it's designed
 * for tasks which wish to do async console IO.
 */
static struct kern_task console_task;
static uint8_t kern_console_task_stack[256]
	    __attribute__ ((aligned(PLATFORM_DEFAULT_KERN_STACK_ALIGNMENT)))
	     = { 0 };

LOGGING_DEFINE(LOG_TASK_CONSOLE, "console task", KERN_LOG_LEVEL_INFO);

static kern_ipc_pipe_t console_ipc_pipe = { 0 };

#define	KERN_CONSOLE_TASK_BUF_SIZE	128
#define	KERN_CONSOLE_TASK_MSG_SIZE	32
static char console_ipc_buf[KERN_CONSOLE_TASK_BUF_SIZE];

/*
 * Consume a message.  Return true if it was consumed,
 * false if no message was available (ie, back to waiting we go.)
 */
static bool
kern_console_consume(void)
{
	char msg_buf[KERN_CONSOLE_TASK_MSG_SIZE];
	kern_ipc_msg_t *msg;
	kern_error_t t;

	msg = (void *) msg_buf;

	t = kern_ipc_pipe_dequeue(&console_ipc_pipe, msg);

	/* Too big a message, error and consume */
	if (t == KERN_ERR_TOOBIG) {
		kern_ipc_pipe_consume(&console_ipc_pipe, msg);
		KERN_LOG(LOG_TASK_CONSOLE, KERN_LOG_LEVEL_CRIT,
		    "[console] msg too big: %d bytes", msg->len);
		return true;
	}

	/* XXX TODO: handle id? */

	/* Handle empty/in progress */
	if (t == KERN_ERR_EMPTY || t == KERN_ERR_INPROGRESS) {
		return false;
	}

	/* not ok in general */
	if (t != KERN_ERR_OK) {
		/* XXX TODO: is this ok? */
		return false;
	}

#if 1
	console_printf("CONSOLE: : %d bytes\n", kern_ipc_msg_payload_len(msg));
#endif

#if 0
	/* Write data */
	console_putsn(kern_ipc_msg_payload_buf(msg),
	    kern_ipc_msg_payload_len(msg));
#endif
#if 1
	console_printf("\n");
#endif
	return true;
}

static void
kern_console_task_fn(void)
{
	kern_task_signal_set_t sig;

	KERN_LOG(LOG_TASK_CONSOLE, KERN_LOG_LEVEL_INFO, "[console] started!");

	/* Enable all task signals for now */
	kern_task_set_sigmask(0xffffffff, KERN_SIGNAL_TASK_MASK);

	/* 1 second timer for now */
	kern_task_timer_set(current_task, 1000);

	while (1) {
		/* Consume and write the console data to the console */
		while (kern_console_consume() == true)
			;
		/* Wait for the next message */
		(void) kern_task_wait(KERN_SIGNAL_TASK_PIPE |
		    KERN_SIGNAL_TASK_KSLEEP, &sig);

		if (sig & KERN_SIGNAL_TASK_KSLEEP) {
			/* Timer fired, for debugging/testing */
			KERN_LOG(LOG_TASK_CONSOLE,
			    KERN_LOG_LEVEL_INFO, "[console] timer!");
			kern_task_timer_set(current_task, 1000);
		}
	}

	KERN_LOG(LOG_TASK_CONSOLE,
	    KERN_LOG_LEVEL_NOTICE, "[console] Finishing!");

	/* Shutdown */
	kern_ipc_pipe_shutdown(&console_ipc_pipe);

	/* Noone after here should be writing to the pipe, so .. */

	/* Consume and write the console data to the console */
	while (kern_console_consume() == true)
		;

	/* Close */
	kern_ipc_pipe_close(&console_ipc_pipe);

	kern_task_exit();
}

void
kern_task_console_init(void)
{

	KERN_LOG(LOG_TASK_CONSOLE, KERN_LOG_LEVEL_INFO,
	    "Setup!");

	/* Setup the console IO pipe */
	kern_ipc_pipe_setup(&console_ipc_pipe,
	    console_ipc_buf,
	    KERN_CONSOLE_TASK_BUF_SIZE,
	    KERN_CONSOLE_TASK_MSG_SIZE);

	/* Start the task */
	kern_task_init(&console_task, kern_console_task_fn,
	     NULL, "task_console",
	    (stack_addr_t) kern_console_task_stack,
	     sizeof(kern_console_task_stack),
	    0);
	/* Start the task */
	kern_task_start(&console_task);
}


/*
 * Note: right now there's no way to write INTO the IPC pipe here.
 *
 * Why? I'm still thinking of how to do it given that there's no
 * blocking or signaling when the producer side can't write.
 * All they can do is sleep and try again.
 *
 * It'll be fine for testing out console IO from multiple kernel tasks -
 * userland tasks would need to handle the console write path being full
 * and try again.
 */
