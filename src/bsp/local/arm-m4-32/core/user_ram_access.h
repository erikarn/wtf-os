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

#ifndef	__PLATFORM_USER_RAM_ACCESS_H__
#define	__PLATFORM_USER_RAM_ACCESS_H__

extern	bool platform_user_ram_copy_from_user(const uaddr_t uaddr,
	    paddr_t paddr, uint32_t len);
extern	bool platform_user_ram_copy_to_user(const paddr_t paddr,
	    uaddr_t uaddr, uint32_t len);
extern	bool platform_user_ram_read_byte_from_user(const uaddr_t uaddr,
	    uint8_t *dst);

#endif	/* __PLATFORM_USER_RAM_ACCESS_H__ */
