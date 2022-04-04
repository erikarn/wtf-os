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
	console_printf("[idle] started!\n");
	while (1) {
		console_printf("[idle] entering idle!\n");
		platform_cpu_idle();
	}
}

static void
kern_test_task_fn(void)
{
	console_printf("[test] started!\n");
	while (1) {
		console_printf("[test] entering idle!\n");
		platform_cpu_idle();
	}
}

void
kern_task_setup(void)
{
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
	platform_critical_lock_t s;

	platform_critical_enter(&s);
	task = _kern_task_lookup_locked(task_id);
	platform_critical_exit(&s);
	return (task);
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
	platform_critical_lock_t s;

	console_printf("[task] %s: called\n", __func__);
	platform_critical_enter(&s);
	kern_task_set_state_locked(KERN_TASK_STATE_DYING);
	platform_critical_exit(&s);
	return;
}

void
kern_task_kill(kern_task_id_t task_id)
{
	return;
}

int
kern_task_wait(kern_task_signal_mask_t sig_mask,
    kern_task_signal_set_t *sig_set)
{
	return (-1);
}

int
kern_task_signal(kern_task_id_t task_id, kern_task_signal_set_t sig_set)
{
	return (-1);
}

static void
_kern_task_set_state_locked(struct kern_task *task,
    kern_task_state_t new_state)
{
	/* Update state */
	task->cur_state = new_state;

	/* XXX TODO: need to shift it to different task lists */
	/* XXX TODO: may need to signal a context switch */
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
	_kern_task_set_state_locked(task, new_state);
	kern_task_refcount_dec(task);
}
