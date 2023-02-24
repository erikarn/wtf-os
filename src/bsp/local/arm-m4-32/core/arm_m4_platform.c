#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <os/bit.h>
#include <os/reg.h>
#include <os/bitmask.h>

#include <core/platform.h>
#include <core/arm_m4_nvic.h>
#include <core/arm_m4_systick.h>
#include <core/arm_m4_mpu.h>
#include <hw/types.h>
#include <hw/scb_defs.h>
#include <hw/arm_mpu_defs.h>
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
 * @param[in] is_user Whether to initialise as a userland task
 * @retval stack address as if the hardware had saved execution here
 */

/*
 * We're using THREAD+PSP, see the switch.S code for why.
 */
#define	MODE_HANDLER_MSP 0xfffffff1
#define	MODE_THREAD_MSP 0xfffffff9
#define	MODE_THREAD_PSP 0xfffffffd

stack_addr_t
platform_task_stack_setup(stack_addr_t stack, void *entry_point, void *param,
    uint32_t r9, bool is_user)
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
	*ps = MODE_THREAD_PSP;		// INITIAL EXC_RETURN
	ps--;

	/*
	 * Correct initialisation of r9 is required for
	 * PIC userland code.
	 */
	ps -= 2;	/* skip r11, r10 */

	*ps = r9;
	ps--;

	ps -= 5;	/* skip r8, r7, r6, r5, r4 */

	if (is_user) {
		*ps = 0x03; // CONTROL, unpriv
	} else {
		*ps = 0x02; // CONTROL, priv
	}

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

void
platform_mpu_enable(void)
{
	arm_m4_mpu_enable();
}

void
platform_mpu_disable(void)
{
	arm_m4_mpu_disable();
}

void
platform_mpu_table_init(platform_mpu_phys_entry_t *table)
{
	int i;
	for (i = 0; i < PLATFORM_MPU_PHYS_ENTRY_COUNT; i++) {
		table[i].base_reg = 0;
		table[i].rasr_reg = 0;
	}
}

/*
 * Initialise this particular slot with the given info.
 *
 * Return false if it can't be programmed in.
 *
 * XXX this is a hack to get MPU programming up and going; will need
 * to make this more .. generic? well designed? I dunno. Later.
 * when this actually works.
 */
bool
platform_mpu_table_set(platform_mpu_phys_entry_t *e, uint32_t base_addr,
    uint32_t size, platform_prot_type_t prot)
{
	int i, sf = 0;
	uint32_t mask, rasr_reg;

	/* none? mark it blank */
	if (prot == PLATFORM_PROT_TYPE_NONE) {
		e->base_reg = 0;
		e->rasr_reg = 0;
		return (true);
	}

	/* minimum size */
	if (size < 32) {
		console_printf("%s: invalid size (%d)!\n", __func__, size);
		return (false);
	}

	/* find the right size */
	for (i = 4; i < 31; i++) {
		if (1 << (i+1) == size) {
			sf = i;
			break;
		}
	}

	/* not valid? whoops */
	if (sf == 0) {
		return (false);
	}

	/* calculate address mask */
	mask = (1 << (sf + 1)) - 1;

	/* check! */
	if ((base_addr & mask) != 0) {
		console_printf("%s: invalid alignment!\n", __func__);
		return (false);
	}

	/* generate rasr - yeah, this should be a table, done at compile time */
	rasr_reg = 0;
	rasr_reg |= RMW(rasr_reg, ARM_M4_MPU_REG_RSAR_SIZE, sf);
	rasr_reg |= RMW(rasr_reg, ARM_M4_MPU_REG_RSAR_SRD, 0);
	rasr_reg |= ARM_M4_MPU_REG_RSAR_ENABLE;
	switch (prot) {
	case PLATFORM_PROT_TYPE_EXEC_RO:
		rasr_reg |= RMW(rasr_reg, ARM_M4_MPU_REG_RSAR_TEX, 0x0);
		rasr_reg |= RMW(rasr_reg, ARM_M4_MPU_REG_RSAR_AP, 0x6);
		rasr_reg |= ARM_M4_MPU_REG_RSAR_C;
		rasr_reg |= ARM_M4_MPU_REG_RSAR_S;
		break;
	case PLATFORM_PROT_TYPE_NOEXEC_RW:
		rasr_reg |= RMW(rasr_reg, ARM_M4_MPU_REG_RSAR_TEX, 0x1);
		rasr_reg |= RMW(rasr_reg, ARM_M4_MPU_REG_RSAR_AP, 0x3);
		rasr_reg |= ARM_M4_MPU_REG_RSAR_B;
		rasr_reg |= ARM_M4_MPU_REG_RSAR_C;
		rasr_reg |= ARM_M4_MPU_REG_RSAR_S;
		rasr_reg |= ARM_M4_MPU_REG_RSAR_XN;
		break;

	default:
		console_printf("%s: invalid prot (%d)\n", __func__, prot);
		return (false);
	}

	/* debug! */
	console_printf("%s: addr=0x%x, size=%d, mask=0x%x, sf=%d, rasr_reg=0x%08x\n",
	    __func__,
	    base_addr,
	    size,
	    mask,
	    sf, rasr_reg);

	e->base_reg = base_addr;
	e->rasr_reg = rasr_reg;

	return (true);
}

/*
 * Program this table into the hardware.
 */
void
platform_mpu_table_program(const platform_mpu_phys_entry_t *table)
{
	int i;
	for (i = 0; i < PLATFORM_MPU_PHYS_ENTRY_COUNT; i++) {
		arm_m4_mpu_program_region(i, table[i].base_reg,
		    table[i].rasr_reg);
	}
}
