#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <hw/types.h>

#include <kern/libraries/string/string.h>
#include <kern/libraries/list/list.h>
#include <kern/libraries/container/container.h>

#include <kern/core/exception.h>
#include <kern/core/task.h>
#include <kern/core/timer.h>
#include <kern/console/console.h>

#include <core/platform.h>
#include <core/lock.h>

struct kern_task *current_task = NULL;
static platform_spinlock_t kern_task_spinlock;

static struct list_head kern_task_list;
static struct list_head kern_task_active_list;
static uint32_t active_task_count;

static bool task_switch_ready = false;

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

	list_node_init(&task->task_list_node);

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
	 * Mark this task as idle, we haven't started it running.
	 */
	task->cur_state = KERN_TASK_STATE_IDLE;

	/*
	 * Next we call into the platform code to initialise our
	 * stack with the above parameters so we can context
	 * switch /into/ the task when we're ready to run it.
	 */
	task->stack_top = platform_task_stack_setup(
	    task->kern_stack + kern_stack_size,
	    entry_point, NULL);

	/*
	 * Last, add it to the global list of tasks.
	 */
	platform_spinlock_lock(&kern_task_spinlock);
	list_add_tail(&kern_task_list, &task->task_list_node);
	platform_spinlock_unlock(&kern_task_spinlock);
}

/*
 * Select a task to run.
 *
 * This is called by the platform task switching code to select
 * a new task to run.
 */
void
kern_task_select(void)
{
	struct kern_task *task;
	struct list_node *node;

	/*
	 * Walk the list of active tasks.  For now we just pick the
	 * task at the head of the list and we put it at the tail,
	 * making this purely a round robin scheduler.
	 */
	platform_spinlock_lock(&kern_task_spinlock);
	node = kern_task_active_list.head;
	if (node == NULL) {
		task = NULL;
	} else {
		task = container_of(node, struct kern_task, task_active_node);
	}

	/*
	 * If there isn't a task on the active list then we schedule the
	 * idle task.  If there /is/ a task then we get it ready to run
	 * and move the current task to the end of the active list.
	 */

	/*
	 * Update this task to be ready to run again.
	 *
	 * It's done like this rather than calling set_state because
	 * we don't want to shuffle it around on lists and kick more
	 * re-scheduling.  RUNNING<->READY is fine like this; it's
	 * the sleep transitions we need that set state call for.
	 */
	if (current_task->cur_state == KERN_TASK_STATE_RUNNING)
		current_task->cur_state = KERN_TASK_STATE_READY;

	/* Update current task */
	if (task != NULL) {
		current_task = task;
		/*
		 * Move to the end of the active list so other tasks
		 * can run.
		 */
		list_delete(&kern_task_active_list, &task->task_active_node);
		list_add_tail(&kern_task_active_list, &task->task_active_node);
	} else {
		current_task = &idle_task;
	}

	/* Mark it as running */
	current_task->cur_state = KERN_TASK_STATE_RUNNING;

	platform_spinlock_unlock(&kern_task_spinlock);

	/*
	 * If we have any other tasks available then we need
	 * to kick start the timer so we can context switch.
	 *
	 * Technically we only need to run the timer if
	 * we have more than one running task.  There's no
	 * need to context switch if we have a /single/ task
	 * running.
	 */
	kern_timer_taskcount(active_task_count);
}

static void
kern_idle_task_fn(void)
{
	int count = 0;

	console_printf("[idle] started!\n");
	while (1) {
		console_printf("[idle] entering idle!\n");

		/*
		 * if we get to the idle scheduler loop
		 * then it means nothing else needed to
		 * run.
		 *
		 * Call into the timer to see if it needs
		 * to schedule another tick.  If it doesn't,
		 * it may actually decide to disable the timer
		 * entirely.
		 */
		kern_timer_idle();

		platform_cpu_idle();

		// Hack to test sleep/wakeup task stuff
		count++;
		if (count == 10) {
			kern_task_id_t t;

			count = 0;
//			console_printf("[idle] waking up test task!\n");
			t = kern_task_to_id(&test_task);
			kern_task_signal(t, 0x00000001);
		}
	}
}

static void
kern_test_task_fn(void)
{
	kern_task_signal_set_t sig;

	console_printf("[test] started!\n");

	kern_task_set_sigmask(0x11111111, 0x00000001);
	while (1) {
		console_printf("[test] **** entering wait!\n");
		(void) kern_task_wait(0xffffffff, &sig);
//		console_printf("[test] **** entering idle!\n");
//		platform_cpu_idle();
	}
}

/**
 * Update the given task state and potentially reschedule things
 * to run if needed.
 */
static void
_kern_task_set_state_locked(struct kern_task *task,
    kern_task_state_t new_state)
{
	bool do_ctx = false;

	//console_printf("task %s state %d -> %d\n", task->task_name, task->cur_state, new_state);

	/* Update state, only do work if the state hasn't changed */
	if (task->cur_state == new_state)
		return;

	/* Update the new state */
	task->cur_state = new_state;

	switch (task->cur_state) {
	case KERN_TASK_STATE_DYING:
		/* XXX TODO */
		console_printf("[task] taskptr 0x%x todo state DYING\n",
		    task);
		task->cur_state = KERN_TASK_STATE_SLEEPING;
		/* FALLTHROUGH */
	case KERN_TASK_STATE_SLEEPING:
		if (task->is_on_active_list) {
			list_delete(&kern_task_active_list,
			    &task->task_active_node);
			task->is_on_active_list = false;
			active_task_count--;
		}
		do_ctx = true;
		break;
	case KERN_TASK_STATE_READY:
		/*
		 * XXX TODO: we only really need to context switch if
		 * this task has a higher priority than the one currently
		 * running.  For now let's just not and let the scheduler
		 * pick it up.
		 */
		if (task->is_on_active_list == false) {
			list_add_tail(&kern_task_active_list,
			    &task->task_active_node);
			task->is_on_active_list = true;
			active_task_count++;
			do_ctx = true;
		} else {
			do_ctx = false;
		}
		break;
	case KERN_TASK_STATE_RUNNING:
	default:
		console_printf("[task] taskptr 0x%x unhandled state %d\n",
		    task, task->cur_state);
		break;
	}


	/*
	 * Remember, this won't DO the context switch until the critical
	 * section / spinlock is finished.  It's just requesting that
	 * we do a context switch soon.
	 */
	if (do_ctx && task_switch_ready) {
		platform_kick_context_switch();
	}
}

void
kern_task_setup(void)
{
	platform_spinlock_init(&kern_task_spinlock);
	list_head_init(&kern_task_list);
	list_head_init(&kern_task_active_list);
	active_task_count = 0;

	/* Idle task will be magically made ready to run */
	kern_task_init(&idle_task, kern_idle_task_fn, "kidle",
	    (stack_addr_t) kern_idle_stack, sizeof(kern_idle_stack));

	/* Test task will be made ready to run as well */
	kern_task_init(&test_task, kern_test_task_fn, "ktest",
	    (stack_addr_t) kern_test_stack, sizeof(kern_test_stack));
	platform_spinlock_lock(&kern_task_spinlock);
	_kern_task_set_state_locked(&test_task, KERN_TASK_STATE_READY);
	platform_spinlock_unlock(&kern_task_spinlock);
}

static struct kern_task *
_kern_task_lookup_locked(kern_task_id_t task_id)
{
	struct kern_task *task;

	task = (struct kern_task *) (uintptr_t) task_id;
	kern_task_refcount_inc(task);
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
	//console_printf("task %s waking up, state is %d\n", task->task_name, task->cur_state);
	if (task->cur_state == KERN_TASK_STATE_SLEEPING) {
		_kern_task_set_state_locked(task, KERN_TASK_STATE_READY);
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
		console_printf("invalid state (%d)\n", task->cur_state);
		kern_task_refcount_dec(task);
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
	kern_task_refcount_dec(task);

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

/**
 * Called by the timer to potentially schedule a context switch.
 *
 * This will check to see if we CAN context switch or whether we
 * need to wait until we're fully setup.
 */
void
kern_task_tick(void)
{
	if (task_switch_ready == false) {
		return;
	}

	platform_critical_lock_t s;
	platform_critical_enter(&s);
	platform_kick_context_switch();
	platform_critical_exit(&s);
}

/**
 * Called to actually mark the system as ready to task-switch.
 *
 * This sets the flag that allows the timer and scheduler to
 * start running.
 */
void
kern_task_ready(void)
{
	task_switch_ready = true;
}
