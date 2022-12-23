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

#ifndef	__FLASH_FLASH_RESOURCE_ENTRY_H__
#define	__FLASH_FLASH_RESOURCE_ENTRY_H__

/*
 * This is the kernel specific bits for parsing / representing an XIP
 * flash entry (called a 'PAK').
 *
 * It includes both the header and the calculated/parsed entry fields
 * with paddr_t fields representing the payload.
 */

struct flash_resource_pak {
	struct flash_resource_entry_header hdr;
	paddr_t payload_start;
	size_t payload_size;
};

#endif	/* __FLASH_FLASH_RESOURCE_PAK_H__ */
