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

#include <kern/libraries/mem/mem.h>
#include <kern/libraries/string/string.h>

#include <kern/flash/flash_resource.h>
#include <kern/flash/flash_resource_header.h>

/*
 * XXX TODO: put this in a library, not here
 */
const paddr_t
align_paddr_t(paddr_t val, uint32_t align)
{
	paddr_t v;

	v = (val + (align - (val % align)));
	return (v);
}

/*
 * Check the pak at the given location.  Populate the header
 * and return true if it's valid.
 */
bool
flash_resource_check_pak(paddr_t start, struct flash_resource_entry_header *hdr)
{
	uint32_t val;
	const char *s = (const char *)(uintptr_t *) start;
	char label[16] = { 0 };

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
	hdr->magic = val; s += sizeof(uint32_t);

	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ hdr->checksum = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ hdr->type = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ hdr->length = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ hdr->alignment = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ hdr->namelength = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ hdr->payload_length = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, s, sizeof(uint32_t)); /* le32 */ hdr->rsv0 = val; s = s + sizeof(uint32_t);

	/* XXX TODO: crc32b check */

	/* XXX TODO: should extend miniprintf to include %.*s */

	/*
	 * The string starts at the end of the header, no alignment
	 * requirements.
	 */
	kern_strlcpyn(label, s, sizeof(label), hdr->namelength);

	console_printf("[console] pak: %s: %d byte payload, %d byte total length\n",
	    label,
	    hdr->payload_length,
	    hdr->length);

	return (true);
}

bool
flash_resource_span_init(flash_resource_span_t *span, paddr_t start,
    size_t size)
{
	struct flash_resource_entry_header hdr;
	paddr_t end = start + size;

	if (span->flags & FLASH_RESOURCE_SPAN_FLAGS_SETUP)
		return (false);

	span->start = start;
	span->size = size;
	span->flags |= FLASH_RESOURCE_SPAN_FLAGS_SETUP;

	/* And here's where we COULD scan the span to gather some useful info */

	console_printf("[flash] Flash resource span: start @ 0x%08x, %d bytes\n",
	    start, size);

	/* Walk the paks we have */
	while (flash_resource_check_pak(start, &hdr)) {
		start += hdr.length;
		if (start > end)
			break;
	}

	return (true);
}
