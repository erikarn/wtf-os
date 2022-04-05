#ifndef	__ARM_M4_SYSTICK_H__
#define	__ARM_M4_SYSTICK_H__

extern	void arm_m4_systick_init(void);
extern	void arm_m4_systick_enable_interrupt(bool enable);
extern	void arm_m4_systick_set_counter(uint32_t counter_val);
extern	void arm_m4_systick_set_usec(uint32_t usec);
extern	void arm_m4_systick_start_counting(void);
extern	void arm_m4_systick_stop_counting(void);
extern	uint32_t arm_m4_systick_get_tenms_calib(void);

extern	void arm_m4_systick_set_hclk_freq(uint32_t hclk_freq);

#endif	/* __ARM_M4_SYSTICK_H__ */
