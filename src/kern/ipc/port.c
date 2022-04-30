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
#include <stdbool.h>

#include <hw/types.h>

#include <core/platform.h>
#include <core/lock.h>

#include <kern/libraries/container/container.h>
#include <kern/libraries/list/list.h>

#include <kern/console/console.h>

#include <kern/core/exception.h>

#include <kern/ipc/port.h>
#include <kern/ipc/msg.h>

static platform_spinlock_t kern_port_ipc_spinlock;

void
kern_ipc_port_init(void)
{

	platform_spinlock_init(&kern_port_ipc_spinlock);
}

/**
 * Initialise the given port.
 *
 * It will be initialised with the given parameters, but not
 * connected to any remote port.
 */
bool
kern_ipc_port_setup(struct kern_ipc_port *port)
{
	list_node_init(&port->owner_node);
	list_node_init(&port->recv_notif_node);
	list_node_init(&port->compl_notif_node);
	list_head_init(&port->msg_head);
	port->state = KERN_IPC_PORT_STATE_IDLE;
	port->peer = NULL;

	list_head_init(&port->recv_msg.list);
	port->recv_msg.num = 0;
	port->recv_msg.max = 8;

	list_head_init(&port->compl_msg.list);
	port->compl_msg.num = 0;
	port->compl_msg.max = 8;

	return (true);
}

/**
 * Mark the port as ready/active to send/receive data on.
 */
bool
kern_ipc_port_set_active(struct kern_ipc_port *port)
{

	port->state = KERN_IPC_PORT_STATE_RUNNING;
	return (true);
}

/**
 * Shutdown the given port.
 *
 * This will mark the current port as shutting down, further
 * messages can't be enqueued, but current messages will be
 * handled until the queue is empty.
 */
void
kern_ipc_port_shutdown(struct kern_ipc_port *port)
{
	console_printf("%s: TODO\n");
	port->state = KERN_IPC_PORT_STATE_SHUTDOWN;
	/*
	 * XXX TODO: need to inform the remote peer, if one exists,
	 * that we've shutdown.
	 */
}

/**
 * Close the given port.
 *
 * This marks the port as shutting down if it isn't, purges
 * any pending IPCs in the queue, and then finishes wrapping
 * up state for this port.
 */
bool
kern_ipc_port_close(struct kern_ipc_port *port)
{
	console_printf("%s: TODO\n");
	/*
	 * XXX TODO: First up, if we're not running, then we should go
	 * through the shutdown state?
	 */

	port->state = KERN_IPC_PORT_STATE_CLOSED;
	/*
	 * XXX TODO: need to inform the remote peer, if one exists,
	 * that we've closed, disconnecting from the peer.
	 */
	return (false);
}

/**
 * Link two ports together.
 *
 * This links the given local and remote ports together.
 * It assumes the local port is not linked to anything.
 * (Eventually the remote port may end up being a service
 * port and support multiple clients, but that isn't
 * currently the case.)
 */
bool
kern_ipc_port_link(struct kern_ipc_port *port_lcl,
    struct kern_ipc_port *port_rem)
{
	if (port_lcl->peer != NULL) {
		return (false);
	}

	if (port_rem->peer != NULL) {
		return (false);
	}

	/* XXX TODO: notify? */

	port_lcl->peer = port_rem;
	port_rem->peer = port_lcl;

	return (true);
}

/**
 * Enqueue a message on the given port to be sent to the
 * remote port.
 *
 * This is called with the local port with which to
 * send a message to the remote port.  If there isn't
 * a remote port then an error will be returned.
 * If there's no space on the remote port then an error
 * will be returned.
 *
 * Otherwise, the message will be sent and once
 * it is completed, a response will be queued back
 * on this port informing the sender that it is
 * complete.
 *
 * (Why you ask? Well, that way (a) we can properly
 * implement zero copy stuff in the future, (b) we
 * can actually cancel queued but not handled messages,
 * and (c) there's some basic flow control back to
 * the sender.  That way we can implement blocking
 * in user code by doing stuff like send + wait
 * for response before sending again, rather than
 * constantly sending until we hit an error, then
 * we .. sleep? for what signal response? :-)
 *
 * Maybe at some point it would be worthwhile to add
 * notifications when the remote port is ready to
 * receive data again.
 */
bool
kern_port_enqueue_msg(struct kern_ipc_port *lcl_port,
    struct kern_ipc_msg *msg)
{
	struct kern_ipc_port *rem_port;

	/* Check if we have a peer */
	if (lcl_port->peer == NULL) {
		goto error;
	}

	rem_port = lcl_port->peer;

	/* Check if there's space at the receiver */
	if (rem_port->recv_msg.num >= rem_port->recv_msg.max) {
		goto error;
	}

	/* Track locally */
	list_add_tail(&lcl_port->msg_head, &msg->owner_node);
	msg->src_port = lcl_port;

	/* Enqueue to remote port */
	list_add_tail(&rem_port->recv_msg.list, &msg->msg_node);
	msg->dst_port = rem_port;
	rem_port->recv_msg.num++;
	msg->state = KERN_IPC_MSG_STATE_QUEUED;

	/* XXX TODO: notify receiver port */

	return (true);

error:
	return (false);
}

/**
 * Fetch the current receive message on the port.
 *
 * If there are no messages then NULL is returned.
 */
struct kern_ipc_msg *
kern_port_fetch_receive_msg(struct kern_ipc_port *port)
{
	struct list_node *node;
	struct kern_ipc_msg *msg;

	if (list_is_empty(&port->recv_msg.list)) {
		return (NULL);
	}

	node = port->recv_msg.list.head;

	list_delete(&port->recv_msg.list, node);
	port->recv_msg.num--;

	msg = container_of(node, struct kern_ipc_msg, msg_node);
	msg->state = KERN_IPC_MSG_STATE_RECEIVED;

	return (msg);
}

/**
 * Fetch the current completed message on the port.
 *
 * If there are no messages then NULL is returned.
 *
 * (Can I fold this into the general receive path instead?)
 */
struct kern_ipc_msg *
kern_port_fetch_completed_msg(struct kern_ipc_port *port)
{
	struct list_node *node;
	struct kern_ipc_msg *msg;

	if (list_is_empty(&port->compl_msg.list)) {
		return (NULL);
	}

	node = port->compl_msg.list.head;

	list_delete(&port->compl_msg.list, node);
	port->compl_msg.num--;

	msg = container_of(node, struct kern_ipc_msg, msg_node);
	msg->state = KERN_IPC_MSG_STATE_FINISHED;

	return (msg);
}

/**
 * Mark a message as completed, and throw it back to
 * the sender port.
 *
 * This is where lifecycle management gets tricky.
 *
 * We can't close a port until all of its owned messages
 * are completed/cancelled and freed.  So we assume here
 * that the sender of this message is still around,
 * and we push the message into the completion queue.
 *
 * We don't have any completion information to add here yet
 * but it'd be good to have.
 */
bool
kern_port_set_msg_completed(struct kern_ipc_msg *msg)
{
	struct kern_ipc_port *port;

	/* XXX TODO: ensure that this message isn't on any other list */
	port = msg->src_port;

	list_add_tail(&port->compl_msg.list, &msg->msg_node);
	port->compl_msg.num++;
	msg->state = KERN_IPC_MSG_STATE_COMPLETED;

	/* XXX TODO: wakeup receiver port w/ completion notification */

	return (true);
}
