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
#include <kern/libraries/mem/mem.h>

#include <kern/console/console.h>

#include <kern/core/exception.h>
#include <kern/core/task_defs.h>
#include <kern/core/task.h>
#include <kern/core/signal.h>
#include <kern/core/physmem.h>
#include <kern/core/error.h>

#include <kern/ipc/pipe.h>

static platform_spinlock_t kern_ipc_pipe_spinlock;

void
kern_ipc_pipe_init(void)
{
	platform_spinlock_init(&kern_ipc_pipe_spinlock);
}

static kern_error_t
kern_ipc_pipe_consume_locked(kern_ipc_pipe_t *pipe, kern_ipc_msg_t *msg)
{
	return KERN_ERR_UNIMPLEMENTED;
}

static uint32_t
kern_ipc_pipe_space_left_locked(kern_ipc_pipe_t *pipe)
{

	return (pipe->buf_size - pipe->buf_offset);
}

static kern_error_t
kern_ipc_pipe_queue_locked(kern_ipc_pipe_t *pipe, const kern_ipc_msg_t *msg)
{

	if (pipe->state != KERN_IPC_PIPE_STATE_OPEN) {
		return KERN_ERR_SHUTDOWN;
	}

	/* Check message size */
	if (msg->len > pipe->max_msg_size) {
		return KERN_ERR_TOOBIG;
	}

	/* Make sure we have enough space left */
	if (msg->len > kern_ipc_pipe_space_left_locked(pipe)) {
		return KERN_ERR_NOSPC;
	}

	/* Copy, advance offset */
	kern_memcpy(
	    pipe->buf_ptr + pipe->buf_offset,
	    (const char *) msg,
	    msg->len);

	/* Bump offset along */
	pipe->buf_offset += msg->len;

	return KERN_ERR_OK;
}

kern_error_t
kern_ipc_pipe_dequeue_locked(kern_ipc_pipe_t *pipe, kern_ipc_msg_t *msg)
{
	return KERN_ERR_UNIMPLEMENTED;
}

static kern_error_t
kern_ipc_pipe_flush_locked(kern_ipc_pipe_t *pipe, uint32_t *num)
{
	kern_ipc_msg_t hdr;

	if (num != NULL) {
		(*num) = 0;
	}

	while (kern_ipc_pipe_consume_locked(pipe, &hdr) == KERN_ERR_OK) {
		if (num != NULL) {
			(*num)++;
		}
	}

	return KERN_ERR_OK;
}

void
kern_ipc_pipe_setup(kern_ipc_pipe_t *pipe, char *buf, int len,
    uint32_t max_msg_size)
{
	kern_bzero(pipe, sizeof(kern_ipc_pipe_t));

	/*
	 * XXX TODO: have it be a non-open state and
	 * make an explicit call to mark it open?
	 */
	pipe->state = KERN_IPC_PIPE_STATE_OPEN;

	pipe->buf_ptr = buf;
	pipe->buf_size = len;
	pipe->buf_offset = 0;
	pipe->max_msg_size = max_msg_size;
}

/**
 * Shutdown a pipe.
 *
 * This just marks a pipe as shutdown - no new work can be scheduled
 * to it.  Existing data can be read from the pipe, but then no more
 * data will be available.
 *
 * This can only be called by the owner task.
 */
void
kern_ipc_pipe_shutdown(kern_ipc_pipe_t *pipe)
{
	/* TODO: make sure the caller task is the owner */
	pipe->state = KERN_IPC_PIPE_STATE_SHUTDOWN;
}

/**
 * Close a pipe.
 *
 * This just marks a pipe as closed - no new work can be scheduled
 * to it and existing data will be flushed.
 *
 * This can only be called by the owner task.
 *
 * Ideally a pipe will have been shutdown and drained first, so
 * any existing work has been completed.
 *
 * Once a pipe has been marked as closed it can't be re-opened, and
 * the memory buffer that it points to is considered no bueno for further
 * use.
 */
void
kern_ipc_pipe_close(kern_ipc_pipe_t *pipe)
{
	/* TODO: verify caller task is owner */
	/* TODO: verify pipe was alread shutdown and flushed */

	platform_spinlock_lock(&kern_ipc_pipe_spinlock);
	pipe->state = KERN_IPC_PIPE_STATE_CLOSED;

	/* Flush the pipe just in case, flush is non-blocking */
	kern_ipc_pipe_flush_locked(pipe, NULL);
	platform_spinlock_unlock(&kern_ipc_pipe_spinlock);
}

/**
 * Set the pipe owner.  In reality this means the consumer -
 * this is who will be responsible for closing the pipe, and
 * will receive signals when data is queued into it.
 */
kern_error_t
kern_ipc_pipe_set_owner(kern_ipc_pipe_t *pipe, kern_task_id_t task)
{
	pipe->owner_task = task;
	return KERN_ERR_OK;
}

kern_error_t
kern_ipc_pipe_queue(kern_ipc_pipe_t *pipe, const kern_ipc_msg_t *msg)
{
	kern_error_t t;

	platform_spinlock_lock(&kern_ipc_pipe_spinlock);
	t = kern_ipc_pipe_queue_locked(pipe, msg);
	platform_spinlock_unlock(&kern_ipc_pipe_spinlock);

	return t;
}

/*
 * Dequeue data from the pipe.
 *
 * On entry: msg->len contains the size of the buffer, including the header.
 *
 * If a message is received then it will return KERN_ERR_OK and
 * msg->len will be updated to include the size of the message.
 *
 * Else, the message will be left alone and an error will be returned.
 */
kern_error_t
kern_ipc_pipe_dequeue(kern_ipc_pipe_t *pipe, kern_ipc_msg_t *msg)
{
	kern_error_t t;

	platform_spinlock_lock(&kern_ipc_pipe_spinlock);
	t = kern_ipc_pipe_dequeue_locked(pipe, msg);
	platform_spinlock_unlock(&kern_ipc_pipe_spinlock);
	return t;
}

/*
 * Consume a message from the pipe.
 *
 * This is called to consume a message without caring about its
 * contents.  For example, we're closing, or the message is too big.
 *
 * If a message is avaiable, msg->id and msg->len will be set and
 * KERN_ERR_OK will be returned.
 *
 * Else an error will be returned.
 */
kern_error_t
kern_ipc_pipe_consume(kern_ipc_pipe_t *pipe, kern_ipc_msg_t *msg)
{
	kern_error_t t;

	platform_spinlock_lock(&kern_ipc_pipe_spinlock);
	t = kern_ipc_pipe_consume_locked(pipe, msg);
	platform_spinlock_unlock(&kern_ipc_pipe_spinlock);

	return t;
}


/*
 * Flush the pipe.
 *
 * If the caller wants to know how many entries were flushed,
 * they can pass in a pointer.
 */
kern_error_t
kern_ipc_pipe_flush(kern_ipc_pipe_t *pipe, uint32_t *num)
{
	kern_error_t error;

	platform_spinlock_lock(&kern_ipc_pipe_spinlock);
	error = kern_ipc_pipe_flush_locked(pipe, num);
	platform_spinlock_unlock(&kern_ipc_pipe_spinlock);
	return error;
}
