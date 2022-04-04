#ifndef	__KERN_CORE_EXCEPTION_H__
#define	__KERN_CORE_EXCEPTION_H__


extern	void exception_spin(void);
extern	void exception_panic(const char *fmt, ...);
extern	void exception_start(void);
extern	void exception_finish(void);

#endif	/* __KERN_CONF_EXCEPTION_H__ */
