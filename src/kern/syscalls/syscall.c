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
 * Generic syscall handler.
 *
 * For now, since I'm super lazy, I'm going to have the platform dependent
 * code call into here and we'll demux into the right place.
 *
 * Yeah this wastes kernel stack; we should really collapse this down a
 * bit once more syscalls are fleshed out.
 *
 * Also yes I am ALSO being super slack by not specifically using macros,
 * functions, etc to do a copyin/copyout style abstraction whilst I bring
 * this up.  Yes I should do this before I write more than a couple of
 * syscalls.  It should be a glorified no-op here BUT it would let me
 * do some runtime bounds checking that user processes aren't inventing
 * pointers to random memory locations outside their allocated space.
 * (And yes, that's also what an MMU, MPU, etc is for.)
 */
syscall_retval_t
kern_syscall_handler(syscall_arg_t arg, syscall_arg_t arg2,
    syscall_arg_t arg3, syscall_arg_t arg4)
{
	syscall_retval_t retval = -1;
	uint16_t arg1, syscall_id;

	/*
	 * arg1 on a 32 bit platform is defined as:
	 *
	 * | uint16 arg1 | uint16 syscall_id |
	 *
	 * XXX TODO: for 64 bit platforms we'll have to fix this!
	 * (XXX TODO: eg move the decoding into the platform layer maybe?)
	 */
	syscall_id = (arg & 0x0000ffff);
	arg1 = (arg & 0xffff0000) >> 16;

	/* Demux appropriately */
	switch (syscall_id) {
	case SYSCALL_ID_CONSOLE_WRITE:
		retval = kern_syscall_putsn(arg1, arg2, arg3, arg4);
		break;
	case SYSCALL_ID_CONSOLE_SLEEP:
		retval = kern_syscall_sleep(arg1, arg2, arg3, arg4);
		break;
	default:
		retval = -1;
		break;
	}

	return (retval);
}
