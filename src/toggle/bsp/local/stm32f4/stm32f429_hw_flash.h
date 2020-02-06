#ifndef	__STM32F429_HW_FLASH_REG_H__
#define	__STM32F429_HW_FLASH_REG_H__

extern	void stm32f429_hw_flash_enable_icache(void);
extern	void stm32f429_hw_flash_enable_dcache(void);
extern	void stm32f429_hw_flash_disable_icache(void);
extern	void stm32f429_hw_flash_disable_dcache(void);
extern	void stm32f429_hw_flash_reset_icache(void);
extern	void stm32f429_hw_flash_reset_dcache(void);

#endif	/* __STM32F429_HW_FLASH_REG_H__ */
