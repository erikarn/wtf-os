#ifndef	__ARM_M4_PLATFORM_H__
#define	__ARM_M4_PLATFORM_H__

extern	void platform_cpu_init(void);
extern	void platform_cpu_idle(void);

extern	void platform_irq_enable(uint32_t irq);
extern	void platform_irq_disable(uint32_t irq);

extern	void platform_cpu_irq_enable(void);
extern	void platform_cpu_irq_disable(void);

extern	void platform_cpu_fiq_enable(void);
extern	void platform_cpu_fiq_disable(void);

#endif	/* __ARM_M4_PLATFORM_H__ */
