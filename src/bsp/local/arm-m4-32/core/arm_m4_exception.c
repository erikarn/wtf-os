#include <stddef.h>

#include <hw/types.h>
#include <hw/scb_defs.h>

#include <os/reg.h>
#include <core/platform.h>

#include <kern/console/console.h>
#include <kern/core/exception.h>


/**
 * The default interrupt handler.
 *
 * This disables the interrupt to avoid interrupt storms.
 */
void
_arm_m4_default_interrupt_handler(void)
{
	uint32_t ipsr;

	ipsr = get_ipsr() - 16;

	console_printf("[exception] unhandled NVIC IRQ (%d), disabling\n",
	    ipsr);

	/*
	 * Map IPSR to an actual NVIC IRQ.
	 *
	 * XXX TODO: hard-coded constant above!
	 */
	platform_irq_disable(ipsr);
}

/**
 * The default exception handler.
 */
void
_arm_m4_default_exception_handler(void)
{
	uint32_t ipsr, apsr;

	exception_start();

	console_printf("[exception] oh no what happened!\n");

	ipsr = get_ipsr();
	apsr = get_apsr();

	console_printf("[exception] IPSR=0x%x APSR=0x%x\n", ipsr, apsr);

	/*
	 * For now just dump out all the registers of interest without
	 * any decoding, explanation.  Worry about that later.
	 */
	console_printf("[exception] ACTLR=0x%x, ICSR=0x%x, SCR=0x%x, CCR=0x%x\n",
	    os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_ACTLR),
	    os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_ICSR),
	    os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_SCR),
	    os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_CCR));

	console_printf("[exception] CFSR=0x%x, MMAR=0x%x, BFAR=0x%x, AFSR=0x%x\n",
	    os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_CFSR),
	    os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_MMAR),
	    os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_BFAR),
	    os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_AFSR));

	/*
	 * It'd be nice to dump out the stack frame too, wouldn't it?
	 * With all the registers that were saved?
	 */

	exception_spin();
}


/**
 * Set the pending SV bit to trigger an SV exception.
 *
 * This must be called with interrupts disabled so it can be done atomically.
 */
void
arm_m4_exception_set_pendsv(void)
{
	uint32_t val;

	val = os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_ICSR);
	val |= ARM_M4_SCB_REG_ICSR_PENDSVSET;
	os_reg_write32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_ICSR, val);
}

/**
 * Syscall entry point from arm_m4_svc.S.
 */
uint32_t
arm_m4_c_syscall_handler(uint32_t arg1, uint32_t arg2, uint32_t arg3,
    uint32_t arg4)
{
	console_printf("[syscall] called! 0x%x 0x%x 0x%x 0x%x\n",
	    arg1, arg2, arg3, arg4);
	return (0);
}
