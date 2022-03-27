#ifndef	__KERN_CONSOLE_H__
#define	__KERN_CONSOLE_H__

typedef void console_op_putc_fn_t(char c);
typedef void console_op_flush_fn_t(void);

struct console_ops {
	console_op_putc_fn_t *putc_fn;
	console_op_flush_fn_t *flush_fn;
};

extern	void console_init(void);
extern	void console_set_ops(struct console_ops *);

extern	void console_putc(char c);
extern	void console_puts(const char *s);
extern	void console_flush(void);

extern	int console_printf(const char *fmt, ...);

#endif	/* __KERN_CONSOLE_H__ */
