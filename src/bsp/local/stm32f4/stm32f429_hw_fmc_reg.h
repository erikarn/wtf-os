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
#ifndef	__STM32F429_HW_FMC_REG_H__
#define	__STM32F429_HW_FMC_REG_H__

/*
 * Memory Map for the various possible hookups:
 *
 * Bank 1 - 0x60000000 -> 0x6fffffff - NOR/PSRAM/SRAM
 * Bank 2 - 0x70000000 -> 0x7fffffff - NAND
 * Bank 3 - 0x80000000 -> 0x8fffffff - NAND
 * Bank 4 - 0x90000000 -> 0x9fffffff - PC Card
 * Bank 5 - 0xc0000000 -> 0xcfffffff - SDRAM
 * Bank 6 - 0xd0000000 -> 0xdfffffff - SDRAM
 */

/*
 * SDRAM controller registers
 */

/* control register 1, 2 */
#define	FMC_REG_SDCR1			0x140
#define	FMC_REG_SDCR2			0x144
#define		FMC_REG_SDCR_NC_M	0x00000003
#define		FMC_REG_SDCR_NC_S	0
#define		FMC_REG_SDCR_NR_M	0x0000000c
#define		FMC_REG_SDCR_NR_S	2
#define		FMC_REG_SDCR_MWID_M	0x00000030
#define		FMC_REG_SDCR_MWID_S	4
#define		FMC_REG_SDCR_NB		BIT_U32(6)
#define		FMC_REG_SDCR_CAS_M	0x00000180
#define		FMC_REG_SDCR_CAS_S	7
#define		FMC_REG_SDCR_WP		BIT_U32(9)
#define		FMC_REG_SDCR_SDCLK_M	0x00000c00
#define		FMC_REG_SDCR_SDCLK_S	10
#define		FMC_REG_SDCR_RBURST	BIT_U32(12)
#define		FMC_REG_SDCR_RPIPE_M	0x00006000
#define		FMC_REG_SDCR_RPIPE_S	13


/* timing register 1, 2 */
#define	FMC_REG_SDTR1			0x148
#define	FMC_REG_SDTR2			0x14c
#define		FMC_REG_SDTR_TMRD_M	0x0000000f
#define		FMC_REG_SDTR_TMRD_S	0
#define		FMC_REG_SDTR_TXSR_M	0x000000f0
#define		FMC_REG_SDTR_TXSR_S	4
#define		FMC_REG_SDTR_TRAS_M	0x00000f00
#define		FMC_REG_SDTR_TRAS_S	8
#define		FMC_REG_SDTR_TRC_M	0x0000f000
#define		FMC_REG_SDTR_TRC_S	12
#define		FMC_REG_SDTR_TWR_M	0x000f0000
#define		FMC_REG_SDTR_TWR_S	16
#define		FMC_REG_SDTR_TRP_M	0x00f00000
#define		FMC_REG_SDTR_TRP_S	20
#define		FMC_REG_SDTR_TRCD_M	0x0f000000
#define		FMC_REG_SDTR_TRCD_S	24

/* command register */
#define	FMC_REG_SDCMR			0x150
#define		FMC_REG_SDCMR_MODE_M	0x00000007
#define		FMC_REG_SDCMR_MODE_S	0
#define		FMC_REG_SDCMR_CTB2	BIT_U32(3)
#define		FMC_REG_SDCMR_CTB1	BIT_U32(4)
#define			FMC_REG_SDCMR_MODE_NORMAL	0
#define			FMC_REG_SDCMR_MODE_CMD_CLK_EN	1
#define			FMC_REG_SDCMR_MODE_CMD_PALL	2
#define			FMC_REG_SDCMR_MODE_CMD_AUTO_REFRESH	3
#define			FMC_REG_SDCMR_MODE_CMD_LOAD	4
#define			FMC_REG_SDCMR_MODE_CMD_SELF_REFRESH	5
#define			FMC_REG_SDCMR_MODE_CMD_POWER_DOWN	6
#define		FMC_REG_SDCMR_NRFS_M	0x000001e0
#define		FMC_REG_SDCMR_NRFS_S	5
#define		FMC_REG_SDCMR_MRD_M	0x003ffe00
#define		FMC_REG_SDCMR_MRD_S	9

/* refresh timer register */
#define	FMC_REG_SDRTR			0x154
#define		FMC_REG_SDRTR_CRE	BIT_U32(0)
#define		FMC_REG_SDRTR_COUNT_M	0x00003ffe
#define		FMC_REG_SDRTR_COUNT_S	1
#define		FMC_REG_SDRTR_REIE	BIT_U32(14)

/* status register */
#define	FMC_REG_SDSR			0x158

#endif	/* __STM32F429_HW_FMC_REG_H__ */
