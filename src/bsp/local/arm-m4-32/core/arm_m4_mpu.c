/*
 * Copyright (C) 2022 Adrian Chadd <adrian@freebsd.org>.
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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <os/bit.h>
#include <os/reg.h>
#include <os/bitmask.h>

#include <core/platform.h>
#include <core/arm_m4_systick.h>
#include <core/arm_m4_mpu.h>
#include <asm/asm_defs.h>
#include <hw/types.h>
#include <hw/arm_mpu_defs.h>

#include <kern/console/console.h>

static uint8_t mpu_nregions;
static uint8_t mpu_config;

void
arm_m4_mpu_init(void)
{
	uint32_t reg;

	reg = os_reg_read32(ARM_M4_MPU_BASE, ARM_M4_MPU_REG_TYPE);
	console_printf("[mpu] ARM MPU type - 0x%x\n", reg);
	console_printf("[mpu] separate=%d, dregion=%d, iregion=%d\n",
	    (!! reg & ARM_M4_MPU_REG_TYPE_SEPARATE),
	    MS(reg, ARM_M4_MPU_REG_TYPE_DREGION),
	    MS(reg, ARM_M4_MPU_REG_TYPE_IREGION));

	/* For now assume ARM-v7 semantics, non-shared, DREGION indicates */
	mpu_nregions = MS(reg, ARM_M4_MPU_REG_TYPE_DREGION);

	/* Enable default memory map for privileged access */
	mpu_config = ARM_M4_MPU_REG_CTRL_PRIVDEFENA;
}

/**
 * Enable the MPU.
 */
void
arm_m4_mpu_enable(void)
{
	if (mpu_nregions == 0)
		return;

	os_reg_write32(ARM_M4_MPU_BASE, ARM_M4_MPU_REG_CTRL,
	    mpu_config | ARM_M4_MPU_REG_CTRL_ENABLE);
}

/**
 * Disable the MPU.
 */
void
arm_m4_mpu_disable(void)
{
	if (mpu_nregions == 0)
		return;

	os_reg_write32(ARM_M4_MPU_BASE, ARM_M4_MPU_REG_CTRL, 0);
}

/**
 * Program in the given region.
 *
 * This does some basic sanity checking of the regions, memory alignment,
 * etc and will write both registers for you.
 *
 * This should be called with the MPU disabled.
 *
 * Yes, this is specifically very not optimised for performance.
 * Once things work, we can move to inlining these, doing bulk table
 * updates, only validating when we're populating the register contents
 * in order to write out here, etc.
 *
 *
 * @param[in] base_addr base address, without region/valid fields
 * @param[in] rasr_reg RASR register contents, pre-calculated
 * @retval 0 if ok, non-zero if error.
 */
int
arm_m4_mpu_program_region(uint32_t region, uint32_t base_addr,
    uint32_t rasr_reg)
{
	if (region >= mpu_nregions)
		return (-1);

	/* XXX TODO: verify base / address mask aren't insane */

	base_addr |= RMW(base_addr, ARM_M4_MPU_REG_RBAR_REGION, region);
	base_addr |= ARM_M4_MPU_REG_RBAR_VALID;

	os_reg_write32(ARM_M4_MPU_BASE, ARM_M4_MPU_REG_RBAR, base_addr);
	os_reg_write32(ARM_M4_MPU_BASE, ARM_M4_MPU_REG_RSAR, rasr_reg);

	return (0);
}
