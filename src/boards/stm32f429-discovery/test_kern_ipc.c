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
#include "kern/core/error.h"
#include "kern/core/task.h"
#include "kern/core/timer.h"
#include "kern/core/physmem.h"

#include <kern/ipc/port.h>
#include <kern/ipc/msg.h>

#include "core/platform.h"


static struct kern_task test_ipc_kern_task_1;

static void
kern_test_ipc_task_1(void *arg)
{
	struct kern_ipc_port *port = NULL;

	port = kern_ipc_port_create(kern_task_current_id());
	if (port == NULL) {
		console_printf("%s: failed to create task\n", __func__);
		goto done;
	}

	kern_ipc_port_set_active(port);
	if (! kern_ipc_port_add_name(port, "task_1_port")) {
		console_printf("%s: failed to add port name\n", __func__);
		goto done;
	}

	console_printf("%s: ready!\n", __func__);
	while (1) {
		struct kern_ipc_msg *msg;

		msg = kern_port_receive_message(port);
		if (msg == NULL) {
			console_printf("%s: null msg?\n", __func__);
			continue;
		}
		kern_port_set_msg_completed(msg);
	}

done:
	if (port != NULL) {
		kern_ipc_port_delete_name("task_1_port");
		kern_ipc_port_shutdown(port);
		kern_ipc_port_close(port);
		kern_ipc_port_destroy(port);
	}
	kern_task_exit();
}

void
setup_kern_test_ipc_task_1(void)
{
	paddr_t kern_stack;

	/* XXX TODO: check for retval=0 */
	kern_stack = kern_physmem_alloc(512, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);

	kern_task_init(&test_ipc_kern_task_1, kern_test_ipc_task_1,
	    NULL,
	    "kern_ipc_test_1", (stack_addr_t) kern_stack, 512,
	    TASK_FLAGS_DYNAMIC_KSTACK);
	kern_task_start(&test_ipc_kern_task_1);
}

