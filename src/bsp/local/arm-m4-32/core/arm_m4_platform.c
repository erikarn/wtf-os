#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <os/bit.h>
#include <os/reg.h>

#include <core/platform.h>
#include <core/arm_m4_nvic.h>
#include <core/arm_m4_systick.h>
#include <hw/types.h>
#include <hw/scb_defs.h>
#include <asm/asm_defs.h>

#include <kern/console/console.h>

/*
 * These variables are set in the linker and startup .s file.
 * They provide the initial platform memory layout that we will
 * need in order to bootstrap our own setup.
 */

extern	uint32_t _estack;
extern	uint32_t _sdata, _edata;
extern	uint32_t _sbss, _ebss;
extern	uint32_t _sidata;

/**
 * Perform platform CPU initialisation.
 *
 * This performs the platform and CPU initialisation.
 */
void
platform_cpu_init(void)
{
	uint32_t val;

	console_printf("%s: called\n", __func__);

	console_printf("%s: _estack=0x%x\n", __func__, &_estack);
	console_printf("%s: _data=0x%x -> 0x%x\n", __func__, &_sdata, &_edata);
	console_printf("%s: _bss=0x%x -> 0x%x\n", __func__, &_sbss, &_ebss);

	/* Setup interrupt controller */
	arm_m4_nvic_init();
	/* and system tick controller */
	arm_m4_systick_init();
	/* enable lazy FPU saving */

	val = os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_FPCCR);
	val |= ARM_M4_SCB_REG_FPCCR_LSPEN;
	val |= ARM_M4_SCB_REG_FPCCR_ASPEN;
	os_reg_write32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_FPCCR, val);
}

/**
 * Enter an interruptable CPU idle state.
 *
 * This is called during idle loop operation; it'll either exit because it
 * wants to or because an interrupt occurs.
 */
void
platform_cpu_idle(void)
{

	enter_wfi();
}

/**
 * Platform specific IRQ handling.
 *
 * This hides interrupt routing to the underlying IRQ providers.
 *
 * XXX TODO: once this is working, we should just split interrupt identifiers
 * up into something like a pair of 16 bit identifiers or a pair of 32 bit
 * identifiers, rather than the ye olde way of having a flat, large
 * IRQ space.  I've always hated that.
 */

void
platform_irq_enable(uint32_t irq)
{

	arm_m4_nvic_enable_irq(irq);
}

void
platform_irq_disable(uint32_t irq)
{

	arm_m4_nvic_disable_irq(irq);
}

/**
 * Directly enable CPU interrupts, without any tracking of the CPU interrupt
 * mask.
 */
void
platform_cpu_irq_enable(void)
{
	enable_irq();
}

/**
 * Directly disable CPU interrupts, without any tracking of the CPU interrupt
 * mask.
 */
void
platform_cpu_irq_disable(void)
{
	disable_irq();
}

/**
 * Disable IRQs, save the current mask so it's restored to what it was.
 */
irq_save_t
platform_cpu_irq_disable_save(void)
{
	irq_save_t mask;

	mask = (irq_save_t) get_primask();
	disable_irq();
	return (mask);
}

/**
 * Re-enable IRQs using the previous interrupt mask.
 *
 * If interrupts were disabled then they'll stay disabled.
 */
void
platform_cpu_irq_enable_restore(irq_save_t mask)
{

	set_primask(mask);
}

/**
 * A default return routine that just spins.
 *
 * We shouldn't end up here but let's get noticed if we do.
 */
static void
platform_task_exit(void)
{
	console_printf("[task_exit] %s: called! How'd I get here!\n", __func__);

	/* For now */
	while (1)
		;
}

/**
 * Setup the stack for a newly created kernel task.
 *
 * This populates the given stack frame with enough
 * information for the M4 to jump /back/ into the task
 * as if it was a normal context switch.
 *
 * The stack address is decremented first before used, just
 * like the hardware would, so pass in (stack_base+stack_size),
 * which will initially point to the address AFTER the stack RAM.
 *
 * The returned stack address can then be stored in the task struct
 * as if it had been saved into during an exception.
 *
 * @param[in] stack the top of stack, as if this were the SP/MSP.
 * @param[in] entry_point kernel task entry point to jump to
 * @param[in] param argument to pass into r0.
 * @retval stack address as if the hardware had saved execution here
 */
stack_addr_t
platform_task_stack_setup(stack_addr_t stack, void *entry_point, void *param)
{
	stack_addr_t *ps = (void *) (uintptr_t) stack;

	ps--;

	*ps = 0x01000000;		// INITIAL XSPR
	ps--;
	*ps = ((kern_code_exec_addr_t) entry_point) & 0xfffffffe;	// PC: bit 0 must be clear here
	ps--;
	*ps = ((kern_code_exec_addr_t) platform_task_exit);		// LR
	ps = ps - 5;	/* skip r12, r3, r3, r1 */
	*ps = ((kern_code_exec_addr_t) entry_point); // r0
	ps--;
	*ps = 0xfffffffd;		// INITIAL EXC_RETURN
	ps -= 8;	/* skip r11, r10, r9, r8, r7, r6, r5, r4 */

	return (stack_addr_t) (ps);
}

/**
 * Kick off a context switch, either now or soon.
 *
 * This may be called inside a critical section; it's not guaranteed to
 * DO the context switch but it'll do what's needed to do one when
 * no spinlock/critical sections are held.
 */
void
platform_kick_context_switch(void)
{
	arm_m4_exception_set_pendsv();
}

/**
 * Setup the platform timer for the given interval in milliseconds.
 *
 * This requires that the platform timer code has already been
 * configured so this can be called with a millisecond value.
 */
void
platform_timer_set_msec(uint32_t msec)
{

	arm_m4_systick_set_usec(msec * 1000);
}

/**
 * Enable the platform timer.
 *
 * This is free-running and will keep generating interrupts at the
 * programmed rate until it is disabled.
 */
void
platform_timer_enable(void)
{

	arm_m4_systick_start_counting();
}

void
platform_timer_disable(void)
{

	arm_m4_systick_stop_counting();
}
