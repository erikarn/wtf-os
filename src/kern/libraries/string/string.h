#ifndef	__LIB_STRING_STRING_H__
#define	__LIB_STRING_STRING_H__

extern	size_t kern_strlen(const char *str);
extern	size_t kern_strnlen(const char *str, size_t maxlen);
extern	size_t kern_strlcpy(char *dst, const char *src, size_t dstlen);
extern	int kern_strncmp(const char *s1, const char *s2, size_t len);

#endif	/* __LIB_STRING_STRING_H__ */
