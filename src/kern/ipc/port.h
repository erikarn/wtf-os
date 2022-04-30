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

#ifndef	__KERN_CORE_PORT_H__
#define	__KERN_CORE_PORT_H__

struct kern_ipc_port;

/*
 * struct kern_ipc_port - an IPC port between two kernel/user tasks.
 *
 * This is the basic port abstraction to link to a remote port,
 * contain some useful state for the local port, queue messages
 * to and from the port and notify the owner whenever there
 * are messages available.
 *
 * Notably this doesn't have any concept of unique ids, file
 * descriptors, handles, etc.  This is the kernel side of the
 * IPC setup; the port identifier problem is much more interesting
 * when interfacing to user tasks.  So, I'll either add it
 * when I get this working between kernel tasks, or I'll
 * actually add a separate wrapper with the port identifier
 * space per task.
 */

typedef enum {
	KERN_IPC_PORT_STATE_NONE = 0,
	KERN_IPC_PORT_STATE_IDLE = 1,
	KERN_IPC_PORT_STATE_RUNNING = 2,
	KERN_IPC_PORT_STATE_SHUTDOWN = 3,
	KERN_IPC_PORT_STATE_CLOSED = 4,
} kern_ipc_port_state_t;

struct kern_ipc_port {

	/* Node on an ownership list (eg task) */
	struct list_node owner_node;

	/* Node on notification list */
	struct list_node recv_notif_node;
	struct list_node compl_notif_node;

	/* Port state */
	kern_ipc_port_state_t state;

	/*
	 * List of messages that we own, that we've either
	 * queued or completed.
	 *
	 * This is how we ensure that all pending messages
	 * are cleaned up and aren't left dangling.
	 */
	struct list_head msg_head;

	/*
	 * Until I flesh out multi-peer ports with lists so we
	 * don't just support writing to random destination
	 * ports/tasks, there's a single peer link each way
	 * between two ports.
	 */
	struct kern_ipc_port *peer;

	/*
	 * When sending messages we will push them
	 * directly on the recv queue on the destination peer,
	 * or return an error if the destination peer queue
	 * is full.
	 */
	struct {
		struct list_head list;
		uint8_t num;
		uint8_t max;
	} recv_msg;

	/*
	 * And when we have sent a message and have a reply
	 * queued, we'll push them into this queue so
	 * completions can be handled quickly.
	 */
	struct {
		struct list_head list;
		uint8_t num;
		uint8_t max;
	} compl_msg;
};

extern	void kern_ipc_port_init(void);

#endif	/* __KERN_CORE_PORT_H__ */
