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
#include <stdlib.h>

#include <hw/types.h>

#include <kern/core/physmem.h>
#include <kern/console/console.h>

void
kern_physmem_init(void)
{
}

/**
 * Add the physical memory range between 'start' and 'end'-1.
 */
void
kern_physmem_add_range(paddr_t start, paddr_t end, uint32_t flags)
{
	paddr_t size;

	/*
	 * XXX TODO: yeah, it's cast to uint32_t for printing
	 * because I don't have a 64 bit printing type to use.
	 */
	size = end - start;
	console_printf("[physmem] adding 0x%x -> 0x%x (%d bytes),"
	    " flags 0x%08x\n",
	    (uint32_t) start, (uint32_t) end, size, flags);
}
