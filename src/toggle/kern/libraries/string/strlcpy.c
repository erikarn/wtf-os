#include <stddef.h>
#include <stdint.h>

#include <kern/libraries/string/string.h>

/**
 * kern_strlcpy - copy w/ NUL termination included.
 *
 * @param[out] dst destination buffer
 * @param[in] src source buffer
 * @param[dst_size] destination buffer size, incl the NUL termination space
 * @retval the length of the string in src (minus the NUL termination)
 */
size_t
kern_strlcpy(char *dst, const char *src, size_t dst_size)
{
	size_t i = 0, sl = 0;

	if (src == NULL)
		return (0);

	/* Note: not sanity checking dst is not null yet */

	while (src[sl] != '\0') {
		/* Only copy a byte if we have space */
		if ((i + 1) < dst_size) {
			dst[i] = src[sl];
			i++;
		}
		sl++;
	}

	/* NUL terminate how far we got */
	dst[i] = '\0';

	/* Return length of src */
	return (sl);
}
