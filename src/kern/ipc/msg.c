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

#include <kern/core/physmem.h>

#include <kern/ipc/msg.h>


/**
 * Allocate a message with the given size payload.
 *
 * For now we're just directly using the physmem allocator
 * during bootstrapping.  Later on I'll worry about making
 * it more efficient.
 *
 * @param[in] msg_size Size of payload
 * @return msg if OK, NULL if error
 */
struct kern_ipc_msg *
kern_ipc_msg_allocate(uint32_t msg_size)
{
	struct kern_ipc_msg *msg;
	uint32_t len;

	/*
	 * XXX TODO: I really should make sure the data part starts
	 * at a valid aligned offset.
	 */
	len = sizeof(struct kern_ipc_msg) + msg_size;

	/* XXX TODO: magic numbers for alignment should be platform dependent! */
	msg = (void *) kern_physmem_alloc(len, 4, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	if (msg == NULL) {
		console_printf("%s: alloc failed\n", __func__);
		return (NULL);
	}
	msg->msg_size = msg_size;
	return (msg);
}

/**
 * Free a message and any memory associated with it.
 *
 * @param[in] msg Message to free
 */
void
kern_ipc_msg_free(struct kern_ipc_msg *msg)
{

	kern_physmem_free((paddr_t) msg);
}
