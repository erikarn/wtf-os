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

#ifndef	__FLASH_FLASH_RESOURCE_H__
#define	__FLASH_FLASH_RESOURCE_H__

#include "hw/types.h"
#include "os/bit.h"

#define	FLASH_RESOURCE_SPAN_FLAGS_NONE		0x0

/* We've enumerated the span */
#define	FLASH_RESOURCE_SPAN_FLAGS_SETUP		BIT_U32(0)

struct flash_resource_span {
	paddr_t start;
	paddr_t end;
	size_t size;
	uint32_t flags;
};

typedef struct flash_resource_span flash_resource_span_t;
typedef struct flash_resource_pak flash_resource_pak_t;

extern	bool flash_resource_span_init(flash_resource_span_t *span,
	     paddr_t start, size_t size);
extern	bool flash_resource_lookup(flash_resource_span_t *span,
	    flash_resource_pak_t *pak, const char *label);


#endif	/* __FLASH_FLASH_RESOURCE_H__ */
