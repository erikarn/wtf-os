
#include <stddef.h>

#include <kern/console/console.h>

static char cons_add_crlf = 1;

static struct console_ops *c_ops = NULL;

void
console_init(void)
{
}

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
