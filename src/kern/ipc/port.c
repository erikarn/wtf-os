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
#include <kern/libraries/string/string.h>

#include <kern/console/console.h>

#include <kern/core/exception.h>
#include <kern/core/task_defs.h>
#include <kern/core/task.h>
#include <kern/core/signal.h>
#include <kern/core/physmem.h>

#include <kern/ipc/port.h>
#include <kern/ipc/msg.h>

static platform_spinlock_t kern_port_ipc_spinlock;
static struct list_head kern_port_name_list;

void
kern_ipc_port_init(void)
{

	platform_spinlock_init(&kern_port_ipc_spinlock);
	list_head_init(&kern_port_name_list);
}

/**
 * Add the given port to the global namespace.
 *
 * This just adds it.  I'm not keeping references right now.
 *
 * @param[in] port port to add
 * @param[in] name name to add
 * @retval true if added, false if not (eg duplicate, already added)
 */
bool
kern_ipc_port_add_name(struct kern_ipc_port *port, const char *name)
{
	kern_strlcpy(port->port_name, name, KERN_IPC_PORT_NAME_LEN);

	platform_spinlock_lock(&kern_port_ipc_spinlock);
	list_add_tail(&kern_port_name_list, &port->name_node);
	platform_spinlock_unlock(&kern_port_ipc_spinlock);

	return (true);
}

static struct kern_ipc_port *
kern_ipc_port_lookup_name_locked(const char *name)
{
	struct list_node *n;
	struct kern_ipc_port *port;

	n = kern_port_name_list.head;
	while (n != NULL) {
		port = container_of(n, struct kern_ipc_port, name_node);
		if (kern_strncmp(name, port->port_name, KERN_IPC_PORT_NAME_LEN) == 0) {
			return (port);
		}
		n = n->next;
	}
	return (false);
}

/**
 * External function to lookup a port by name.
 *
 * This again doesn't do any refcounting; my hope is to get the
 * kernel IPC here to be non-blocking, so if a task or port is closed
 * with in flight messages, they can just be deleted or something.
 *
 * I dunno if that's going to work out in the long run but I really,
 * really would like to get IPC messages flowing already, sheesh!
 *
 * @param[in] name name to lookup
 * @retval port if found, false otherwise
 */
struct kern_ipc_port *
kern_ipc_port_lookup_name(const char *name)
{
	struct kern_ipc_port *port;

	platform_spinlock_lock(&kern_port_ipc_spinlock);
	port = kern_ipc_port_lookup_name_locked(name);
	platform_spinlock_unlock(&kern_port_ipc_spinlock);
	return (port);
}

/**
 * Delete the given global port name.
 *
 * @param[in] name name to delete
 * @retval true if deleted, false otherwise
 */
bool
kern_ipc_port_delete_name(const char *name)
{
	struct kern_ipc_port *port;

	platform_spinlock_lock(&kern_port_ipc_spinlock);
	port = kern_ipc_port_lookup_name_locked(name);
	if (port != NULL) {
		list_delete(&kern_port_name_list, &port->name_node);
		port->port_name[0] = '\0';
	}
	platform_spinlock_unlock(&kern_port_ipc_spinlock);

	return (port != NULL);
}

/**
 * Initialise the given port.
 *
 * It will be initialised with the given parameters, but not
 * connected to any remote port.
 */
bool
kern_ipc_port_setup(struct kern_ipc_port *port, kern_task_id_t task)
{
	list_node_init(&port->owner_node);
	list_node_init(&port->recv_notif_node);
	list_node_init(&port->compl_notif_node);
	list_node_init(&port->name_node);
	list_head_init(&port->msg_head);
	port->state = KERN_IPC_PORT_STATE_IDLE;
	port->port_name[0] = '\0';

	list_head_init(&port->recv_msg.list);
	port->recv_msg.num = 0;
	port->recv_msg.max = 8;

	list_head_init(&port->compl_msg.list);
	port->compl_msg.num = 0;
	port->compl_msg.max = 8;

	port->owner_task = task;

	return (true);
}

struct kern_ipc_port *
kern_ipc_port_create(kern_task_id_t task)
{
	struct kern_ipc_port *port;

	port = (void *) kern_physmem_alloc(sizeof(*port), 4,
	    KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (port == NULL) {
		return (NULL);
	}
	kern_ipc_port_setup(port, task);
	return (port);
}

void
kern_ipc_port_destroy(struct kern_ipc_port *port)
{
	console_printf("%s: TODO!\n", __func__);
}

/**
 * Mark the port as ready/active to send/receive data on.
 */
bool
kern_ipc_port_set_active(struct kern_ipc_port *port)
{
	platform_spinlock_lock(&kern_port_ipc_spinlock);
	port->state = KERN_IPC_PORT_STATE_RUNNING;
	platform_spinlock_unlock(&kern_port_ipc_spinlock);
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
	platform_spinlock_lock(&kern_port_ipc_spinlock);
	port->state = KERN_IPC_PORT_STATE_SHUTDOWN;
	platform_spinlock_unlock(&kern_port_ipc_spinlock);

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

	platform_spinlock_lock(&kern_port_ipc_spinlock);
	port->state = KERN_IPC_PORT_STATE_CLOSED;
	platform_spinlock_unlock(&kern_port_ipc_spinlock);
	/*
	 * XXX TODO: need to inform the remote peer, if one exists,
	 * that we've closed, disconnecting from the peer.
	 */
	return (false);
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
    struct kern_ipc_port *rem_port,
    struct kern_ipc_msg *msg)
{

	platform_spinlock_lock(&kern_port_ipc_spinlock);

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

	/* Notify receiver port */
	/*
	 * XXX TODO: I don't like it being done inside
	 * the kern IPC spinlock; ideally we'd have a way
	 * to ensure that the local/rem port isn't going
	 * to go away whilst we do operations (eg refcounting)
	 * but we don't have that right now.
	 */
	kern_task_signal(rem_port->owner_task,
	    KERN_SIGNAL_TASK_WAIT_PORT_RXREADY);

	platform_spinlock_unlock(&kern_port_ipc_spinlock);

	return (true);

error:
	platform_spinlock_unlock(&kern_port_ipc_spinlock);
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

	platform_spinlock_lock(&kern_port_ipc_spinlock);

	if (list_is_empty(&port->recv_msg.list)) {
		platform_spinlock_unlock(&kern_port_ipc_spinlock);
		return (NULL);
	}

	node = port->recv_msg.list.head;

	list_delete(&port->recv_msg.list, node);
	port->recv_msg.num--;

	platform_spinlock_unlock(&kern_port_ipc_spinlock);

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

	platform_spinlock_lock(&kern_port_ipc_spinlock);

	if (list_is_empty(&port->compl_msg.list)) {
		platform_spinlock_unlock(&kern_port_ipc_spinlock);
		return (NULL);
	}

	node = port->compl_msg.list.head;

	list_delete(&port->compl_msg.list, node);
	port->compl_msg.num--;

	platform_spinlock_unlock(&kern_port_ipc_spinlock);

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

	platform_spinlock_lock(&kern_port_ipc_spinlock);

	/* XXX TODO: ensure that this message isn't on any other list */
	port = msg->src_port;

	list_add_tail(&port->compl_msg.list, &msg->msg_node);
	port->compl_msg.num++;
	msg->state = KERN_IPC_MSG_STATE_COMPLETED;

	/* XXX TODO: wakeup receiver port w/ completion notification */
	platform_spinlock_unlock(&kern_port_ipc_spinlock);

	/* Notify receiver port */
	kern_task_signal(port->owner_task,
	    KERN_SIGNAL_TASK_WAIT_PORT_RXREADY);

	return (true);
}


/**
 * Receive a message on the given port, and block (without timeout for now)
 * until it's completed.
 *
 * This (for now!) receives both receive messages /and/ message completions;
 * the caller should check to see what they're getting.
 */
struct kern_ipc_msg *
kern_port_receive_message(struct kern_ipc_port *lcl_port)
{
	struct kern_ipc_msg *msg = NULL;
	kern_task_signal_mask_t sigmask, sig;

	/*
	 * Save the current signal set for this task, and set RXREADY
	 */
	sigmask = kern_task_get_sigmask() & KERN_SIGNAL_TASK_WAIT_PORT_RXREADY;
	kern_task_set_sigmask(KERN_SIGNAL_ALL_MASK,
	    KERN_SIGNAL_TASK_WAIT_PORT_RXREADY);

	/*
	 * Attempt to dequeue a message.  If we succeed here then great!
	 */
	do {
		msg = kern_port_fetch_completed_msg(lcl_port);
		if (msg != NULL)
			break;
		msg = kern_port_fetch_receive_msg(lcl_port);
		if (msg != NULL)
			break;

		/*
		 * We failed to receive.  Now, this could be because
		 * we don't have a peer or some other non-queue-empty
		 * error.  But, as we don't YET have error codes here,
		 * all I can do is sleep.
		 *
		 * So, let's sleep on that signal until we're woken up.
		 * Later on I'll go implement kernel wide error codes
		 * and we can start sprinkling them here.
		 */
		kern_task_wait(KERN_SIGNAL_TASK_WAIT_PORT_RXREADY, &sig);

		/*
		 * If we eventually do timer related stuff then we should
		 * handle the timer signal here.
		 */

	} while (1);

	/*
	 * Restore signal mask - leave the RXREADY bit set if was already
	 * set, else clear it.
	 */
	kern_task_set_sigmask(KERN_SIGNAL_ALL_MASK &
	    ~(KERN_SIGNAL_TASK_WAIT_PORT_RXREADY),
	    sigmask);

	return (msg);
}
