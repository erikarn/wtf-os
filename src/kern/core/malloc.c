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
#include <stdint.h>

#include <hw/types.h>

#include <kern/core/physmem.h>
#include <kern/core/malloc.h>

/*
 * Yes this is a kernel memory allocator for tasks.
 *
 * It's designed to be used where kernel tasks want their own memory
 * to use, so memory allocated here may eventually be accounted
 * to a kernel task, etc.
 */

void *
kern_malloc(size_t size, uint32_t alignment)
{

	return (void *) (kern_physmem_alloc(size, alignment,
	    KERN_PHYSMEM_ALLOC_FLAG_ZERO));
}

void *
kern_malloc_nonzero(size_t size, uint32_t alignment)
{
	return (void *) (kern_physmem_alloc(size, alignment, 0));
}

void *
kern_realloc(void *ptr, size_t size)
{
	/* XXX TODO */
	return (NULL);
}

void
kern_free(void *ptr)
{

	kern_physmem_free((paddr_t) ptr);
}
