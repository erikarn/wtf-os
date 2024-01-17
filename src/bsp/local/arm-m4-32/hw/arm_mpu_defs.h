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

#ifndef	__ARM_MPU_DEFS_H__
#define	__ARM_MPU_DEFS_H__

#define	ARM_M4_MPU_BASE			0xe000ed90

#define	ARM_M4_MPU_MIN_REGION_SIZE		32

#define	ARM_M4_MPU_REG_TYPE		0x0000
#define		ARM_M4_MPU_REG_TYPE_SEPARATE		BIT_U32(0)
#define		ARM_M4_MPU_REG_TYPE_DREGION_M		0x0000ff00
#define		ARM_M4_MPU_REG_TYPE_DREGION_S		8
#define		ARM_M4_MPU_REG_TYPE_IREGION_M		0x00ff0000
#define		ARM_M4_MPU_REG_TYPE_IREGION_S		16

#define	ARM_M4_MPU_REG_CTRL		0x0004
#define		ARM_M4_MPU_REG_CTRL_ENABLE		BIT_U32(0)
#define		ARM_M4_MPU_REG_CTRL_HFNMIENA		BIT_U32(1)
#define		ARM_M4_MPU_REG_CTRL_PRIVDEFENA		BIT_U32(2)

#define	ARM_M4_MPU_REG_RNR		0x0008
#define		ARM_M4_MPU_REG_RNR_REGION_M		0x000000ff
#define		ARM_M4_MPU_REG_RNR_REGION_S		0

#define	ARM_M4_MPU_REG_RBAR		0x000c
#define		ARM_M4_MPU_REG_RBAR_REGION_M		0x00000007
#define		ARM_M4_MPU_REG_RBAR_REGION_S		0
#define		ARM_M4_MPU_REG_RBAR_VALID		BIT_U32(4)

#define	ARM_M4_MPU_REG_RSAR		0x0010
#define		ARM_M4_MPU_REG_RSAR_ENABLE		BIT_U32(0)
#define		ARM_M4_MPU_REG_RSAR_SIZE_M		0x0000003e
#define		ARM_M4_MPU_REG_RSAR_SIZE_S		1
#define		ARM_M4_MPU_REG_RSAR_SRD_M		0x0000ff00
#define		ARM_M4_MPU_REG_RSAR_SRD_S		8
#define		ARM_M4_MPU_REG_RSAR_B			BIT_U32(16)
#define		ARM_M4_MPU_REG_RSAR_C			BIT_U32(17)
#define		ARM_M4_MPU_REG_RSAR_S			BIT_U32(18)
#define		ARM_M4_MPU_REG_RSAR_TEX_M		0x00380000
#define		ARM_M4_MPU_REG_RSAR_TEX_S		19
#define		ARM_M4_MPU_REG_RSAR_AP_M		0x07000000
#define		ARM_M4_MPU_REG_RSAR_AP_S		24
#define		ARM_M4_MPU_REG_RSAR_XN			BIT_U32(28)

/*
 * Aliases of RBAR/RSAR, so you can memcpy() 4 regions at a time
 * from a pre-built table to this particular region, saving some
 * CPU cycles.
 */
#define	ARM_M4_MPU_REG_RBAR_A1		0x0014
#define	ARM_M4_MPU_REG_RSAR_A1		0x0018
#define	ARM_M4_MPU_REG_RBAR_A2		0x001c
#define	ARM_M4_MPU_REG_RSAR_A2		0x0020
#define	ARM_M4_MPU_REG_RBAR_A3		0x0024
#define	ARM_M4_MPU_REG_RSAR_A3		0x0028

#endif	/* __ARM_MPU_DEFS_H__ */
