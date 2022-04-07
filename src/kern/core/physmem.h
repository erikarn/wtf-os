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
#ifndef	__KERN_PHYSMEM_H__
#define	__KERN_PHYSMEM_H__

#include <os/bit.h>

#define	KERN_PHYSMEM_FLAG_NORMAL		BIT_U32(0)
#define	KERN_PHYSMEM_FLAG_EXCLUDE		BIT_U32(1)
#define	KERN_PHYSMEM_FLAG_SRAM			BIT_U32(2)
#define	KERN_PHYSMEM_FLAG_SDRAM			BIT_U32(3)

extern	void kern_physmem_init(void);
extern	void kern_physmem_add_range(paddr_t start, paddr_t end,
	    uint32_t flags);

#endif	/* __KERN_PHYSMEM_H__ */
