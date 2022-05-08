#include <stddef.h>
#include <stdarg.h>

#include <core/platform.h>
#include <kern/core/exception.h>
#include <kern/console/console.h>

/**
 * Called if we need to halt via spinning.
 */
void
exception_spin(void)
{
	while (1)
		platform_cpu_idle();
}

/**
 * Panic the system, print some debugging and spin.
 *
 * @param[in] fmt Format string
 */
void
exception_panic(const char *fmt, ...)
{
	va_list va;

	console_printf("[panic] oh no!\n\n");

	va_start(va, fmt);
	(void) console_vprintf(fmt, va);
	va_end(va);

	exception_spin();
}

/**
 * Start an exception on the current CPU.
 *
 * This will do things like set flags to avoid double panics, later on
 * disable interrupts, use console IO rather than interrupts, etc, etc.
 */
void
exception_start(void)
{
}

/**
 * Finish being in an exception block on the current CPU.
 */
void
exception_finish(void)
{
}
