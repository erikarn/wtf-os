#ifndef	__STM32F429_HW_RCC_H__
#define	__STM32F429_HW_RCC_H__

#include <stdint.h>
#include <stdbool.h>

#include "stm32f429_hw_rcc_defs.h"

extern	uint32_t stm32f429_get_system_core_clock(void);
extern	uint32_t stm32f429_rcc_get_pclk1_freq(void);
extern	uint32_t stm32f429_rcc_get_pclk2_freq(void);

extern	bool stm32f429_rcc_peripheral_enable(
	    stm32f429_rcc_peripheral_name_t periph, bool enable);
extern	bool stm32f429_rcc_peripheral_reset(
	    stm32f429_rcc_peripheral_name_t periph, bool reset);

#endif	/* __STM32F429_HW_RCC_H__ */
