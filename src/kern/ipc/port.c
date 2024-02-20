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
//#include <kern/ipc/msg.h>

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

static bool
kern_ipc_port_delete_name_port_locked(struct kern_ipc_port *port)
{
	if ((port->flags & KERN_IPC_PORT_FLAGS_NAMED) == 0) {
		return false;
	}

	list_delete(&kern_port_name_list, &port->name_node);
	port->port_name[0] = '\0';
	kern_ipc_port_free_reference_locked(port);

	return true;
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
		kern_ipc_port_delete_name_port_locked(port);
	}
	platform_spinlock_unlock(&kern_port_ipc_spinlock);

	return (port != NULL);
}

/**
 * Initialise the given port.
 *
 * It will be initialised with the given parameters.
 *
 * Maybe this shouldn't be what users use, and instead a routine
 * in task.c creates a port specifically /for/ the task, and can
 * thus add it to the task list.
 */
static kern_error_t
kern_ipc_port_setup(struct kern_ipc_port *port)
{

	list_node_init(&port->owner_node);
	list_node_init(&port->name_node);
	port->state = KERN_IPC_PORT_STATE_IDLE;
	port->port_name[0] = '\0';

	/* Owner owns this! */
	port->refcount = 1;

	return (KERN_ERR_OK);
}

/*
 * Create a port via malloc.
 *
 * It doesn't yet have an owner task.  The owning task
 * needs to do a little bit of legwork adding it to its
 * task list, etc.
 */
struct kern_ipc_port *
kern_ipc_port_create()
{
	struct kern_ipc_port *port;
	kern_error_t err;

	port = (void *) kern_physmem_alloc(sizeof(*port), 4,
	    KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (port == NULL) {
		return (NULL);
	}
	err = kern_ipc_port_setup(port);
	if (err != KERN_ERR_OK) {
		kern_physmem_free((paddr_t) port);
		return (NULL);
	}
	return (port);
}

/**
 * Destroy the given IPC port.
 *
 * This frees all of the resources for the given port.
 *
 * I haven't yet figured out the rest of the semantics
 * around the shutdown path, notably how/when the queued
 * messages are freed, how to notify the callers, etc.
 */
void
kern_ipc_port_destroy(struct kern_ipc_port *port)
{
	/* Go through a shutdown/close path if required */
	kern_ipc_port_close(port);

	/*
	 * refcount must be 1, if it's higher than
	 * something still has a reference to this port!
	 */
	console_printf("%s: TODO! Refcount! etc\n", __func__);
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
 *
 * The peer relationship isn't torn down at this point;
 * just that any further enqueues will fail.
 */
void
kern_ipc_port_shutdown(struct kern_ipc_port *port)
{
	console_printf("%s: TODO\n", __func__);

	platform_spinlock_lock(&kern_port_ipc_spinlock);

	/* Mark shutdown */
	port->state = KERN_IPC_PORT_STATE_SHUTDOWN;

	/* Remove from global namespace */
	kern_ipc_port_delete_name_port_locked(port);

	platform_spinlock_unlock(&kern_port_ipc_spinlock);
}

static void
_kern_ipc_port_reference_link_clear_locked(struct kern_ipc_port *port, struct kern_ipc_port *rem)
{
	kern_ipc_port_free_reference(port->peer);
	kern_ipc_port_free_reference(port);
}

/*
 * Remove links to any peer service ports to this port.
 */
static void
_kern_ipc_port_service_list_deregister_locked(struct kern_ipc_port *port)
{
	struct list_node *n;
	struct kern_ipc_port *rem;

	while (! list_is_empty(&port->service_list.head)) {
		n = list_get_head(&port->service_list.head);
		rem = container_of(n, struct kern_ipc_port, service_list.node);

		/* Remove from list */
		list_delete(&port->service_list.head, n);

		/* Remove the references between these two ports */
		_kern_ipc_port_reference_link_clear_locked(port, rem);
	}
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
	console_printf("%s: called\n", __func__);

	/*
	 * Skip shutdown; go straight to closed.
	 */
	platform_spinlock_lock(&kern_port_ipc_spinlock);
	port->state = KERN_IPC_PORT_STATE_CLOSED;

	/* TODO: all the work */

	/* Remove it from the global namespace if required */
	kern_ipc_port_delete_name_port_locked(port);

	/* TODO: notify pending/completed messages that they're done */
	console_printf("%s: TODO: message pending/completed callback/completion\n", __func__);

	/* Remove it from any peer relationship if required */
	/* (Each has a refcount on each other here) */
	if (port->peer != NULL) {
		struct kern_ipc_port *rem = port->peer;

		/* XXX TODO: turn into a method! */
		port->peer->peer = NULL;
		port->peer = NULL;

		/* Clear the reference between the two ports */
		/* TODO: what about messages that this remote port has queued to us? */
		_kern_ipc_port_reference_link_clear_locked(port, rem);

		/* TODO: hopefully the above doesn't free the damned ports! */
	}

	/* And peer lists too, if we have multiple connections to us */
	/* TODO: what about messages that those ports have queued to us? */
	_kern_ipc_port_service_list_deregister_locked(port);

	platform_spinlock_unlock(&kern_port_ipc_spinlock);

	/* For now we always return we were able to close it */
	return (true);
}

/**
 * Connect to the given remote port.
 *
 * This connects the given local port, local to the current task,
 * to the given remote port (in any task.)
 *
 * This will check if the port is a service port or not, and
 * either establish a service connection, or a straight peer
 * to peer connection.
 */
kern_error_t
kern_ipc_port_connect(struct kern_ipc_port *lcl, struct kern_ipc_port *rem)
{
	return KERN_ERR_UNIMPLEMENTED;
}

/**
 * Disconnect from the given remote port.
 *
 * This will disconnect from the given remote port,
 * severing the port peer relationship and/or removing
 * from the service list if appropriate.
 */
kern_error_t
kern_ipc_port_disconnect(struct kern_ipc_port *lcl, struct kern_ipc_port *rem)
{
	return KERN_ERR_UNIMPLEMENTED;
}

