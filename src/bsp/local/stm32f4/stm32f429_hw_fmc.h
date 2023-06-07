
#ifndef	__STM32F429_HW_FMC_H__
#define	__STM32F429_HW_FMC_H__


extern	bool stm32f429_hw_fmc_send_command(uint32_t command);
extern	bool stm32f429_hw_fmc_send_clock_enable(void);
extern	bool stm32f429_hw_fmc_set_refresh_count(uint32_t refresh);
extern	bool stm32f429_hw_fmc_disable_write_protection(void);


#endif
