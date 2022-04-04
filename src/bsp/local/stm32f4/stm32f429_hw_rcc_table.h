#ifndef	__STM32F429_HW_RCC_TABLE_H__
#define	__STM32F429_HW_RCC_TABLE_H__

#include <stdint.h>
#include <stdbool.h>

#include "stm32f429_hw_rcc_defs.h"

struct stm32f429_rcc_table_entry {
	stm32f429_rcc_peripheral_name_t periph;
	stm32f429_rcc_bus_name_t bus;
	stm32f429_rcc_clock_name_t clock;

	uint32_t rcc_reset_reg;
	uint32_t rcc_reset_mask;

	uint32_t rcc_clken_reg;
	uint32_t rcc_clken_mask;
};

extern	const struct stm32f429_rcc_table_entry *
	    stm32f429_rcc_table_get_peripheral(
	    stm32f429_rcc_peripheral_name_t periph);

#endif	/* __STM32F429_HW_RCC_TABLE_H__ */
