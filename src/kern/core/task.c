#include <stddef.h>
#include <stdarg.h>

#include <hw/types.h>

#include <kern/core/exception.h>
#include <kern/core/task.h>
#include <kern/console/console.h>

#include <core/platform.h>

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
