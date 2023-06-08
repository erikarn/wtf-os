/*
 * Copyright (C) 2023 Adrian Chadd <adrian@freebsd.org>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-Licence-Identifier: GPL-3.0-or-later
 */

#include <stdint.h>
#include <stdbool.h>

#include "../os/reg.h"
#include "stm32f429_hw_map.h"
#include "stm32f429_hw_fmc_reg.h"
#include "stm32f429_hw_fmc.h"

/**
 * Send a command + arguments, wait for the command to complete.
 *
 * This writes the command word, which needs to include the destination
 * bank flag and any arguments, and then will wait for the command completes.
 *
 * Since this may be done early in boot, I'm not yet(!) using a DELAY()
 * function.
 *
 * @param[in] command command/flags
 * @retval true if success, false if command failed / timed out
 */
bool
stm32f429_hw_fmc_send_command(uint32_t command)
{
	uint32_t timeout;

	os_reg_write32(FMC_R_BASE, FMC_REG_SDCMR, command);
	timeout = 0xffff;
	/* XXX TODO: some early boot delay function? */
	while ((timeout != 0) &&
	     ((os_reg_read32(FMC_R_BASE, FMC_REG_SDSR) & 0x00000020) != 0)) {
		timeout--;
	}

	return (timeout != 0);
}

/**
 * Set the refresh timer.
 *
 * This sets the initial refresh timer between refresh cycles.
 *
 * @param[in] refresh timer
 * @retval true if configured, false if failed
 */
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
