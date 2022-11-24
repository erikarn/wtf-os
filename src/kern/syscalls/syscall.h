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
#ifndef	__KERN_SYSCALL_H__
#define	__KERN_SYSCALL_H__

/*
 * Write to console: XXX TODO: temporary syscall for debugging
 * before we have device drivers!
 *
 * arg1 - na
 * arg2 - char *
 * arg3 - uint32_t
 * arg4 - na
 */
#define	SYSCALL_ID_CONSOLE_WRITE		0x0001
extern	syscall_retval_t kern_syscall_putsn(syscall_arg_t arg1,
	    syscall_arg_t arg2, syscall_arg_t arg3, syscall_arg_t arg4);

/*
 * Sleep for n milliseconds
 *
 * arg1 - na
 * arg2 - uint32_t milliseconds
 * arg3 - na
 * arg4 - na
 */
#define	SYSCALL_ID_CONSOLE_SLEEP		0x0002

extern	syscall_retval_t kern_syscall_sleep(syscall_arg_t arg1,
	    syscall_arg_t arg2, syscall_arg_t arg3, syscall_arg_t arg4);

#define	SYSCALL_ID_CONSOLE_WAIT			0x0003

/*
 * Exit the current userland task.
 *
 * arg1 - uint32_t exit status
 * arg2 - na
 * arg3 - na
 * arg4 - na
 */
#define	SYSCALL_ID_TASK_EXIT			0x0004
extern	syscall_retval_t kern_syscall_exit(syscall_arg_t arg1,
	    syscall_arg_t arg2, syscall_arg_t arg3, syscall_arg_t arg4);


extern	syscall_retval_t kern_syscall_handler(syscall_arg_t arg1,
	    syscall_arg_t arg2, syscall_arg_t arg3, syscall_arg_t arg4);

#endif	/* __KERN_SYSCALL_H__ */
