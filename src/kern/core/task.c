#include <stddef.h>
#include <stdarg.h>

#include <hw/types.h>

#include <kern/core/exception.h>
#include <kern/core/task.h>
#include <kern/console/console.h>

#include <core/platform.h>
#include <core/lock.h>

#include <kern/libraries/string/string.h>

struct kern_task *current_task = NULL;
static platform_spinlock_t kern_task_spinlock;

/*
 * Idle task
 */
static struct kern_task idle_task;
/* Note: 256 bytes for now; I think we're saving VFP registers */
static uint8_t kern_idle_stack[256] __attribute__ ((aligned(8))) = { 0 };

static struct kern_task test_task;
/* Note: 256 bytes for now; I think we're saving VFP registers */
static uint8_t kern_test_stack[256] __attribute__ ((aligned(8))) = { 0 };

/**
 * Initialise the given task structure.
 *
 * This just sets the generic and the platform side of things up
 * but doesn't add it to the runlist or start the task.
 */
void
kern_task_init(struct kern_task *task, void *entry_point,
    const char *name, stack_addr_t kern_stack, int kern_stack_size)
{

	kern_strlcpy(task->task_name, name, KERN_TASK_NAME_SZ);

	/*
	 * For now we start with the entry point and kernel stack
	 * being for the "kernel".  I'll worry about changing this
	 * to support entering a non-kernel task later (where we'll
	 * end up with both kernel/user stack, and a user address
	 * entry point.)
	 */
	task->kern_entry_point = entry_point;
	task->kern_stack = kern_stack;
	task->kern_stack_size = kern_stack_size;

	/*
	 * Next we call into the platform code to initialise our
	 * stack with the above parameters so we can context
	 * switch /into/ the task when we're ready to run it.
	 */
	task->stack_top = platform_task_stack_setup(
	    task->kern_stack + kern_stack_size,
	    entry_point, NULL);
}

/*
 * Select a task to run.
 *
 * This is called by the platform task switching code to select
 * a new task to run.
 *
 * It's inspired by how FreeRTOS does its task switching whilst
 * I stand up something.
 */
void
kern_task_select(void)
{
	if (current_task == &idle_task)
		current_task = &test_task;
	else
		current_task = &idle_task;
}

static void
kern_idle_task_fn(void)
{
	kern_task_signal_set_t sig;

	console_printf("[idle] started!\n");
	while (1) {
		console_printf("[idle] entering idle!\n");
		(void) kern_task_wait(0xffffffff, &sig);
		platform_cpu_idle();
	}
}

static void
kern_test_task_fn(void)
{
	console_printf("[test] started!\n");

	kern_task_set_sigmask(0x11111111, 0x00000001);
	while (1) {
		console_printf("[test] entering idle!\n");
		platform_cpu_idle();
	}
}

void
kern_task_setup(void)
{
	platform_spinlock_init(&kern_task_spinlock);

	kern_task_init(&idle_task, kern_idle_task_fn, "kidle",
	    (stack_addr_t) kern_idle_stack, sizeof(kern_idle_stack));
	kern_task_init(&test_task, kern_test_task_fn, "ktest",
	    (stack_addr_t) kern_test_stack, sizeof(kern_test_stack));
}

static struct kern_task *
_kern_task_lookup_locked(kern_task_id_t task_id)
{
	struct kern_task *task;

	task = (struct kern_task *) (uintptr_t) task_id;
	task->refcount++;
	return (task);
}

struct kern_task *
kern_task_lookup(kern_task_id_t task_id)
{
	struct kern_task *task;

	platform_spinlock_lock(&kern_task_spinlock);
	task = _kern_task_lookup_locked(task_id);
	platform_spinlock_unlock(&kern_task_spinlock);
	return (task);
}

kern_task_id_t
kern_task_to_id(struct kern_task *task)
{

	return (kern_task_id_t) (uintptr_t) task;
}

void
kern_task_refcount_inc(struct kern_task *task)
{
	task->refcount++;
}

void
kern_task_refcount_dec(struct kern_task *task)
{
	task->refcount--;
}

static void
_kern_task_set_state_locked(struct kern_task *task,
    kern_task_state_t new_state)
{
	/* Update state, only do work if the state hasn't changed */
	if (task->cur_state == new_state)
		return;

	task->cur_state = new_state;

	/* XXX TODO: need to shift it to different task lists */
	/* XXX TODO: may need to signal a context switch */
}

void
kern_task_exit(void)
{

	console_printf("[task] %s: called\n", __func__);

	platform_spinlock_lock(&kern_task_spinlock);
	kern_task_set_state_locked(KERN_TASK_STATE_DYING);
	platform_spinlock_unlock(&kern_task_spinlock);

	return;
}

void
kern_task_kill(kern_task_id_t task_id)
{
	console_printf("[task] %s: called\n", __func__);
	/* XXX TODO */
	return;
}


/**
 * Wait for the given set of signals to be set on the current task.
 *
 * If any of those bits are already set it will return immediately.
 * Otherwise it will mark itself as sleeping, and when it becomes
 * runnable again it will check again.
 *
 * Any signals that were returned in the mask will be cleared.
 *
 * This can not be called in a critical section or with locks held.
 *
 * @param[in] sig_mask mask for signals to check
 * @param[out] sig_set set of signals that were triggered
 */
int
kern_task_wait(kern_task_signal_mask_t sig_mask,
    kern_task_signal_set_t *sig_set)
{
	volatile kern_task_signal_set_t sigs;

	/*
	 * This is a bit messy and I dislike it, but I'll do this
	 * for initial bootstrapping.
	 *
	 * I want to make sure I'm not pre-empted whilst checking
	 * the signal mask in case it gets updated by another task
	 * or interrupt that has pre-empted.
	 *
	 * But, we don't have atomic mutexes here right now and
	 * I can't exactly hold a critical section across yielding.
	 *
	 * So, commit some messy sins for now even though they're not
	 * technically correct, and then we'll fix it all in post.
	 */
	while (1) {
		platform_spinlock_lock(&kern_task_spinlock);

		/* Get the currently set signals */
		sigs = current_task->sig_set;

		if ((sigs & sig_mask & current_task->sig_mask) != 0) {
			_kern_task_set_state_locked(current_task,
			    KERN_TASK_STATE_READY);
			/* Mask out what we've just found */
			current_task->sig_set &= ~(sigs & sig_mask);
			platform_spinlock_unlock(&kern_task_spinlock);
			break;
		}

		/* Still waiting */
		_kern_task_set_state_locked(current_task,
		    KERN_TASK_STATE_SLEEPING);
		platform_spinlock_unlock(&kern_task_spinlock);

		/*
		 * At this point we'll hopefully end up being pre-empted.
		 * I may end up really needing an explicit yield() routine
		 * so I definitely do context switch to something else;
		 * the worst(!) that can happen here is it'll just keep
		 * running through this loop until it ends up context
		 * switching.
		 *
		 * XXX TODO: yeah add a sleep/yield function here.
		 */
	}

	*sig_set = sigs;
	return (0);
}

static int
_kern_task_state_valid_locked(struct kern_task *task)
{
	switch (task->cur_state) {
	case KERN_TASK_STATE_SLEEPING:
	case KERN_TASK_STATE_READY:
	case KERN_TASK_STATE_RUNNING:
		return (1);
	default:
		return (0);
	}
}

static void
_kern_task_wakeup_locked(struct kern_task *task)
{
	if (task->cur_state == KERN_TASK_STATE_SLEEPING) {
		_kern_task_set_state_locked(current_task,
		    KERN_TASK_STATE_READY);
	}
}

/**
 * Signal the given task.
 *
 * This sets the signals on the given task and if it's ready to
 * be woken up, it'll mark it as READY.  The scheduler will then
 * eventually run the task.
 *
 * @param[in] task_id kernel task ID
 * @param[in] sig_set signal set to set
 * @retval 0 if OK, -1 if error
 */
int
kern_task_signal(kern_task_id_t task_id, kern_task_signal_set_t sig_set)
{
	struct kern_task *task;

	platform_spinlock_lock(&kern_task_spinlock);

	/*
	 * This task can be active, running on another core; it may be
	 * sleeping.
	 */
	task = _kern_task_lookup_locked(task_id);

	if (task == NULL) {
		platform_spinlock_unlock(&kern_task_spinlock);
		return (-1);
	}

	if (! _kern_task_state_valid_locked(task)) {
		platform_spinlock_unlock(&kern_task_spinlock);
		return (-1);
	}

	task->sig_set |= sig_set;
	if ((task->sig_set & task->sig_mask) != 0) {
		/*
		 * Wake up the task if it's not sleeping.
		 *
		 * It may decide that it's not /actually/ ready
		 * to wake up because of the mask set by a call
		 * into kern_task_wait(), but that's not our problem.
		 */
		_kern_task_wakeup_locked(task);
	}

	platform_spinlock_unlock(&kern_task_spinlock);

	return (0);
}

void
kern_task_set_state_locked(kern_task_state_t new_state)
{
	_kern_task_set_state_locked(current_task, new_state);
}

void
kern_task_set_task_state_locked(kern_task_id_t task_id,
    kern_task_state_t new_state)
{
	struct kern_task *task;

	task = _kern_task_lookup_locked(task_id);

	if (task == NULL)
		return;

	_kern_task_set_state_locked(task, new_state);
	kern_task_refcount_dec(task);
}

void
kern_task_set_sigmask(kern_task_signal_mask_t and_sig_mask,
    kern_task_signal_mask_t or_sig_mask)
{
	platform_spinlock_lock(&kern_task_spinlock);
	current_task->sig_mask &= and_sig_mask;
	current_task->sig_mask |= or_sig_mask;
	platform_spinlock_unlock(&kern_task_spinlock);
}

kern_task_signal_mask_t
kern_task_get_sigmask(void)
{

	return (current_task->sig_mask);
}
