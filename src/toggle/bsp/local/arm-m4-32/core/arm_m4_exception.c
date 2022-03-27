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
	uint32_t ipsr;

	exception_start();

	console_printf("[exception] oh no what happened!\n");

	ipsr = os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_ICSR);

	console_printf("[exception] IPSR=0x%x\n", ipsr);

	exception_spin();
}
