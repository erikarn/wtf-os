#ifndef	__ARM_M4_PLATFORM_H__
#define	__ARM_M4_PLATFORM_H__

#include <stdbool.h>
#include <hw/types.h>
#include <hw/prot.h>

extern	void platform_cpu_init(void);
extern	void platform_cpu_idle(void);

extern	void platform_irq_enable(uint32_t irq);
extern	void platform_irq_disable(uint32_t irq);

extern	void platform_cpu_irq_enable(void);
extern	void platform_cpu_irq_disable(void);

extern	void platform_cpu_fiq_enable(void);
extern	void platform_cpu_fiq_disable(void);

extern	irq_save_t platform_cpu_irq_disable_save(void);
extern	void platform_cpu_irq_enable_restore(irq_save_t mask);

extern	stack_addr_t platform_task_stack_setup(stack_addr_t stack,
	    void *entry_point, void *param, uint32_t r9, bool is_user,
	    void *exit_func);

extern	void arm_m4_exception_set_pendsv(void);
extern	void platform_kick_context_switch(void);

extern	void platform_timer_set_msec(uint32_t msec);
extern	void platform_timer_enable(void);
extern	void platform_timer_disable(void);

extern	void platform_mpu_enable(void);
extern	void platform_mpu_disable(void);
extern	void platform_mpu_table_init(platform_mpu_phys_entry_t *table);
extern	bool platform_mpu_table_entry_validate(uint32_t base_addr,
	     uint32_t size, platform_prot_type_t prot);
extern	bool platform_mpu_table_set(platform_mpu_phys_entry_t *table,
	    uint32_t addr, uint32_t size, platform_prot_type_t prot_type);
extern	void platform_mpu_table_program(const platform_mpu_phys_entry_t *table);
extern	uint32_t platform_mpu_table_min_region_size(void);

#endif	/* __ARM_M4_PLATFORM_H__ */
