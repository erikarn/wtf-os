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

#include "core/platform.h"
#include "core/lock.h"
#include "core/arm_m4_systick.h"
#include "core/arm_m4_nvic.h"

/* XXX */
extern void arm_m4_task_switch();

static void
cons_putc(char c)
{
	stm32f429_uart_tx_byte(c);
}

static void
cons_flush(void)
{
}

/* Console ops for this platform */
static struct console_ops c_ops = {
	.putc_fn = cons_putc,
	.flush_fn = cons_flush,
};

static void
setup_usart1_gpios(void)
{
    struct stm32f429_hw_gpio_pin_config cfg;

    // Enable clock for GPIOA
    stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOA, true);


    /* XXX TODO: 0x07 == GPIO_AF_USART1 */
    stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOA, 9, 0x07);
    stm32f429_hw_gpio_alternate_set(STM32F429_HW_GPIO_BLOCK_GPIOA, 10, 0x07);

    stm32f429_hw_gpio_config_init(&cfg);
    cfg.mode = STM32F429_HW_GPIO_PIN_MODE_ALTERNATE;
    cfg.speed = STM32F429_HW_GPIO_PIN_SPEED_MED;
    cfg.out_type = STM32F429_HW_GPIO_PIN_OUTPUT_TYPE_PUPD;
    cfg.out_pupd = STM32F429_HW_GPIO_PIN_PUPD_PULL_UP;

    stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOA, 9, &cfg);
    stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOA, 10, &cfg);
}

static void
setup_usart1(void)
{
    uint32_t pclk2;

    /*
     * Enable clock for USART1 peripheral.
     */
    stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_USART1, true);
    pclk2 = stm32f429_rcc_get_pclk2_freq();

    /* USART1 setup itself, it's on APB2 */
    stm32f429_uart_init(115200, pclk2);
}

/* Setup LED GPIOs */
static void
setup_led_gpios(void)
{
    struct stm32f429_hw_gpio_pin_config cfg;

    // Enable caches when fetching to/from flash
    stm32f429_hw_flash_enable_icache();
    stm32f429_hw_flash_enable_dcache();

    // Setup GPIO pins for LEDs
    stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOG, true);

    stm32f429_hw_gpio_config_init(&cfg);
    cfg.mode = STM32F429_HW_GPIO_PIN_MODE_OUTPUT;
    cfg.speed = STM32F429_HW_GPIO_PIN_SPEED_LOW;
    cfg.out_type = STM32F429_HW_GPIO_PIN_OUTPUT_TYPE_PUPD;
    cfg.out_pupd = STM32F429_HW_GPIO_PIN_PUPD_NONE;

    stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 13, &cfg);
    stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOG, 14, &cfg);
}

static void
setup_button_gpios(void)
{
    struct stm32f429_hw_gpio_pin_config cfg;

    // Setup GPIO pin for button input
    stm32f429_rcc_peripheral_enable(STM32F429_RCC_PERPIH_GPIOA, true);

    stm32f429_hw_gpio_config_init(&cfg);
    cfg.mode = STM32F429_HW_GPIO_PIN_MODE_INPUT;
    cfg.speed = STM32F429_HW_GPIO_PIN_SPEED_LOW;
    cfg.out_type = STM32F429_HW_GPIO_PIN_OUTPUT_TYPE_PUPD;
    cfg.out_pupd = STM32F429_HW_GPIO_PIN_PUPD_PULL_DOWN;

    stm32f429_hw_gpio_config_set(STM32F429_HW_GPIO_BLOCK_GPIOA, 0, &cfg);
}

/**
 * USART1 interrupt handler, overrides the weak symbol in the startup .s
 * file.
 *
 * In theory this should come in at supervisor level.
 */
void
USART1_IRQHandler(void)
{
	int16_t r;
	stm32f429_uart_interrupt();

	r = stm32f429_uart_try_read();
	if (r > -1) {
		console_input(r);
	}
}

/**
 * External interrupt 0, used for GPIOA0 and the input button press.
 */
void
EXTI0_IRQHandler(void)
{
	console_printf("[button] pressed!\n");
	stm32f429_hw_exti_ack_pending_interrupt(0);
	stm32f429_hw_gpio_toggle_pin(STM32F429_HW_GPIO_BLOCK_GPIOG, 13);
	stm32f429_hw_gpio_toggle_pin(STM32F429_HW_GPIO_BLOCK_GPIOG, 14);
}

void
SysTick_Handler(void)
{
	//console_printf("[systick] triggered!\n");
	kern_timer_tick();
	kern_task_tick();
}

__attribute__((naked)) void
PendSV_Handler(void)
{
	asm("b arm_m4_task_switch");
}

int
main(void)
{

    /* Setup initial hardware before we setup console, echo etc */

    setup_led_gpios();
    setup_button_gpios();

    setup_usart1_gpios();
    setup_usart1();

    /* console initialisation */
    console_init();
    console_set_ops(&c_ops);

    console_puts("\n");
    console_printf("[wtfos] Welcome to wtf-os!\n");

    platform_cpu_init();

    /* do post CPU init interrupt enable for things like USART */
    /* (yeah a hack for now) */
    stm32f429_uart_enable_rx_intr();

    // Systick setup, so we can generate tick events
    arm_m4_systick_set_hclk_freq(stm32f429_get_system_core_clock());

    console_printf("[wtfos] calculated core freq=%d MHz\n",
        stm32f429_get_system_core_clock());
    console_printf("[wtfos] calculated pclk1 freq=%d MHz\n",
        stm32f429_rcc_get_pclk1_freq());
    console_printf("[wtfos] calculated pclk2 freq=%d MHz\n",
        stm32f429_rcc_get_pclk2_freq());
    console_printf("[wtfos] tenms systick = 0x%08x\n",
        arm_m4_systick_get_tenms_calib());

    /* Set this pin high so we get toggling LEDs */
    stm32f429_hw_gpio_toggle_pin(STM32F429_HW_GPIO_BLOCK_GPIOG, 13);

    /*
     * Do some external interrupt experimenting.
     *
     * Use PA0 -> EXTINT_0, set it up as triggering on rising edge only.
     */

    /* Configure EXTI 0 to be GPIOA0 */
    stm32f429_hw_syscfg_config_exti_mux(0,
        STM32F429_HW_SYSCTL_EXTI_BLOCK_GPIOA);
    /* Configure EXTI0 interrupt for rising edge only */
    stm32f429_hw_exti_set_intr_type(0, true, false);
    /* Enable EXTI0 interrupt in the EXTI block */
    stm32f429_hw_exti_enable_interrupt(0, true);
    /* Enable EXTI0 interrupt in the NVIC block (irq 6) */
    platform_irq_enable(6);

    // Enable CPU interrupts
    platform_cpu_irq_enable();

    /*
     * Setup the timer infra; 1000 milliseconds for now.
     *
     * The timer infra will start with the timer itself
     * disabled; it'll be enabled below when we're
     * ready to context switch.
     */
    kern_timer_init();
    kern_timer_set_tick_interval(1000);
    arm_m4_systick_enable_interrupt(true);

    // Start the kernel timer for now; later on it'll be started
    // by the task / scheduler / timer code if we have any work to do.
    kern_timer_start();

    /* Setup task system, idle task; test tasks, etc but not run them */
    kern_task_setup();

    /* Ready to start context switching */
    kern_task_ready();

    /* Kick start context switching */
    kern_task_tick();

    // Now idle, we should either not get here, or not stay here long
    while (1) {
        platform_cpu_idle();
    }
}
