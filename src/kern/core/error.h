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
#ifndef	__KERN_CORE_ERROR_H__
#define	__KERN_CORE_ERROR_H__

typedef uint32_t kern_error_t;

#define	KERN_ERR_OK		0x0
#define	KERN_ERR_NOMEM		0x1
#define	KERN_ERR_NOSPC		0x2
#define	KERN_ERR_INVALID_ARGS	0x3
#define	KERN_ERR_EMPTY		0x4
#define	KERN_ERR_SHUTDOWN	0x5
#define	KERN_ERR_EXISTS		0x6
#define	KERN_ERR_INPROGRESS	0x7
#define	KERN_ERR_INVALID_TASKID	0x8
#define	KERN_ERR_UNIMPLEMENTED	0x9
#define	KERN_ERR_TOOBIG		0xa

#endif	/* __KERN_CORE_ERROR_H__ */
