#ifndef	__KERN_TIMER_H__
#define	__KERN_TIMER_H__

typedef struct kern_timer_event kern_timer_event_t;

typedef	void kern_timer_event_fn_t(void *arg1, uintptr_t arg2,
	    uint32_t arg3);

extern	void kern_timer_init(void);
extern	void kern_timer_set_tick_interval(uint32_t msec);
extern	void kern_timer_start(void);
extern	void kern_timer_stop(void);
extern	void kern_timer_tick(void);

extern	void kern_timer_event_setup(kern_timer_event_t *event,
	    kern_timer_event_fn_t *fn, void *arg1, uintptr_t arg2,
	    uint32_t arg3);
extern	void kern_timer_event_clean(kern_timer_event_t *event);

extern	bool kern_timer_event_add(kern_timer_event_t *event,
	    uint32_t msec);
extern	bool kern_timer_event_del(kern_timer_event_t *event);

#endif	/* __KERN_TIMER_H__ */
