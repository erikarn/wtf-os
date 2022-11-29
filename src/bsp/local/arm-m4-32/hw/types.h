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

#ifndef	__ARM_M4_HW_TYPES_H__
#define	__ARM_M4_HW_TYPES_H__

#include <stdint.h>

/* Note: for M4, there's no virtual address types */

/* Physical address */
typedef uint32_t paddr_t;

/* stack address for kernel/userland */
typedef uint32_t stack_addr_t;

/* user address */
typedef uint32_t uaddr_t;

/* kernel memory for executable code */
typedef uint32_t kern_code_exec_addr_t;
typedef uint32_t kern_code_stack_addr_t;

/* type for saving/restoring IRQ state */
typedef uint32_t irq_save_t;

/* syscall arg field (eg if registers) */
typedef uint32_t syscall_arg_t;

/* syscall return value field (eg if registers) */
typedef uint32_t syscall_retval_t;

/* individual MPU entry field, since for now I only care about MPU, not MMU */
typedef struct {
	uint32_t base_reg;
	uint32_t rasr_reg;
} platform_mpu_phys_entry_t;

#define	PLATFORM_MPU_PHYS_ENTRY_COUNT		8

#endif	/* __ARM_M4_HW_TYPES_H__ */
