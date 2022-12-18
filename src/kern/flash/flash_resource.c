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
#include <stdbool.h>

#include <kern/console/console.h>

#include <kern/flash/flash_resource.h>

bool
flash_resource_span_init(flash_resource_span_t *span, paddr_t start,
    size_t size)
{
	if (span->flags & FLASH_RESOURCE_SPAN_FLAGS_SETUP)
		return (false);

	span->start = start;
	span->size = size;
	span->flags |= FLASH_RESOURCE_SPAN_FLAGS_SETUP;

	/* And here's where we COULD scan the span to gather some useful info */

	console_printf("[flash] Flash resource span: start @ 0x%08x, %d bytes\n",
	    start, size);

	return (true);
}
