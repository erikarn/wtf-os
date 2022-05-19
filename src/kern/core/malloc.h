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
#ifndef	__KERN_CORE_MALLOC_H__
#define	__KERN_CORE_MALLOC_H__

extern	void * kern_malloc(size_t size, uint32_t alignment);
extern	void * kern_malloc_nonzero(size_t size, uint32_t alignment);
extern	void * kern_realloc(void *ptr, size_t size);
extern	void kern_free(void *ptr);

#endif	/* __KERN_CORE_MALLOC_MALLOC_H__ */
