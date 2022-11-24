#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include <hw/types.h>

#include <kern/libraries/string/string.h>
#include <kern/libraries/list/list.h>
#include <kern/libraries/container/container.h>

#include <core/platform.h>
#include <core/lock.h>
#include <core/user_ram_access.h>

#include <kern/core/exception.h>
#include <kern/core/signal.h>
#include <kern/core/task.h>
#include <kern/core/timer.h>
#include <kern/console/console.h>
#include <kern/syscalls/syscall.h>

/*
 * Exit the current userland task.
 *
 * This marks the current task as done and removes it from the
 * running task queue.  It'll then get cleaned up appropriately.
 *
 * This routine does not exit or return control to the
 * userland task.
 */
syscall_retval_t
kern_syscall_exit(syscall_arg_t arg1, syscall_arg_t arg2,
    syscall_arg_t arg3, syscall_arg_t arg4)
{

	/* Mark the current task as exiting */
	kern_task_exit();

	/* Keep context switching until we're cleaned up */
	while (true) {
		platform_kick_context_switch();
	}
	return (0);
}
