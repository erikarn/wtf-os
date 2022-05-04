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

#ifndef	__KERN_CORE_MSG_H__
#define	__KERN_CORE_MSG_H__

#include <os/bit.h>

struct kern_ipc_port;
struct kern_ipc_msg;

typedef uint16_t kern_ipc_msg_size_t;

typedef enum {
	KERN_IPC_MSG_STATE_NONE = 0,
	KERN_IPC_MSG_STATE_QUEUED = 1,
	KERN_IPC_MSG_STATE_RECEIVED = 2,
	KERN_IPC_MSG_STATE_COMPLETED = 3,
	KERN_IPC_MSG_STATE_FINISHED = 4,
} kern_ipc_msg_state_t;

/*
 * A message being sent/received/completed on a port.
 *
 * This is a variable sized struct where the payload is after the
 * message header.  I'll also eventually add some scatter/gather
 * support for pointing to external buffers.  (And for userspace
 * buffers?  That'll be fun too.)
 */
struct kern_ipc_msg {
	/* Source/destination port (for replies) */
	struct kern_ipc_port *src_port;
	struct kern_ipc_port *dst_port;

	kern_ipc_msg_state_t state;

	/* Entry on the queue - rx or completion */
	struct list_node msg_node;

	/* Entry on the owner, for clean-up */
	struct list_node owner_node;

	/* Some type identifier? What about flags? */

	/* Length of the payload part after this header */
	kern_ipc_msg_size_t msg_size;
};

extern	struct kern_ipc_msg * kern_ipc_msg_allocate(uint32_t msg_size);
extern	void kern_ipc_msg_free(struct kern_ipc_msg *msg);

#endif	/* __KERN_CORE_MSG_H__ */
