#ifndef	__STM32F429_HW_EXTI_H__
#define	__STM32F429_HW_EXTI_H__

extern	void stm32f429_hw_exti_enable_interrupt(uint32_t exti_intr,
	    bool enable);
extern	void stm32f429_hw_exti_enable_event(uint32_t exti_intr, bool enable);
extern	void stm32f429_hw_exti_set_intr_type(uint32_t exti_intr,
	    bool on_rising, bool on_falling);
extern	void stm32f429_hw_exti_set_swi_trigger(uint32_t exti_intr);
extern	uint32_t stm32f429_hw_exti_get_pending_interrupts(void);
extern	void stm32f429_hw_exti_ack_pending_interrupt(uint32_t exti_intr);

#endif	/* __STM32F429_HW_EXTI_H__ */
