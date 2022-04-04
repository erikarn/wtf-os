#ifndef	__STM32F429_HW_SYSCFG_H__
#define	__STM32F429_HW_SYSCFG_H__

#include "stm32f429_hw_syscfg_reg.h"

extern	void stm32f429_hw_syscfg_config_exti_mux(uint32_t exti_field,
    stm32f429_hw_syscfg_exti_block_gpio_t gpio_block);

#endif	/* __STM32F429_HW_SYSCFG_H__ */
