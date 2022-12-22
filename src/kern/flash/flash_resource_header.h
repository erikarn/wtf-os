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

#ifndef	__FLASH_FLASH_RESOURCE_HEADER_H__
#define	__FLASH_FLASH_RESOURCE_HEADER_H__

/*
 * This struct represents a header.  It shouldn't be serialised/deserialised
 * straight from flash because who knows what the compiler will do.
 *
 * Fields are uint32_t's stored in little endian format.
 */

struct flash_resource_entry_header {
	uint32_t magic;
	uint32_t checksum;
	uint32_t type;
	uint32_t length;
	uint32_t alignment;
	uint32_t namelength;
	uint32_t payload_length;
	uint32_t rsv0;
};

#define		ENTRY_MAGIC	0x05091979
#define		HEADER_SIZE	(sizeof(uint32_t) * 8)

#define		PAK_ALIGNMENT	32

#endif	/* __FLASH_FLASH_RESOURCE_HEADER_H__ */
