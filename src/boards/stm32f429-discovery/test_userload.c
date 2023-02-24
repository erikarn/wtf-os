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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "bsp/local/os/reg.h"
#include "bsp/local/stm32f4/stm32f429_hw_flash.h"
#include "bsp/local/stm32f4/stm32f429_hw_usart.h"
#include "bsp/local/stm32f4/stm32f429_hw_rcc.h"
#include "bsp/local/stm32f4/stm32f429_hw_rcc_defs.h"
#include "bsp/local/stm32f4/stm32f429_hw_gpio.h"
#include "bsp/local/stm32f4/stm32f429_hw_exti.h"
#include "bsp/local/stm32f4/stm32f429_hw_syscfg.h"

#include "hw/types.h"

#include "kern/console/console.h"
#include "kern/core/task.h"
#include "kern/core/timer.h"
#include "kern/core/physmem.h"
#include "kern/core/malloc.h"
#include "kern/core/task_mem.h"
#include "kern/user/user_exec.h"

/* flash resource */
#include "kern/flash/flash_resource.h"

/* resource paks */
#include "kern/flash/flash_resource_header.h"
#include "kern/flash/flash_resource_pak.h"

#include "core/platform.h"
#include "core/lock.h"
#include "core/arm_m4_systick.h"
#include "core/arm_m4_nvic.h"
#include "core/arm_m4_mpu.h"

extern flash_resource_span_t flash_span;

void
test_userload(void)
{
    struct flash_resource_pak pak;
    struct task_mem tm;
    paddr_t kern_stack;
    struct kern_task *task;

    /* Ok, let's try loading TEST.BIN */
    /*
     * XXX TODO: for now, we aren't going to implement MPU support.
     * Once this actually works, we can figure out how to populate the
     * segments in a non-terrible way, and also figure out how to track
     * all of these memory allocations in the user task struct.
     */
    if (flash_resource_lookup(&flash_span, &pak, "TEST.BIN")) {
        struct user_exec_program_header hdr = { 0 };
        struct user_exec_program_addrs addrs = { 0 };

        console_printf("[wtfos] Found TEST.BIN!\n");

        if (user_exec_program_parse_header(pak.payload_start, pak.payload_size,
          &hdr) == false) {
             console_printf("[wtfos] failed to parse program header\n");
             return;
        }
        console_printf("[wtfos] parsed program header\n");

        /*
         * Header is now parsed out, time to allocate memory for our regions.
         * The copying / populating is done by user_exec_program_setup_segments()
         *
         * (Although right now we're not copying text or rodata, as we're
         * expecting them to be in flash.)
         */
	console_printf("[prog] payload_start=0x%x, len %d bytes\n", pak.payload_start, pak.payload_size);

        /* TEXT if needed, else just point to flash offset - ro, exec */
	console_printf("[prog] text = 0x%x, %d bytes\n", hdr.text_offset, hdr.text_size);
	console_printf("[prog] start = 0x%x\n", hdr.start_offset);

        /* GOT - noexec, ro */
	console_printf("[prog] got = 0x%x, %d bytes\n", hdr.got_offset, hdr.got_size);

        /* BSS - noexec, rw */
	console_printf("[prog] bss = 0x%x, %d bytes\n", hdr.bss_offset, hdr.bss_size);

        /* DATA - noexec, rw */
	console_printf("[prog] data = 0x%x, %d bytes\n", hdr.data_offset, hdr.data_size);

        /* RODATA if needed, else just point to flash offset - ro, noexec */
	console_printf("[prog] rodata = 0x%x, %d bytes\n", hdr.rodata_offset, hdr.rodata_size);

        /* HEAP - noexec, rw */
	console_printf("[prog] heap = %d bytes\n", hdr.heap_size);

        /* STACK - noexec, rw */
	console_printf("[prog] stack = %d bytes\n", hdr.stack_size);

        /*
         * TODO: which segments can we coalesce together to take up less MPU
         * slots?  The STM32F4 only has 8 MPU slots, so we'd end up with
         * only one free for hardware access.
         */

	/* Allocate RAM - not MPU aligned for now */

	kern_task_mem_init(&tm);

	kern_stack = kern_physmem_alloc(512, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);

	/* XIP */
	addrs.text_addr = pak.payload_start + hdr.text_offset;
	addrs.start_addr = pak.payload_start + hdr.start_offset;

	addrs.got_addr = kern_physmem_alloc(hdr.got_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	addrs.bss_addr = kern_physmem_alloc(hdr.bss_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	addrs.data_addr = kern_physmem_alloc(hdr.data_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	/* XIP */
	addrs.rodata_addr = pak.payload_start + hdr.rodata_offset;
	addrs.heap_addr = kern_physmem_alloc(hdr.heap_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);
	addrs.stack_addr = kern_physmem_alloc(hdr.stack_size, 8, KERN_PHYSMEM_ALLOC_FLAG_ZERO);

	/* XIP */
	kern_task_mem_set(&tm, TASK_MEM_ID_TEXT, 0x08000000, 0x200000, false);
	kern_task_mem_set(&tm, TASK_MEM_ID_USER_GOT, addrs.got_addr, hdr.got_size, true);
	kern_task_mem_set(&tm, TASK_MEM_ID_USER_BSS, addrs.bss_addr, hdr.bss_size, true);
	kern_task_mem_set(&tm, TASK_MEM_ID_USER_DATA, addrs.data_addr, hdr.data_size, true);
	/* XIP */
	kern_task_mem_set(&tm, TASK_MEM_ID_USER_RODATA, addrs.rodata_addr, hdr.rodata_size, false);
	kern_task_mem_set(&tm, TASK_MEM_ID_USER_HEAP, addrs.heap_addr, hdr.heap_size, true);
	kern_task_mem_set(&tm, TASK_MEM_ID_USER_STACK, addrs.stack_addr, hdr.stack_size, true);
	kern_task_mem_set(&tm, TASK_MEM_ID_KERN_STACK, kern_stack, 512, true);

	/*
	 * Parse / update relocation entries and other segment offset stuff.
	 */
	if (user_exec_program_setup_segments(pak.payload_start, pak.payload_size,
	    &hdr, &addrs) == false) {
		console_printf("[userload] failed to setup segments\n");
		return;

        }
#if 1
	/*
	 * Here we have the parsed out segments, allocated memory and now
	 * populated them with the relevant contents.
	 *
	 * In theory we can now setup the task, setup r9 with the right GOT base
	 * value, and start execution!
	 */
	task = kern_malloc(sizeof(struct kern_task), 4);
        kern_task_user_init(task, addrs.start_addr, NULL, addrs.got_addr,
            "TEST.BIN",
            &tm,
            TASK_FLAGS_DYNAMIC_STRUCT); // | TASK_FLAGS_ENABLE_MPU);

        /* And start it */
        kern_task_start(task);
#endif
    }
}
