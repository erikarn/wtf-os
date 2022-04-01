#include <stdint.h>

#include "../os/reg.h"
#include "../os/bit.h"
#include "../os/bitmask.h"

#include "stm32f429_hw_map.h"
#include "stm32f429_hw_syscfg_reg.h"

/**
 * Configure the given external interrupt routing.
 * Each mapping register (EXTICR{1,2,3,4}) map whether
 * the GPIO pin at the same offset are mapped to trigger
 * it (if it's an input pin.)
 *
 * Eg EXTICR1, field EXTI0, is a 4 bit field which indicates
 * which GPIO pin from GPIOA0, GPIOB0, GPIOC0, etc map to
 * EXTI0.  Field EXTI1 is a 4 bit field which indicates
 * which GPIO pin from GPIOA1, GPIOB1, GPIOC1, etc map to
 * EXTI1.
 *
 * So:
 *
 * + There's no "don't map", the power-on default for these
 *   registers is all 0x0, so they map GPIOA0..15 to EXTI0..15.
 * + You can't just map random pins to random EXTI inputs.
 * + GPIOK pins 8-15 aren't available to use as GPIO EXTI mux
 *   inputs. (9.3.5, 9.3.6, SYSCFG_EXTICR3 and SYSCFG_EXTICR4.)
 */

void
stm32f429_hw_syscfg_config_exti_mux(uint32_t exti_field,
    stm32f429_hw_syscfg_exti_block_gpio_t gpio_block)
{
	uint32_t reg, val;

	/* Only 0..15 */
	if (exti_field > 15) {
		return;
	}

	/* Figure out the register offset */
	reg = STM32F429_HW_SYSCFG_REG_EXTICR1 + ((exti_field / 4) * 4);

	/* Program in the mux value */
	val = os_reg_read32(SYSCFG_BASE, reg);
	val &= ~ (0xf << (exti_field & 0x3));
	val |= ((gpio_block & 0xf) << (exti_field & 0x3));
	os_reg_write32(SYSCFG_BASE, reg, val);
}
