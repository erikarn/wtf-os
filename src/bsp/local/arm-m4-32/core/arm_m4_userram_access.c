
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h> /* XXX TODO: write local versions! */

#include <os/bit.h>
#include <os/reg.h>

#include <core/platform.h>
#include <hw/types.h>
#include <asm/asm_defs.h>

#include <core/user_ram_access.h>

/*
 * These routines are the platform specific routines for
 * accessing memory in user task space from the kernel.
 *
 * These could be inline as they're basically pass-through
 * for M4, /but/ we could also do some software based
 * validation of the memory addresses for the given
 * task we're running!
 */

bool
platform_user_ram_copy_from_user(const uaddr_t uaddr, paddr_t paddr,
    uint32_t len)
{
	memcpy((void *)(uintptr_t)paddr, (void *)(uintptr_t)uaddr, len);

	return (true);
}

bool
platform_user_ram_copy_to_user(const paddr_t paddr, uaddr_t uaddr,
    uint32_t len)
{
	memcpy((void *)(uintptr_t)uaddr, (void *)(uintptr_t)paddr, len);

	return (true);
}

bool
platform_user_ram_read_byte_from_user(const uaddr_t uaddr, uint8_t *dst)
{
	*dst = *((uint8_t *)(uintptr_t) uaddr);
	return (true);
}

