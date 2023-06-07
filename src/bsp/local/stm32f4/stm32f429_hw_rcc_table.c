
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <os/reg.h>
#include <os/bitmask.h>
#include <hw/scb_defs.h>

#include <stm32f4/stm32f429_hw_map.h>
#include <stm32f4/stm32f429_hw_rcc_reg.h>

#include <stm32f4/stm32f429_hw_rcc_table.h>

/**
 * A table of the peripherals and their reset/enable/clock control.
 *
 * This should likely live at the top of the stm32f429 BSP so we can
 * put other interesting tidbits in it, but for now here's where it's
 * at.
 *
 * Note that it's not indexed at all; this is to avoid issues where
 * someone (notably me) may add entries in the middle of enums later
 * and I don't want to have things go sideways.  Yeah it means it's
 * an O(n) lookup.  If that's a problem for you then you can just
 * reimplement the lookup routines contained herein.
 */

static const struct stm32f429_rcc_table_entry periph_table[] = {
	/* AHB1 */
	{ .periph = STM32F429_RCC_PERPIH_GPIOA,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOA_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOA_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOB,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOB_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOB_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOC,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOC_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOC_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOD,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOD_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOD_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOE,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOE_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOE_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOF,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOF_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOF_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOG,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOG_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOG_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOH,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOH_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOH_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOI,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOI_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOI_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOJ,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOJ_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOJ_EN },

	{ .periph = STM32F429_RCC_PERPIH_GPIOK,
	  .bus = STM32F429_RCC_BUS_AHB1,
	  .clock = STM32F429_RCC_CLOCK_AHB1,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB1RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOK_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB1ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB1ENR_GPIOK_EN },

	/* AHB2 */

	/* AHB3 */
	{
	  .periph = STM32F429_RCC_PERPIH_FSMC,
	  .bus = STM32F429_RCC_BUS_AHB3,
	  .clock = STM32F429_RCC_CLOCK_HCLK,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_AHB3RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_AHB3RSTR_FMC_RST,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_AHB3ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_AHB3ENR_FMC_EN,
	},

	/* APB1 */


	/* APB2 */
	{ .periph = STM32F429_RCC_PERPIH_USART1,
	  .bus = STM32F429_RCC_BUS_APB2,
	  .clock = STM32F429_RCC_CLOCK_PCLK2,
	  .rcc_reset_reg = STM32F429_RCC_REG_RCC_APB2RSTR,
	  .rcc_reset_mask = STM32F429_RCC_REG_RCC_APB2RSTR_USART1,
	  .rcc_clken_reg = STM32F429_RCC_REG_RCC_APB2ENR,
	  .rcc_clken_mask = STM32F429_RCC_REG_RCC_APB2ENR_USART1 },
};

const struct stm32f429_rcc_table_entry *
stm32f429_rcc_table_get_peripheral(stm32f429_rcc_peripheral_name_t periph)
{
	int i;

	/* XXX TODO; yeah I need an ARRAY_LENGTH somewhere */
	for (i = 0; i < sizeof(periph_table) / sizeof(periph_table[0]); i++) {
		if (periph == periph_table[i].periph)
			return &periph_table[i];
	}

	return (NULL);
}
