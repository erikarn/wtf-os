#ifndef	__KERN_TASK_H__
#define	__KERN_TASK_H__

typedef enum {
	KERN_TASK_STATE_NONE = 0,
	KERN_TASK_STATE_SLEEPING = 1,
	KERN_TASK_STATE_READY = 2,
	KERN_TASK_STATE_RUNNING = 3,
	KERN_TASK_STATE_DYING = 4,
} kern_task_state_t;

typedef uint32_t kern_task_signal_mask_t;
typedef uint32_t kern_task_signal_set_t;

/**
 * In this task implementation, I'm going to super cheat and
 * just have task IDs match their location in memory.
 * However, things can't make that assumption, and so other
 * tasks (kernel and eventually userland) can only store
 * kern_task_id_t's to reference other tasks.
 */
typedef uintptr_t kern_task_id_t;

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

	kern_task_state_t cur_state;

	/*
	 * Refcount for things that have a reference to this
	 * task.  Tasks can't be deleted until the last
	 * reference is gone.
	 */
	uint16_t refcount;

	/* Current priority.  255 is the highest priority. */
	uint8_t priority;
	uint8_t pad0;

	/*
	 * Current signal set and mask.
	 *
	 * sig_signals is the set of signals that have been set.
	 * Signals need to be ACKed and cleared or they will allow
	 * a follow-up kern_signal_wait() to return immediately.
	 */
	volatile kern_task_signal_set_t sig_set;
	volatile kern_task_signal_mask_t sig_mask;
};

/**
 * The current task, used by the scheduler, tasks themselves and
 * the context switch routine.
 */
extern	struct kern_task * current_task;

/* XXX sigh, naming is hard */
extern	void kern_task_init(struct kern_task *task, void *entry_point,
	    const char *name, stack_addr_t kern_stack, int kern_stack_size);

extern	void kern_task_setup(void);

/**
 * Select a new task to run.  This is called from the platform specific
 * task switching code.  It'll change current_task if required.
 */
extern	void kern_task_select(void);

/**
 * Lookup a task; if it's found return it with the refcount incremented.
 */
extern	struct kern_task * kern_task_lookup(kern_task_id_t task_id);

/**
 * Turn a task struct into a task id.
 */
extern	kern_task_id_t kern_task_to_id(struct kern_task *task);

/**
 * Task reference increment/decrement.
 */
extern	void kern_task_refcount_inc(struct kern_task *task);
extern	void kern_task_refcount_dec(struct kern_task *task);

/**
 * Exit a task.  This is only callable from the task itself.
 */
extern	void kern_task_exit(void);

/**
 * Kill the given task id.
 *
 * I'm NOT quite sure how to implement this without leaking memory,
 * locks, etc, if I'm a kernel task.  Similar if some other task is
 * waiting for a signal from this task, IPC from this task, etc.
 *
 * Hm will need to think about this.
 */
extern	void kern_task_kill(kern_task_id_t task_id);

/**
 * Wait for a signal.  Only callable from the tsak itself.
 *
 * @retval 0 if a signal was set (whether we waited or not).
 * @retval -1 if error.
 */
extern	int kern_task_wait(kern_task_signal_mask_t sig_mask,
	    kern_task_signal_set_t *sig_set);

/**
 * Signal the given task.
 *
 * Multiple signals can be sent at once.
 */
extern	int kern_task_signal(kern_task_id_t task_id,
	    kern_task_signal_set_t signals);

/**
 * Set the state of the current task.
 *
 * Must be called in a critical section.
 */
extern	void kern_task_set_state_locked(kern_task_state_t new_state);

/**
 * Set the state of the given task.
 *
 * Must be called in a critical section.
 */
extern	void kern_task_set_task_state_locked(kern_task_id_t task_id,
	    kern_task_state_t new_state);

extern	void kern_task_set_sigmask(kern_task_signal_mask_t and_sig_mask,
	    kern_task_signal_mask_t or_mask);
extern	kern_task_signal_mask_t kern_task_get_sigmask(void);

#endif	/* __KERN_TASK_H__ */
