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

#include "hw/types.h"

#include <kern/console/console.h>

#include <kern/libraries/mem/mem.h>
#include <kern/libraries/string/string.h>
#include <kern/libraries/align/align_paddr.h>

#include <kern/flash/flash_resource.h>
#include <kern/flash/flash_resource_header.h>
#include <kern/flash/flash_resource_pak.h>

/*
 * Check the pak at the given location.  Populate the header
 * and return true if it's valid.
 */
bool
flash_resource_check_pak(paddr_t start, struct flash_resource_pak *pak,
    char *label, int labellen)
{
	uint32_t val;
	const char *s = (const char *)(uintptr_t *) start;

	/* XXX TODO: these really need to be converted to le32! */
	kern_memcpy(&val, s, sizeof(uint32_t));
	if (val != ENTRY_MAGIC) {
		console_printf("[flash] invalid magic at 0x%x (found 0x%x, expected 0x%x)\n",
		    start,
		    val,
		    ENTRY_MAGIC);
		return (false);
	}

	/* Ok, populate the rest of the header now */
	/* XXX TODO: yes, need le32 conversion here */
	/*
	 * XXX TODO: yes, should really build tools to build parsers for
	 * stuff like this
	 */
	pak->hdr.magic = val; s += sizeof(uint32_t);

	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ pak->hdr.checksum = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ pak->hdr.type = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ pak->hdr.length = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ pak->hdr.alignment = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ pak->hdr.namelength = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ pak->hdr.payload_length = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ pak->hdr.rsv0 = val; s = s + sizeof(uint32_t);

	/* XXX TODO: crc32b check */

	/* XXX TODO: should extend miniprintf to include %.*s */

	/*
	 * The string starts at the end of the header, no alignment
	 * requirements.
	 */
	kern_strlcpyn(label, s, labellen, pak->hdr.namelength);

	console_printf("[console] pak: %s@0x%x %d byte payload, %d byte total length\n",
	    label,
	    s,
	    pak->hdr.payload_length,
	    pak->hdr.length);

	/* Payload starts at aligned value after the start + hdr + string */
	pak->payload_start =
	    align_paddr_t(start + HEADER_SIZE + pak->hdr.namelength,
	    pak->hdr.alignment);
	pak->payload_size = pak->hdr.payload_length;

	return (true);
}

bool
flash_resource_span_init(flash_resource_span_t *span, paddr_t start,
    size_t size)
{
	struct flash_resource_pak pak;
	paddr_t end = start + size;
	char label[16];

	if (span->flags & FLASH_RESOURCE_SPAN_FLAGS_SETUP)
		return (false);

	span->start = start;
	span->size = size;
	span->end = end;
	span->flags |= FLASH_RESOURCE_SPAN_FLAGS_SETUP;

	/* And here's where we COULD scan the span to gather some useful info */

	console_printf("[flash] Flash resource span: start @ 0x%08x, %d bytes\n",
	    start, size);

	/* Walk the paks we have */
	kern_bzero(label, sizeof(label));
	while (flash_resource_check_pak(start, &pak, label, sizeof(label))) {
		start += pak.hdr.length;
		if (start > end)
			break;
		kern_bzero(label, sizeof(label));
	}

	return (true);
}

bool
flash_resource_lookup(flash_resource_span_t *span,
    struct flash_resource_pak *pak, const char *label)
{
	paddr_t start;
	char pak_label[16];

	if ((span->flags & FLASH_RESOURCE_SPAN_FLAGS_SETUP) == 0) {
		console_printf("%s: not setup\n", __func__);
		return (false);
	}

	/* Walk the paks we have */
	kern_bzero(pak_label, sizeof(pak_label));
	start = span->start;
	while (flash_resource_check_pak(start, pak, pak_label,
	    sizeof(pak_label))) {
		/* XXX TODO: we REALLY need an internal non-crap string representation! */
		if (kern_strncmp(pak_label, label, kern_strlen(label)) == 0) {
			return (true);
		}
		start += pak->hdr.length;
		if (start > span->end)
			break;
		kern_bzero(pak_label, sizeof(pak_label));
	}

	return (false);

}
