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

	console_printf("[fsmc] Enable FSMC peripheral\n");

	/* + Enable GPIOB, C, D, E, F, G interfaces */
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOB, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOC, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOD, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOE, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOF, true);
	stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOG, true);

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

	/* clock enable command */
	console_printf("[fsmc] Clock enable command\n");
	stm32f429_hw_fmc_send_clock_enable();

	/* PALL command */
	stm32f429_hw_fmc_send_command(0x0a);

	/* Auto-refresh command */
	stm32f429_hw_fmc_send_command(0x6b);

	/* MRD register program */
	stm32f429_hw_fmc_send_command(0x4620c);

	/* Set refresh count */
	stm32f429_hw_fmc_set_refresh_count(0x056a);

	/* Disable write protection */
	stm32f429_hw_fmc_disable_write_protection();

	/*
	 * Now, test the SDRAM between
	 * 0xc0000000 -> 0xcfffffff
	 * 0xd0000000 -> 0xdfffffff
	 *
	 * (8MB on this board, right?)
	 */
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


