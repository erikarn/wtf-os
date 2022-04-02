
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <os/reg.h>
#include <os/bit.h>
#include <os/bitmask.h>

#include <hw/arm_systick_defs.h>

#include <core/arm_m4_systick.h>

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
