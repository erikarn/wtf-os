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
#include <kern/core/error.h>

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

static kern_error_t
kern_ipc_port_get_reference_locked(struct kern_ipc_port *port)
{
	kern_error_t retval;

	if (port->refcount == 255) {
		retval = KERN_ERR_NOSPC;
	} else {
		port->refcount++;
		retval = KERN_ERR_OK;
	}
	return (retval);
}

/**
 * Called by the port reference owner, whether it's task local or not.
 */
void
kern_ipc_port_free_reference_locked(struct kern_ipc_port *port)
{
	/*
	 * XXX TODO: not sure how to communicate to the port
	 * owner that the port reference has changed / is now able
	 * to complete shutdown/close/free.
	 */
	port->refcount--;
}

kern_error_t
kern_ipc_port_get_reference(struct kern_ipc_port *port)
{
	kern_error_t retval;

	platform_spinlock_lock(&kern_port_ipc_spinlock);
	retval = kern_ipc_port_get_reference_locked(port);
	platform_spinlock_unlock(&kern_port_ipc_spinlock);
	return (retval);
}

/**
 * Called by the port reference owner, whether it's task local or not.
 *
 * Once the refcount is zero, it can be freed.
 */
void
kern_ipc_port_free_reference(struct kern_ipc_port *port)
{
	platform_spinlock_lock(&kern_port_ipc_spinlock);
	kern_ipc_port_free_reference_locked(port);
	platform_spinlock_unlock(&kern_port_ipc_spinlock);
}


/**
 * Add the given port to the global namespace.
 *
 * Called by the port owner.
 *
 * This adds it, and will increment the refcount on the port.
 *
 * @param[in] port port to add
 * @param[in] name name to add
 * @retval KERN_ERR_OK if OK, error if error
 */
kern_error_t
kern_ipc_port_add_name(struct kern_ipc_port *port, const char *name)
{
	kern_error_t retval = KERN_ERR_OK;

	kern_strlcpy(port->port_name, name, KERN_IPC_PORT_NAME_LEN);

	platform_spinlock_lock(&kern_port_ipc_spinlock);
	retval = kern_ipc_port_get_reference_locked(port);
	if (retval != KERN_ERR_OK) {
		goto error;
	}
	list_add_tail(&kern_port_name_list, &port->name_node);

error:
	platform_spinlock_unlock(&kern_port_ipc_spinlock);
	return (retval);
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
 * This returns the port if found, with the refcount incremented.
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
	if (port == NULL) {
		goto done;
	}

	/* Don't return ports if they're not in running state */
	if (port->state != KERN_IPC_PORT_STATE_RUNNING) {
		port = NULL;
		goto done;
	}

	/* get reference; fail if too many */
	if (kern_ipc_port_get_reference_locked(port) != KERN_ERR_OK) {
		port = NULL;
		goto done;
	}
done:
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
		kern_ipc_port_free_reference_locked(port);
	}
	platform_spinlock_unlock(&kern_port_ipc_spinlock);

	return (port != NULL);
}

/**
 * Initialise the given port.
 *
 * It will be initialised with the given parameters and connected
 * to the given task.
 *
 * XXX TODO: this needs to use the task lock somehow!
 * Maybe this shouldn't be what users use, and instead a routine
 * in task.c creates a port specifically /for/ the task, and can
 * thus add it to the task list.
 */
static kern_error_t
kern_ipc_port_setup(struct kern_ipc_port *port, kern_task_id_t task)
{
	struct kern_task *t;

	t = kern_task_lookup(task);
	if (t == NULL) {
		return (KERN_ERR_INVALID_TASKID);
	}

	list_node_init(&port->owner_node);
	list_node_init(&port->recv_notif_node);
	list_node_init(&port->compl_notif_node);
	list_node_init(&port->name_node);
	list_head_init(&port->msg_head);
	port->state = KERN_IPC_PORT_STATE_IDLE;
	port->port_name[0] = '\0';

	list_head_init(&port->recv_msg.list);
	port->recv_msg.num = 0;
	port->recv_msg.max = 8; /* XXX hard-coded for now */

	list_head_init(&port->compl_msg.list);
	port->compl_msg.num = 0;
	port->compl_msg.max = 8; /* XXX hard-coded for now */

	port->owner_task = task;

	/* XXX owner list stuff shouldn't be in here */
	/* XXX TODO: Need to be using the task lock! Ew! */
	list_add_tail(&t->task_port_list, &port->owner_node);
	kern_task_refcount_dec(t); t = NULL;

	/* Owner owns this! */
	port->refcount = 1;

	return (KERN_ERR_OK);
}

struct kern_ipc_port *
kern_ipc_port_create(kern_task_id_t task)
{
	struct kern_ipc_port *port;
	kern_error_t err;

	port = (void *) kern_physmem_alloc(sizeof(*port), 4,
	    KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (port == NULL) {
		return (NULL);
	}
	err = kern_ipc_port_setup(port, task);
	if (err != KERN_ERR_OK) {
		kern_physmem_free((paddr_t) port);
		return (NULL);
	}
	return (port);
}

/**
 * Destroy the allocation of this port.
 *
 * This will eventually check if the port refcount is zero,
 * and only do work when it is.
 */
void
kern_ipc_port_destroy(struct kern_ipc_port *port)
{
	console_printf("%s: TODO!; refcount=%d\n", __func__, port->refcount);
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
	console_printf("%s: TODO\n", __func__);

	platform_spinlock_lock(&kern_port_ipc_spinlock);
	port->state = KERN_IPC_PORT_STATE_SHUTDOWN;
	platform_spinlock_unlock(&kern_port_ipc_spinlock);
}

/**
 * Close the given port.
 *
 * Only call from the port owner task.
 *
 * This marks the port as shutting down if it isn't, purges
 * any pending IPCs in the queue, and then finishes wrapping
 * up state for this port.
 */
bool
kern_ipc_port_close(struct kern_ipc_port *port)
{
	console_printf("%s: TODO\n", __func__);

	/*
	 * Skip shutdown; go straight to closed.
	 */
	platform_spinlock_lock(&kern_port_ipc_spinlock);
	port->state = KERN_IPC_PORT_STATE_CLOSED;

	/*
	 * XXX TODO: walk the list of RX'ed frames that
	 * we haven't yet handled and queue them for
	 * completion w/ a canceled/closed status.
	 * Yeah I'm going to have to add that to the IPC
	 * message.  That way the receiver knows to
	 * immediately toss them.
	 */

	/*
	 * XXX TODO: Next up is frames in our completed
	 * queue.
	 */

	/*
	 * XXX TODO: Frames that we own that are on the
	 * queues of other ports?  Maybe we can cancel them.
	 * Maybe they're dequeued already and are owned
	 * by the receiving task for now.  In any case,
	 * we'll cancel the ones we can cancel and the
	 * others we'll need to wait until they're completed
	 * and on our completion queue before we cancel those.
	 */

	/*
	 * We also can't close if there are outstanding refcounts,
	 * so we need to stick around until that's done.
	 */

	/*
	 * XXX TODO; Yes, this means I need to modify this
	 * routine to be callable multiple times and return
	 * whether the close has finished or not.  That way
	 * the owner (ie, some wrapper function?) can either
	 * block waiting for things to finish, or can return
	 * and let the task continue doing other stuff; it
	 * can then check in later on to free things.
	 *
	 * Finally yeah, this means during actual task shutdown
	 * we'll end up looping over ports to wait until
	 * they're done before we actually /exit/ the task.
	 * Maybe that's nice, maybe we can put ports that
	 * just have refcounts outstanding on some dead list
	 * and free them in the idle thread if needs be,
	 * so the task can be cleaned up later.
	 */
	platform_spinlock_unlock(&kern_port_ipc_spinlock);

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
kern_error_t
kern_port_enqueue_msg(struct kern_ipc_port *lcl_port,
    struct kern_ipc_port *rem_port,
    struct kern_ipc_msg *msg)
{
	kern_error_t retval = KERN_ERR_OK;

	platform_spinlock_lock(&kern_port_ipc_spinlock);

	/*
	 * Check local port state, if we're not active then
	 * we can't queue a message.
	 */
	if (lcl_port->state != KERN_IPC_PORT_STATE_RUNNING) {
		retval = KERN_ERR_SHUTDOWN;
		goto error;
	}

	/*
	 * Check remote port state, if we're not active then
	 * we can't queue a message.
	 */
	if (rem_port->state != KERN_IPC_PORT_STATE_RUNNING) {
		retval = KERN_ERR_SHUTDOWN;
		goto error;
	}

	/* Check if there's space at the receiver */
	if (rem_port->recv_msg.num >= rem_port->recv_msg.max) {
		retval = KERN_ERR_NOSPC;
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

	platform_spinlock_unlock(&kern_port_ipc_spinlock);

	/* Notify receiver port */
	kern_task_signal(rem_port->owner_task,
	    KERN_SIGNAL_TASK_WAIT_PORT_RXREADY);
	return (KERN_ERR_OK);

error:
	platform_spinlock_unlock(&kern_port_ipc_spinlock);
	return (retval);
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
