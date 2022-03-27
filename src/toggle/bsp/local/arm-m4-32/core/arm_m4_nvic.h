#ifndef	__ARM_M4_NVIC_H__
#define	__ARM_M4_NVIC_H__

extern	void arm_m4_nvic_init(void);
extern	void arm_m4_nvic_enable_irq(uint32_t irq);
extern	void arm_m4_nvic_disable_irq(uint32_t irq);
extern	void arm_m4_nvic_set_pending_irq(uint32_t irq);
extern	void arm_m4_nvic_clear_pending_irq(uint32_t irq);
extern	uint32_t arm_m4_nvic_get_pending_irq(uint32_t irq);
extern	void arm_m4_nvic_set_priority(uint32_t irq, uint32_t priority);
extern	uint32_t arm_m4_nvic_get_priority(uint32_t irq);

#endif	/* __ARM_M4_NVIC_H__ */
