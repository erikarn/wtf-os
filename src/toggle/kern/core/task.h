#ifndef	__KERN_TASK_H__
#define	__KERN_TASK_H__

/*
 * This defines a single task.
 *
 * For now it's not in a collection; I'll worry about that once
 * I actually /have/ two tasks running (an idle task and some
 * running task.  Ok, maybe two running tasks.  We'll see.)
 */

#define	KERN_TASK_NAME_SZ		16

struct kern_task {
	volatile stack_addr_t stack_top;

	char task_name[KERN_TASK_NAME_SZ];
	stack_addr_t kern_stack;
	int kern_stack_size;
	void *kern_entry_point;
};

extern	struct kern_task * current_task;

/* XXX sigh, naming is hard */
extern	void kern_task_init(struct kern_task *task, void *entry_point,
	    const char *name, stack_addr_t kern_stack, int kern_stack_size);

extern	void kern_task_setup(void);
extern	void kern_task_select(void);


#endif	/* __KERN_TASK_H__ */
