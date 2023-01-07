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
#ifndef	__USER_USER_EXEC_H__
#define	__USER_USER_EXEC_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <hw/types.h>


/*
 * As implemented in USER_TASK.ld, there are a handful of fields in
 * the header before the used binary itself.  User binaries are PIC
 * using r9 as the base pointer to GOT.
 *
 * Header contents, aligned on DWORD boundaries (4 bytes):
 *
 * + text offset (uint32_t)
 * + text size
 * + GOT offset
 * + GOT size
 * + BSS offset
 * + BSS size
 * + data offset
 * + data size
 * + rodata offset
 * + rodata size
 * + heap size
 * + stack size
 */

struct user_exec_program_header {
	uint32_t text_offset;
	uint32_t text_size;
	uint32_t got_offset;
	uint32_t got_size;
	uint32_t bss_offset;
	uint32_t bss_size;
	uint32_t data_offset;
	uint32_t data_size;
	uint32_t rodata_offset;
	uint32_t rodata_size;

	uint32_t heap_size;
	uint32_t stack_size;
};

/* Note: is paddr_t the correct address type to use here? */
struct user_exec_program_addrs {
	paddr_t text_addr;
	paddr_t got_addr;
	paddr_t bss_addr;
	paddr_t data_addr;
	paddr_t rodata_addr;

	paddr_t heap_addr;
	paddr_t stack_addr;
};


/*
 * Parse out the header.
 */
extern	bool user_exec_program_parse_header(paddr_t addr, size_t size,
    struct user_exec_program_header *hdr);

/*
 * Setup all of the segments in preparation for starting
 * user task execution.
 */
extern	bool user_exec_program_setup_segments(paddr_t addr, size_t size,
    const struct user_exec_program_header *hdr,
    struct user_exec_program_addrs *addrs);

#endif	/* __USER_USER_EXEC_H__ */
