#include <stdint.h>
#include <stdbool.h>

#include "../os/reg.h"
#include "stm32f429_hw_map.h"
#include "stm32f429_hw_fmc_reg.h"
#include "stm32f429_hw_fmc.h"

/* XXX for debugging */
#include <stddef.h>
#include "kern/console/console.h"

/*
 * XXX TODO: these commands have flag bits too so break things
 * apart so we know things like which bank they're sent to.
 */


bool
stm32f429_hw_fmc_send_command(uint32_t command)
{
	uint32_t timeout;

	console_printf("%s: writing 0x%08x\n", __func__, command);

	os_reg_write32(FMC_R_BASE, FMC_REG_SDCMR, command);
	timeout = 0xffff;
	while ((timeout != 0) &&
	     ((os_reg_read32(FMC_R_BASE, FMC_REG_SDSR) & 0x00000020) != 0)) {
		timeout--;
	}

	return (timeout != 0);
}

bool
stm32f429_hw_fmc_set_refresh_count(uint32_t refresh)
{
	uint32_t reg;

	/* XXX TODO: yeah, this should be a proper RMW */
	reg = os_reg_read32(FMC_R_BASE, FMC_REG_SDRTR);
	/* Bit[0] = clear refresh error, make sure we write 0 to not clear */
	/* bit[1..13] = refresh */
	/* bit[14] = generate inerrupt if refresh error */
	reg &= 0x3ffe;
	reg |= ((refresh & 0x1fff) << 1);
	os_reg_write32(FMC_R_BASE, FMC_REG_SDRTR, reg);

	return (true);
}

bool
stm32f429_hw_fmc_disable_write_protection(void)
{
	uint32_t reg;

	reg = os_reg_read32(FMC_R_BASE, FMC_REG_SDCR2);
	/* XXX TODO: yeah, this should be a proper RMW */
	reg &= 0xFFFFFDFF;
	os_reg_write32(FMC_R_BASE, FMC_REG_SDCR2, reg);

	return (true);
}
