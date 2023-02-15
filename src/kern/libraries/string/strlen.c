
#include <stddef.h>
#include <stdint.h>

#include <kern/libraries/string/string.h>

/**
 * strlen - calculate string length.
 */
size_t
kern_strlen(const char *str)
{
	size_t len = 0;

	if (str == NULL)
		return 0;

	while (*str != '\0') {
		str++;
		len++;
	}

	return (len);
}
