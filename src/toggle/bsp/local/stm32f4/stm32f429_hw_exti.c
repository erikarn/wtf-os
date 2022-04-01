#include <stdint.h>
#include <stdbool.h>

#include "../os/reg.h"
#include "../os/bit.h"
#include "../os/bitmask.h"

#include "stm32f429_hw_map.h"
#include "stm32f429_hw_exti_reg.h"
#include "stm32f429_hw_exti.h"

void
stm32f429_hw_exti_enable_interrupt(uint32_t exti_intr, bool enable)
{
	uint32_t reg;

	reg = os_reg_read32(EXTI_BASE, STM32F429_HW_EXTI_REG_IMR);
	if (enable) {
		reg |= BIT_U32(exti_intr);
	} else {
		reg &= ~BIT_U32(exti_intr);
	}
	os_reg_write32(EXTI_BASE, STM32F429_HW_EXTI_REG_IMR, reg);
}

void
stm32f429_hw_exti_enable_event(uint32_t exti_intr, bool enable)
{
	uint32_t reg;

	reg = os_reg_read32(EXTI_BASE, STM32F429_HW_EXTI_REG_EMR);
	if (enable) {
		reg |= BIT_U32(exti_intr);
	} else {
		reg &= ~BIT_U32(exti_intr);
	}
	os_reg_write32(EXTI_BASE, STM32F429_HW_EXTI_REG_EMR, reg);
}

void
stm32f429_hw_exti_set_intr_type(uint32_t exti_intr, bool on_rising,
    bool on_falling)
{
	uint32_t rtsr, ftsr;

	rtsr = os_reg_read32(EXTI_BASE, STM32F429_HW_EXTI_REG_RTSR);
	ftsr = os_reg_read32(EXTI_BASE, STM32F429_HW_EXTI_REG_FTSR);

	rtsr &= ~BIT_U32(exti_intr);
	ftsr &= ~BIT_U32(exti_intr);
	if (on_rising) {
		rtsr |= BIT_U32(exti_intr);
	}

	if (on_falling) {
		ftsr |= BIT_U32(exti_intr);
	}

	os_reg_write32(EXTI_BASE, STM32F429_HW_EXTI_REG_RTSR, rtsr);
	os_reg_write32(EXTI_BASE, STM32F429_HW_EXTI_REG_FTSR, ftsr);
}

/**
 * Trigger the given interrupt by software.
 *
 * This triggers the interrupt to happen via a software write, rather
 * than an external interrupt input (eg GPIO.)
 *
 * @param[in] exti_intr external interrupt ID
 */
void
stm32f429_hw_exti_set_swi_trigger(uint32_t exti_intr)
{
	uint32_t reg;

	reg = os_reg_read32(EXTI_BASE, STM32F429_HW_EXTI_REG_SWIER);
	reg |= BIT_U32(exti_intr);
	os_reg_write32(EXTI_BASE, STM32F429_HW_EXTI_REG_SWIER, reg);
}

uint32_t
stm32f429_hw_exti_get_pending_interrupts(void)
{
	return os_reg_read32(EXTI_BASE, STM32F429_HW_EXTI_REG_PR);
}

/**
 * Ack a pending interrupt - hardware or software generated.
 *
 * @param[in] exti_intr external interrupt ID
 */
void
stm32f429_hw_exti_ack_pending_interrupt(uint32_t exti_intr)
{
	uint32_t reg;

	reg = os_reg_read32(EXTI_BASE, STM32F429_HW_EXTI_REG_PR);
	reg |= BIT_U32(exti_intr);
	os_reg_write32(EXTI_BASE, STM32F429_HW_EXTI_REG_PR, reg);
}
