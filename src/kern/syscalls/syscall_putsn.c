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
kern_syscall_putsn(syscall_arg_t arg1, syscall_arg_t arg2,
    syscall_arg_t arg3, syscall_arg_t arg4)
{
	syscall_retval_t retval;
	uint8_t ch;
	uint32_t i;

	for (i = 0; i < arg3; i++) {
		if (platform_user_ram_read_byte_from_user(arg2 + i, &ch)
		    == false) {
			retval = -1;
			goto done;
		}
		console_putc(ch);
	}
	retval = 0;

done:
	return (retval);
}
