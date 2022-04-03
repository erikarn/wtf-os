#include <stddef.h>
#include <stdint.h>

#include <os/reg.h>
#include <os/bit.h>

#include <hw/arm_nvic_defs.h>

#include <core/arm_m4_nvic.h>

/*
 * This is the cortex M4 NVIC (nested vectored interrupt controller) support.
 *
 * Interrupts are numbered from 0 .. ARM_M4_NVIC_NUM_INTERRUPTS - 1.
 */

/**
 * Perform any cortex-M4 specific NVIC platform initialisation.
 */
void
arm_m4_nvic_init(void)
{
}

/**
 * Enable the given IRQ.
 *
 * @param[in] irq NVIC IRQ
 */
void
arm_m4_nvic_enable_irq(uint32_t irq)
{
	uint32_t ro, vo;

	if (irq >= ARM_M4_NVIC_NUM_INTERRUPTS) {
		return;
	}

	ro = irq / 32;
	vo = irq % 32;

	os_reg_write32(ARM_M4_NVIC_REG_BASE, ARM_M4_NVIC_REG_ISER0 + (ro * 4),
	    BIT_U32(vo));
}

/**
 * Disable the given IRQ.
 *
 * @param[in] irq NVIC IRQ
 */
void
arm_m4_nvic_disable_irq(uint32_t irq)
{
	uint32_t ro, vo;

	if (irq >= ARM_M4_NVIC_NUM_INTERRUPTS) {
		return;
	}

	ro = irq / 32;
	vo = irq % 32;

	os_reg_write32(ARM_M4_NVIC_REG_BASE, ARM_M4_NVIC_REG_ICER0 + (ro * 4),
	    BIT_U32(vo));
}

/**
 * Set the IRQ as pending.
 *
 * @param[in] irq NVIC IRQ
 */
void
arm_m4_nvic_set_pending_irq(uint32_t irq)
{
	uint32_t ro, vo;

	if (irq >= ARM_M4_NVIC_NUM_INTERRUPTS) {
		return;
	}

	ro = irq / 32;
	vo = irq % 32;

	os_reg_write32(ARM_M4_NVIC_REG_BASE, ARM_M4_NVIC_REG_ISPR0 + (ro * 4),
	    BIT_U32(vo));
}

/**
 * Clear the IRQ pending status.
 *
 * @param[in] irq NVIC IRQ
 */
void
arm_m4_nvic_clear_pending_irq(uint32_t irq)
{
	uint32_t ro, vo;

	if (irq >= ARM_M4_NVIC_NUM_INTERRUPTS) {
		return;
	}

	ro = irq / 32;
	vo = irq % 32;

	os_reg_write32(ARM_M4_NVIC_REG_BASE, ARM_M4_NVIC_REG_ICPR0 + (ro * 4),
	    BIT_U32(vo));
}

/**
 * Get the pending IRQ status.
 *
 * @param[in] irq NVIC IRQ
 * @retval 0 if not pending, >0 if pending
 */
uint32_t
arm_m4_nvic_get_pending_irq(uint32_t irq)
{
	uint32_t ro, vo, reg;

	if (irq >= ARM_M4_NVIC_NUM_INTERRUPTS) {
		return (0);
	}

	ro = irq / 32;
	vo = irq % 32;

	reg = os_reg_read32(ARM_M4_NVIC_REG_BASE,
	    ARM_M4_NVIC_REG_IABR0 + (ro * 4));

	return (!! (reg & (BIT_U32(vo))));
}

/**
 * Set the IRQ priority.
 */
void
arm_m4_nvic_set_priority(uint32_t irq, uint8_t priority)
{
	uint32_t ro, vo, val;

	if (irq >= ARM_M4_NVIC_NUM_INTERRUPTS) {
		return;
	}

	ro = irq / 4;
	vo = irq % 4;

	/*
	 * Each field is 8 bits wide; there's four to each
	 * register.
	 */
	val = os_reg_read32(ARM_M4_NVIC_REG_BASE,
	    ARM_M4_NVIC_REG_IPR0 + (ro * 4));
	val |= ((priority & 0xff) << (vo * 8));
	os_reg_write32(ARM_M4_NVIC_REG_BASE,
	    ARM_M4_NVIC_REG_IPR0 + (ro * 4), val);

}

/**
 * Get the IRQ priority.
 */
uint8_t
arm_m4_nvic_get_priority(uint32_t irq)
{
	uint32_t ro, vo, val;

	if (irq >= ARM_M4_NVIC_NUM_INTERRUPTS) {
		return (0);
	}

	ro = irq / 4;
	vo = irq % 4;

	/*
	 * Each field is 8 bits wide; there's four to each
	 * register.
	 */
	val = os_reg_read32(ARM_M4_NVIC_REG_BASE,
	    ARM_M4_NVIC_REG_IPR0 + (ro * 4));

	return ((val >> (vo * 8)) & 0xff);

	return (0);
}
