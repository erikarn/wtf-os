
#include <stddef.h>
#include <stdarg.h>

#include <kern/console/console.h>
#include <kern/libraries/printf/mini_printf.h>

static char cons_add_crlf = 1;

static struct console_ops *c_ops = NULL;

/**
 * Initialise the console subsystem.
 */
void
console_init(void)
{
}

/**
 * Set the console operations.
 *
 * These are the underlying hardware operations to
 * read, write and flush the console device.
 *
 * @param[in] c console operations
 */
void
console_set_ops(struct console_ops *c)
{

	c_ops = c;
}

/**
 * Write a character to the console.
 *
 * @param[in] c character to write.
 */
void
console_putc(char c)
{
	if (c_ops != NULL) {
		c_ops->putc_fn(c);
	}
}

/**
 * Write a string to the console.
 *
 * This will do LF->CRLF translation if enabled.
 *
 * @param[in] s C string to write.
 */
void
console_puts(const char *s)
{
	while (*s != '\0') {
		if ((cons_add_crlf == 1) && (*s == '\n')) {
			console_putc('\r');
		}
		console_putc(*s);
		s++;
	}
}

/**
 * Write a string to the console, knowing the length.
 *
 * This will do LF->CRLF translation if enabled.
 *
 * @param[in] s C string to write.
 */
void
console_putsn(const char *s, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		if ((cons_add_crlf == 1) && (*s == '\n')) {
			console_putc('\r');
		}
		console_putc(*s);
		s++;
	}
}


/**
 * Flush the console to the underlying physical device.
 *
 * If ths console is buffered and/or if the underlying
 * hardware device is buffered, then this routine will
 * block until the flush completes.
 */
void
console_flush(void)
{
}

/**
 * Add a character into the console input buffer.
 *
 * This is to be called from the receive path of the console
 * driver to push characters into the console input path.
 *
 * This routine will likely be called from an interrupt context,
 * so when it's time to actually do anything, we'll want to have
 * some kind of interrupt/mutex/spinlock stuff going.
 *
 * @param[in] c character to add to the input.
 */
void
console_input(char c)
{
	/* XXX TODO, obviously */
}

/**
 * Test console printf implementation.
 *
 * This uses the mini printf implementation and a local stream
 * iterator to print each byte out as we receive it.
 */
static int
_console_printf_puts(char *s, int len, void *state)
{

	console_putsn(s, len);

	return (len);
}

int
console_printf(const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = mini_vpprintf(_console_printf_puts, NULL, fmt, va);
	va_end(va);

	return (ret);
}

int
console_vprintf(const char *fmt, va_list args)
{

	return mini_vpprintf(_console_printf_puts, NULL, fmt, args);
}
