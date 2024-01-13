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

#include <kern/ipc/pipe.h>

void
kern_ipc_pipe_setup(kern_ipc_pipe_t *pipe, char *buf, int len,
    uint32_t max_msg_size)
{
}

/**
 * Shutdown a pipe.
 *
 * This just marks a pipe as shutdown - no new work can be scheduled
 * to it.  Existing data can be read from the pipe, but then no more
 * data will be available.
 */
void
kern_ipc_pipe_close(kern_ipc_pipe_t *pipe)
{
}

/**
 * Close a pipe.
 *
 * This just marks a pipe as closed - no new work can be scheduled
 * to it and existing data will be flushed.
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
}

/**
 * Set the pipe owner.  In reality this means the consumer -
 * this is who will be responsible for closing the pipe, and
 * will receive signals when data is queued into it.
 */
kern_error_t
kern_ipc_pipe_set_owner(kern_ipc_pipe_t *pipe, kern_task_id_t task)
{
	return KERN_ERR_UNIMPLEMENTED;
}

kern_error_t
kern_ipc_pipe_queue(kern_ipc_pipe_t *pipe, const kern_ipc_msg_t *msg)
{
	return KERN_ERR_UNIMPLEMENTED;
}

kern_error_t
kern_ipc_pipe_dequeue(kern_ipc_pipe_t *pipe, kern_ipc_msg_t *msg)
{
	return KERN_ERR_UNIMPLEMENTED;
}

kern_error_t
kern_ipc_pipe_flush(kern_ipc_pipe_t *pipe, uint32_t *num)
{
	*num = 0;
	return KERN_ERR_UNIMPLEMENTED;
}

