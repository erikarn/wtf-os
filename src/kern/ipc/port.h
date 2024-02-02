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

#include <kern/core/task_defs.h>

struct kern_ipc_port;
struct kern_ipc_msg;

#define	KERN_IPC_PORT_NAME_LEN		16

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

typedef enum {
	/* port isn't malloc'ed, so don't free() */
	KERN_IPC_PORT_FLAGS_STATIC_MEM = 0x00000001,

	/* port is a service, can handle >1 connects */
	KERN_IPC_PORT_FLAGS_SERVICE = 0x00000002,
} kern_ipc_port_flags_t;

struct kern_ipc_port {
	/*
	 * For named ports... for now.  It's a waste of memory
	 * having it here for non-named ports, but I want to at
	 * least bootstrap this stuff.
	 */
	char port_name[KERN_IPC_PORT_NAME_LEN];
	struct list_node name_node;

	/*
	 * XXX TODO: for now I'm going to cheat and just put ports
	 * as being owned directly by tasks.  I'd like to add an
	 * ownership list abstraction, but not today.
	 */
	kern_task_id_t owner_task;

	/* Node on an ownership list (eg task) */
	struct list_node owner_node;

	/*
	 * Peer port.  This is for a two way unicast data port
	 * exchange.  For a service port that handles incoming
	 * requests then this will be NULL, as it has multiple
	 * peers.
	 *
	 * TODO: would be good to be able to just have a single
	 * list of peers on both sides, and unicast peers just
	 * have a single entry on each others list.
	 */
	struct kern_ipc_port *peer;

	/*
	 * Service list.  This is for a service port serving
	 * multiple clients, handling incoming requests / replies,
	 * and/or sending multicast replies.
	 *
	 * Eventually it may also support sending to specific
	 * ports, but it'll require a specific port label.
	 */
	struct {
		struct list_head head;
		struct list_node node;
	} service_list;

	/* Port state */
	kern_ipc_port_state_t state;

	/* Refcount */
	uint8_t refcount;
};

extern	void kern_ipc_port_init(void);

extern	kern_error_t kern_ipc_port_add_name(struct kern_ipc_port *port,
	    const char *name);
extern	struct kern_ipc_port * kern_ipc_port_lookup_name(const char *name);
extern	bool kern_ipc_port_delete_name(const char *name);

extern	kern_error_t kern_ipc_port_get_reference(struct kern_ipc_port *port);
extern	void kern_ipc_port_free_reference(struct kern_ipc_port *port);

extern	struct kern_ipc_port * kern_ipc_port_create();
extern	void kern_ipc_port_destroy(struct kern_ipc_port *);

extern	bool kern_ipc_port_set_active(struct kern_ipc_port *port);
extern	void kern_ipc_port_shutdown(struct kern_ipc_port *port);
extern	bool kern_ipc_port_close(struct kern_ipc_port *port);

extern	kern_error_t kern_ipc_port_connect(struct kern_ipc_port *lcl, struct kern_ipc_port *rem);
extern	kern_error_t kern_ipc_port_disconnect(struct kern_ipc_port *lcl, struct kern_ipc_port *rem);

#endif	/* __KERN_CORE_PORT_H__ */
