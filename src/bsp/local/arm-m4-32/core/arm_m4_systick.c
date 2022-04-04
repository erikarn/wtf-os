
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <os/reg.h>
#include <os/bit.h>
#include <os/bitmask.h>

#include <hw/arm_systick_defs.h>

#include <core/arm_m4_systick.h>

#include <kern/core/exception.h>
#include <kern/console/console.h>

static uint32_t systick_hclk_freq = 0;

/**
 * Perform any cortex-M4 specific NVIC platform initialisation.
 */
void
arm_m4_systick_init(void)
{

	/* Disable interrupts, set source to AHB/8, disable clock */
	os_reg_write32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CTRL, 0);
}

/**
 * Enable or disable generating interrupts.
 */
void
arm_m4_systick_enable_interrupt(bool enable)
{
	uint32_t val;

	val = os_reg_read32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CTRL);
	if (enable) {
		val |= ARM_SYSTICK_REG_STK_CTRL_TICKINT;
	} else {
		val &= ~ARM_SYSTICK_REG_STK_CTRL_TICKINT;
	}
	os_reg_write32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CTRL, val);
}

/**
 * Program in the given value and start counting.
 */
void
arm_m4_systick_set_counter_and_start(uint32_t counter_val)
{
	uint32_t val;

	/* Stop counting */
	val = os_reg_read32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CTRL);
	val &= ~ARM_SYSTICK_REG_STK_CTRL_ENABLE;
	os_reg_write32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CTRL, val);

	/* Program reload value */
	val = RMW(0, ARM_SYSTICK_REG_STK_LOAD_RELOAD, counter_val);
	os_reg_write32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_LOAD, val);

	/* Clear current value */
	os_reg_write32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_VAL, 0);

	/* Enable counting */
	val = os_reg_read32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CTRL);
	val |= ARM_SYSTICK_REG_STK_CTRL_ENABLE;
	os_reg_write32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CTRL, val);
}

void
arm_m4_systick_set_usec_and_start(uint32_t usec_val)
{
	uint64_t tmp;

	if (systick_hclk_freq == 0) {
		exception_panic("%s: systick_hclk_freq=0");
	}

	tmp = (uint64_t) usec_val * (uint64_t) systick_hclk_freq;
	tmp = tmp / 1000000ULL;

	/* Clamp at uint32_t */
	if (tmp > 0x0ffffffffL)
		tmp = 0xffffffff;

	arm_m4_systick_set_counter_and_start((uint32_t) tmp);
}

/**
 * Stop counting.
 */
void
arm_m4_systick_stop_counting(void)
{
	uint32_t val;

	val = os_reg_read32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CTRL);
	val &= ~ARM_SYSTICK_REG_STK_CTRL_ENABLE;
	os_reg_write32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CTRL, val);
}

/**
 * Read the calibrated value for 10ms clock ticks @ AHB/8.
 */
uint32_t
arm_m4_systick_get_tenms_calib(void)
{
	uint32_t val;

	val = os_reg_read32(ARM_SYSTICK_BASE, ARM_SYSTICK_REG_STK_CALIB);
	return MS(val, ARM_SYSTICK_REG_STK_CALIB_TENMS);
}

void
arm_m4_systick_set_hclk_freq(uint32_t hclk_freq)
{
	systick_hclk_freq = hclk_freq;
}
