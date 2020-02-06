#ifndef	__STM32F429_HW_FLASH_REG_H__
#define	__STM32F429_HW_FLASH_REG_H__

// TODO: bit macros

#define	FLASH_ACR		0x00000000 /* access control */
#define		FLASH_ACR_PRFTEN	(1 << 8) /* prefetch */
#define		FLASH_ACR_ICEN		(1 << 9) /* instruction cache */
#define		FLASH_ACR_DCEN		(1 << 10) /* data cache */
#define		FLASH_ACR_ICRST		(1 << 11) /* reset flash icache */
#define		FLASH_ACR_DCRST		(1 << 12) /* reset flash dcache */

#define	FLASH_KEYR		0x00000004 /* key */
#define	FLASH_OPTKEYR		0x00000008 /* option key */
#define	FLASH_SR		0x0000000c /* status register */
#define	FLASH_CR		0x00000010 /* control register */
#define	FLASH_OPTCR		0x00000014 /* option control register */
#define	FLASH_OPTCR1		0x00000018 /* option control register 1 */

#endif	/* __STM32F429_HW_FLASH_H__ */
