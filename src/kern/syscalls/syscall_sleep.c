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

syscall_retval_t
kern_syscall_sleep(syscall_arg_t arg1, syscall_arg_t arg2,
    syscall_arg_t arg3, syscall_arg_t arg4)
{
	syscall_retval_t retval;
	kern_task_signal_set_t sig;
	bool ret;

	ret = kern_task_timer_set(current_task, (uint32_t) arg2);
	if (ret == false) {
		retval = -1;
		goto done;
	}

	sig = 0;
	(void) kern_task_wait(KERN_SIGNAL_TASK_KSLEEP, &sig);
	retval = 0;

done:
	return (retval);
}
