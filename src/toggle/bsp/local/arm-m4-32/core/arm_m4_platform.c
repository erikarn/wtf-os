#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <core/platform.h>
#include <core/arm_m4_nvic.h>
#include <core/arm_m4_systick.h>
#include <hw/types.h>
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

	console_printf("%s: called\n", __func__);

	console_printf("%s: _estack=0x%x\n", __func__, &_estack);
	console_printf("%s: _data=0x%x -> 0x%x\n", __func__, &_sdata, &_edata);
	console_printf("%s: _bss=0x%x -> 0x%x\n", __func__, &_sbss, &_ebss);

	/* Setup interrupt controller */
	arm_m4_nvic_init();
	/* and system tick controller */
	arm_m4_systick_init();
}

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
