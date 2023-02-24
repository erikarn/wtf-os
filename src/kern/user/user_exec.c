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

#include <hw/types.h>

#include <kern/console/console.h>

#include <kern/libraries/mem/mem.h>
#include <kern/libraries/align/align_paddr.h>

#include <kern/user/user_exec.h>

/*
 * This kernel code module drives initialising a user task from either
 * an XIP task or (later on) loading into memory from external storage.
 *
 * It takes care of populating the GOT based on the contents of the
 * user binary.
 */

static bool
is_in_range(uint32_t val, uint32_t start, uint32_t len)
{
	return ((val >= start) && (val < (start+len)));
}

/*
 * Parse out the header.
 */
bool
user_exec_program_parse_header(paddr_t addr, size_t size,
    struct user_exec_program_header *hdr)
{
	uint32_t val;
	paddr_t s;

	s = addr;

	/* Parse out fields */
	/* XXX TODO: yeah, need a generated parser here, le sigh */
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->text_offset = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->text_size = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->got_offset = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->got_size = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->bss_offset = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->bss_size = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->data_offset = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->data_size = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->rodata_offset = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->rodata_size = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->heap_size = val; s = s + sizeof(uint32_t);
	kern_memcpy(&val, (const char *) s, sizeof(uint32_t)); hdr->stack_size = val; s = s + sizeof(uint32_t);

	/* XXX TODO: bounds check the header fields */

	/*
	 * Note: all the offsets are from the beginning of this user
	 * binary payload.  We're not adjusting them here; instead
	 * we are just going to validate that the offset+size fits
	 * within the size of this particular 'size' region of
	 * memory.
	 */

	/* For now we're fine, let's just do it */
	return (true);
}

/*
 * Various segment setup routines.
 *
 * The caller is responsible for populating the contents
 * of struct user_exec_program_addrs with either relocated
 * paddrs based on addr + offset, or allocating buffers
 * to copy fields into.  That way this routine doesn't
 * need to worry about /how/ to allocate/free memory,
 * or whether we're XIP or not, etc.  It just assumes
 * everything is ready for relocation and preparing for
 * running.
 */

bool
user_exec_program_setup_data_segment(paddr_t addr, size_t size,
    const struct user_exec_program_header *hdr,
    struct user_exec_program_addrs *addrs)
{
	/*
	 * Here we're copying the contents of the data
	 * segment (that's read only from the image)
	 * into our allocated data segment.
	 *
	 * XXX TODO: we really should just define a macro
	 * to turn a paddr_t into a const char or char,
	 * or const void or void.
	 */
	kern_memcpy((void *) addrs->data_addr, (void *)
	    (addr + (hdr->data_offset)),
	    hdr->data_size);

	return (true);
}

bool
user_exec_program_setup_bss_segment(paddr_t addr, size_t size,
    const struct user_exec_program_header *hdr,
    struct user_exec_program_addrs *addrs)
{
	/*
	 * Here we just zero the bss segment as that is what
	 * it's there for!
	 */
	kern_bzero((void *) addrs->bss_addr, hdr->bss_size);

	return (true);
}

bool
user_exec_program_setup_rodata_segment(paddr_t addr, size_t size,
    const struct user_exec_program_header *hdr,
    struct user_exec_program_addrs *addrs)
{
	/*
	 * Nothing to do here - rodata doesn't require
	 * indirection or copying.
	 */
	return (true);
}

bool
user_exec_program_setup_got_segment(paddr_t addr, size_t size,
    const struct user_exec_program_header *hdr,
    struct user_exec_program_addrs *addrs)
{
	paddr_t s;
	uint32_t *got_entry;
	uint32_t i, val;

	/*
	 * Here's where all the fun relocation magic occurs!
	 *
	 * And yes it's inefficient, with memcpy, etc, but
	 * hey, get it working first.
	 */

	s = addr + hdr->got_offset;
	got_entry = (uint32_t *) addrs->got_addr;
	for (i = 0; i < hdr->got_size; i += sizeof(uint32_t)) {
		/* XXX parser? I dunno */
		kern_memcpy(&val, (const char *) s, sizeof(uint32_t));

		/*
		 * val is now an offset inside our file, starting at 0.
		 *
		 * We need to now compare it against the segments we
		 * know about to see which segment it is in, and relocate
		 * them to the correct paddr_t addresses.
		 */
		if (is_in_range(val, hdr->bss_offset, hdr->bss_size)) {
			*got_entry = val + addrs->bss_addr;
		} else if (is_in_range(val, hdr->text_offset, hdr->text_size)) {
			*got_entry = val + addrs->text_addr;
		} else if (is_in_range(val, hdr->data_offset, hdr->data_size)) {
			*got_entry = val + addrs->data_addr;
		} else if (is_in_range(val, hdr->rodata_offset,
		    hdr->rodata_size)) {
			*got_entry = val + addrs->rodata_addr;
		} else {
			/* XXX TODO: more debugging obv */
			console_printf("%s: couldn't find GOT offset (%d) in segments!\n", __func__, val);
			return (false);
		}

		/* Next entry */
		got_entry++;
		s += sizeof(uint32_t);
	}

	return (true);
}

/*
 * Setup all of the segments in preparation for starting
 * user task execution.
 */
bool
user_exec_program_setup_segments(paddr_t addr, size_t size,
    const struct user_exec_program_header *hdr,
    struct user_exec_program_addrs *addrs)
{

	if (user_exec_program_setup_data_segment(addr, size, hdr, addrs) == false) {
		return (false);
	}
	if (user_exec_program_setup_rodata_segment(addr, size, hdr, addrs) == false) {
		return (false);
	}
	if (user_exec_program_setup_bss_segment(addr, size, hdr, addrs) == false) {
		return (false);
	}
	if (user_exec_program_setup_got_segment(addr, size, hdr, addrs) == false) {
		return (false);
	}

	return (true);
}
