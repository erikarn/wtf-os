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

#ifndef	__ARM_M4_HW_PROT_H__
#define	__ARM_M4_HW_PROT_H__

typedef enum {
	PLATFORM_PROT_TYPE_NONE,
	PLATFORM_PROT_TYPE_EXEC_RO,
	PLATFORM_PROT_TYPE_NOEXEC_RO,
	PLATFORM_PROT_TYPE_NOEXEC_RW,
	PLATFORM_PROT_TYPE_DEVICE_RO,
	PLATFORM_PROT_TYPE_DEVICE_RW,
} platform_prot_type_t;

#endif	/* __ARM_M4_HW_PROT_H__ */
