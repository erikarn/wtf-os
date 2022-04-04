
#include <stddef.h>
#include <stdint.h>

#include <kern/libraries/string/string.h>

/**
 * kern_strnlen - calculate string length.
 */
size_t
kern_strnlen(const char *str, size_t maxlen)
{
	size_t len = 0;

	if (str == NULL)
		return 0;

	while (*str != '\0' && (len < maxlen))
		len++;

	return (len);
}
