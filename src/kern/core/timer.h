#ifndef	__KERN_TIMER_H__
#define	__KERN_TIMER_H__

/* XXX TODO: make these hw/types.h things? */
typedef uint32_t kern_timer_tick_type_t;
typedef int32_t kern_timer_tick_comp_type_t;

extern	kern_timer_tick_type_t kern_timer_tick_msec;

typedef struct kern_timer_event kern_timer_event_t;

typedef	void kern_timer_event_fn_t(kern_timer_event_t *ev, void *arg1,
	    uintptr_t arg2, uint32_t arg3);

struct kern_timer_event {
	kern_timer_event_fn_t *fn;
	struct list_node node;
	void *arg1;
	uintptr_t arg2;
	uint32_t arg3;
	uint32_t tick;
	bool added;
};


extern	void kern_timer_init(void);
extern	void kern_timer_set_tick_interval(uint32_t msec);
extern	void kern_timer_start(void);
extern	void kern_timer_stop(void);
extern	void kern_timer_tick(void);
extern	void kern_timer_idle(void);
extern	void kern_timer_taskcount(uint32_t tasks);

extern	void kern_timer_event_setup(kern_timer_event_t *event,
	    kern_timer_event_fn_t *fn, void *arg1, uintptr_t arg2,
	    uint32_t arg3);
extern	void kern_timer_event_clean(kern_timer_event_t *event);

extern	bool kern_timer_event_add(kern_timer_event_t *event,
	    uint32_t msec);
extern	bool kern_timer_event_del(kern_timer_event_t *event);

#endif	/* __KERN_TIMER_H__ */
