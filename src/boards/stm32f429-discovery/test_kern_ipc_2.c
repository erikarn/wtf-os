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
#include <stdint.h>
#include <stdbool.h>

#include "hw/types.h"

#include "kern/console/console.h"
#include "kern/core/task.h"
#include "kern/core/timer.h"
#include "kern/core/physmem.h"

#include <kern/ipc/port.h>
#include <kern/ipc/msg.h>

#include "core/platform.h"

static struct kern_task test_ipc_kern_task_2;

static void
kern_test_ipc_task_2(void *arg)
{
	struct kern_ipc_port *port = NULL;
	struct kern_ipc_port *rem_port = NULL;
	kern_task_signal_set_t sig;
	bool ret;

	port = kern_ipc_port_create(kern_task_current_id());
	if (port == NULL) {
		console_printf("%s: failed to create task\n", __func__);
		goto done;
	}

	kern_task_set_sigmask(0xffffffff, KERN_SIGNAL_TASK_MASK);

	kern_ipc_port_set_active(port);

	console_printf("%s: ready!\n", __func__);

	while (1) {
		struct kern_ipc_msg *msg = NULL;

		if (rem_port == NULL) {
			rem_port = kern_ipc_port_lookup_name("task_1_port");
			if (rem_port == NULL) {
				console_printf("%s: couldn't find task_1_port yet!\n",
				    __func__);
			}
			goto next;
		}

		/*
		 * Only do the actual send attempt if we have
		 * successfully looked up the remote port.
		 */
		if (rem_port != NULL) {
			msg = kern_ipc_msg_allocate(32);
			if (kern_port_enqueue_msg(port, rem_port, msg) == false) {
				console_printf("%s: couldn't enqueue msg!\n",
				    __func__);
				goto next;
			}
		}
		msg = NULL;

		/*
		 * Ok, we've done a send.  Now we wait for the
		 * completion.
		 */
		msg = kern_port_receive_message(port);
		if (msg != NULL) {
			/* yeah yeah we should actually check this is
			 * our actual completed message and not a sent
			 * message we have to respond to, but I'm not
			 * doing crazy stuff, so we can just free it.
			 */
			console_printf("%s: got completion!\n", __func__);
			kern_ipc_msg_free(msg);
			msg = NULL;
		}

next:
		/* Free anything we have left over here */
		if (msg != NULL) {
			kern_ipc_msg_free(msg);
		}
		ret = kern_task_timer_set(current_task, 1000);
		if (ret == false)
			continue;

		(void) kern_task_wait(KERN_SIGNAL_TASK_KSLEEP, &sig);
	}

done:
	if (port != NULL) {
		kern_ipc_port_delete_name("task_1_port");
		kern_ipc_port_shutdown(port);
		kern_ipc_port_close(port);
		kern_ipc_port_destroy(port);

		/* XXX remove reference to remote port? */
	}
	kern_task_exit();
}

void
setup_kern_test_ipc_task_2(void)
{
	paddr_t kern_stack;

	/* XXX TODO: check for retval=0 */
	kern_stack = kern_physmem_alloc(512, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);

	kern_task_init(&test_ipc_kern_task_2, kern_test_ipc_task_2,
	    NULL,
	    "kern_ipc_test_2", (stack_addr_t) kern_stack, 512,
	    TASK_FLAGS_DYNAMIC_KSTACK);
	kern_task_start(&test_ipc_kern_task_2);
}
