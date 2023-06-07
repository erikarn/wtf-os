#ifndef	__STM32F429_HW_FMC_REG_H__
#define	__STM32F429_HW_FMC_REG_H__


/*
 * TODO item for the rest of it
 *
 * + Enable GPIOB, C, D, E, F, G interfaces
 * + GPIO config for alt mode for SDRAM
 * + FMC config in RCC
 * + FMC configuration for banks
 * + Sending SDRAM init commands
 * + PALL
 * + Auto refresh
 * + MRD register
 * + Refresh count
 * + Disable write protection
 * + and how to test it!
 */

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

/* control register 0, 1 */
#define	FMC_REG_SDCR1			0x140
#define	FMC_REG_SDCR2			0x144

/* timing register 1, 2 */
#define	FMC_REG_SDTR1			0x148
#define	FMC_REG_SDTR2			0x14c

/* command register */
#define	FMC_REG_SDCMR			0x150

#define		FMC_REG_SDCMR_MODE_M	0x00000007
#define		FMC_REG_SDCMR_MODE_S	2
#define		FMC_REG_SDCMR_CTB2	BIT_U32(3)
#define		FMC_REG_SDCMR_CTB1	BIT_U32(4)
#define			FMC_REG_SDCMR_MODE_NORMAL	0
#define			FMC_REG_SDCMR_MODE_CMD_CLK_EN	1
#define			FMC_REG_SDCMR_MODE_CMD_PALL	2
#define			FMC_REG_SDCMR_MODE_CMD_AUTO_REFRESH	3
#define			FMC_REG_SDCMR_MODE_CMD_LOAD	4
#define			FMC_REG_SDCMR_MODE_CMD_SELF_REFRESH	5
#define			FMC_REG_SDCMR_MODE_CMD_POWER_DOWN	6
#define		FMC_REG_SDCMR_NRFS_M	0x000001c0
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
