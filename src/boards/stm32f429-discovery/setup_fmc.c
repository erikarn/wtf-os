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

#include "bsp/local/stm32f4/stm32f429_hw_fmc.h"

/* XXX these two for bring-up */
#include "bsp/local/stm32f4/stm32f429_hw_fmc_reg.h"
#include "bsp/local/stm32f4/stm32f429_hw_rcc_reg.h"
#include "bsp/local/stm32f4/stm32f429_hw_syscfg_reg.h"
#include "bsp/local/stm32f4/stm32f429_hw_map.h"

#include "hw/types.h"

#include "kern/console/console.h"
#include "kern/core/task.h"
#include "kern/core/timer.h"
#include "kern/core/physmem.h"
#include "kern/user/user_exec.h"

/* flash resource */
#include "kern/flash/flash_resource.h"

#include "core/platform.h"
#include "core/lock.h"
#include "core/arm_m4_systick.h"
#include "core/arm_m4_nvic.h"
#include "core/arm_m4_mpu.h"


void
board_stm32f429i_discovery_fmc_init(void)
{
	struct stm32f429_hw_gpio_pin_config cfg;
	bool ret;

	console_printf("%s: syscfg memrmp=0x%x\n",
	    __func__,
	    os_reg_read32(SYSCFG_BASE, STM32F429_HW_SYSCFG_REG_MEMRMP));

	console_printf("[fsmc] Enable FSMC peripheral\n");

	/* + Enable GPIOB, C, D, E, F, G interfaces */
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOB, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOC, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOD, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOE, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOF, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOG, true);

	/*
	 * Take the ones out of reset that aren't at this point
	 *
	 * This should be implemented as a proper driver checking whether it's
	 * in reset before taking them out.
	 */
#if 0
	stm32f429_rcc_peripheral_reset(STM32F429_RCC_PERPIH_GPIOA, false);
	stm32f429_rcc_peripheral_reset(STM32F429_RCC_PERPIH_GPIOB, false);
	stm32f429_rcc_peripheral_reset(STM32F429_RCC_PERPIH_GPIOC, false);
	stm32f429_rcc_peripheral_reset(STM32F429_RCC_PERPIH_GPIOD, false);
	stm32f429_rcc_peripheral_reset(STM32F429_RCC_PERPIH_GPIOE, false);
	stm32f429_rcc_peripheral_reset(STM32F429_RCC_PERPIH_GPIOF, false);
	stm32f429_rcc_peripheral_reset(STM32F429_RCC_PERPIH_GPIOG, false);
#endif

	/* + GPIO config + alt mode for SDRAM */

	stm32f429_hw_gpio_config_init(&cfg);
	cfg.mode = STM32F429_HW_GPIO_PIN_MODE_ALTERNATE;
	cfg.speed = STM32F429_HW_GPIO_PIN_SPEED_HIGH;
	cfg.out_type = STM32F429_HW_GPIO_PIN_OUTPUT_TYPE_PUPD;
	cfg.out_pupd = STM32F429_HW_GPIO_PIN_PUPD_NONE;

	/* GPIO B */
	/* Note: mode, pupd, speed includes pins PB3,PB4 too, but they're not
	 * routed to the SDRAM controller? And ALT func is set to 0 for those? */
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOB, 5, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOB, 5, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOB, 6, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOB, 6, &cfg);

	/* GPIO C */
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOC, 0, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOC, 0, &cfg);

	/* GPIO D */
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 0, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 0, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 1, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 1, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 8, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 8, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 9, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 9, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 10, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 10, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 14, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 14, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 15, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOD, 15, &cfg);

	/* GPIO E */
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 0, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 0, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 1, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 1, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 7, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 7, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 8, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 8, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 9, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 9, &cfg);

	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 10, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 10, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 11, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 11, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 12, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 12, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 13, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 13, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 14, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 14, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 15, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOE, 15, &cfg);

	/* GPIO F */
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 0, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 0, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 1, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 1, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 2, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 2, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 3, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 3, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 4, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 4, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 5, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 5, &cfg);

	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 11, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 11, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 12, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 12, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 13, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 13, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 14, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 14, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 15, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOF, 15, &cfg);

	/* GPIO G */
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 0, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 0, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 1, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 1, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 4, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 4, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 5, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 5, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 8, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 8, &cfg);
	stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 15, 0x0c);
	stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 15, &cfg);

	/* + FMC config in RCC - enables clock */
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_FSMC, true);

	console_printf("%s: ahb1 rstr=0x%x, enr=0x%x\n",
	    __func__,
	    os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_AHB1RSTR),
	    os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_AHB1ENR));
	console_printf("%s: ahb3 rstr=0x%x, enr=0x%x\n",
	    __func__,
	    os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_AHB3RSTR),
	    os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_AHB3ENR));

	/*
	 * The rest of this should be done in the FMC driver
	 * with a bank config struct.  However for now just bootstrap and
	 * verify!
	 */
	 os_reg_write32(FMC_R_BASE, FMC_REG_SDCR1, 0x00002ED0);
	 os_reg_write32(FMC_R_BASE, FMC_REG_SDCR2, 0x000001D4);
	 os_reg_write32(FMC_R_BASE, FMC_REG_SDTR1, 0x0F1F6FFF);
	 os_reg_write32(FMC_R_BASE, FMC_REG_SDTR2, 0x01010361);

	/* + Sending SDRAM init commands */
	console_printf("%s: SDCR1=0x%x, SDCR2=0x%x, SDTR1=0x%x, SDTR2=0x%x\n",
	    __func__,
	    os_reg_read32(FMC_R_BASE, FMC_REG_SDCR1),
	    os_reg_read32(FMC_R_BASE, FMC_REG_SDCR2),
	    os_reg_read32(FMC_R_BASE, FMC_REG_SDTR1),
	    os_reg_read32(FMC_R_BASE, FMC_REG_SDTR2));

	/* clock enable command */
	console_printf("[fsmc] Clock enable command\n");
	ret = stm32f429_hw_fmc_send_clock_enable();
	if (! ret) {
		console_printf("[fsmc] clock enable failed!\n");
	}

	/* PALL command */
	ret = stm32f429_hw_fmc_send_command(0x0a);
	if (! ret) {
		console_printf("[fsmc] PALL command failed!\n");
	}

	/* Auto-refresh command */
	ret = stm32f429_hw_fmc_send_command(0x6b);
	if (! ret) {
		console_printf("[fsmc] Auto refresh command failed!\n");
	}

	/* MRD register program */
	ret = stm32f429_hw_fmc_send_command(0x4620c);
	if (! ret) {
		console_printf("[fsmc] MRD register command failed!\n");
	}

	/* Set refresh count */
	stm32f429_hw_fmc_set_refresh_count(0x056a);

	/* Disable write protection */
	stm32f429_hw_fmc_disable_write_protection();
	console_printf("[fsmc] Ready, I think?\n");

	/*
	 * Now, test the SDRAM between
	 * bank1: 0xc0000000 -> 0xcfffffff
	 * bank2: 0xd0000000 -> 0xdfffffff
	 *
	 * The commands have bit 3 (CTB2) to enable bank 2,
	 * my guess is that this is in bank two at D0000000
	 * for 8MB.
	 *
	 * (8MB on this board, right?)
	 * c0000000 -> c3fffffff
	 */
	char *buf = (void *) 0xd0000000;

	for (int i = 0; i < 8 * 1024 * 1024; i++) {
		buf[i] = i % 256;
	}
	for (int i = 0; i < 8 * 1024 * 1024; i++) {
		if (buf[i] != (i % 256)) {
			console_printf("%s: 0x%x: got 0x%x expected 0x%x\n",
			    __func__,
			    i,
			    buf[i],
			    (i % 256));
		}
	}
}

/*
 * TODO item for the rest of it
 *
 * + Enable GPIOB, C, D, E, F, G interfaces
 * + GPIO config + alt mode for SDRAM
 * + FMC config in RCC
 * + FMC configuration for banks
 * + Sending SDRAM init commands
 * + PALL
 * + Auto refresh
 * + MRD register
 * + Refresh count
 * + Disable write protection
 * + and how to test it!
 */


