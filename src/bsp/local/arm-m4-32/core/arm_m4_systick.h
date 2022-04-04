#ifndef	__ARM_M4_SYSTICK_H__
#define	__ARM_M4_SYSTICK_H__

extern	void arm_m4_systick_init(void);
extern	void arm_m4_systick_enable_interrupt(bool enable);
extern	void arm_m4_systick_set_counter_and_start(uint32_t counter_val);
extern	void arm_m4_systick_stop_counting(void);
extern	uint32_t arm_m4_systick_get_tenms_calib(void);

#endif	/* __ARM_M4_SYSTICK_H__ */
