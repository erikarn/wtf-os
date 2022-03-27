#include <stddef.h>

#include <hw/types.h>
#include <hw/scb_defs.h>

#include <os/reg.h>

#include <kern/console/console.h>
#include <kern/core/exception.h>

/**
 * The default interrupt handler.
 */
void
arm_m4_default_interrupt_handler(void)
{
	console_printf("%s: called\n", __func__);
}

/**
 * The default exception handler.
 */
void
arm_m4_default_exception_handler(void)
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
