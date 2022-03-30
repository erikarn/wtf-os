#ifndef	__STM32F4_HW_PWR_REG_H__
#define	__STM32F4_HW_PWR_REG_H__

#include <os/bit.h>

/*
 * These are the PWR (power block) registers for the STM32F42xxx
 * and STM32F43xxx.
 */

#define	STM32F429_PWR_REG_CR				0x000
#define		STM32F429_PWR_REG_CR_VOS_M		0x0000c000
#define		STM32F429_PWR_REG_CR_VOS_S		14
#define		STM32F429_PWR_REG_CR_ODEN		BIT_U32(16)
#define		STM32F429_PWR_REG_CR_ODSWEN		BIT_U32(17)

#define	STM32F429_PWR_REG_CSR				0x004
#define		STM32F429_PWR_REG_CSR_VOS_RDY		BIT_U32(14)
#define		STM32F429_PWR_REG_CSR_OD_RDY		BIT_U32(16)
#define		STM32F429_PWR_REG_CSR_ODSW_RDY		BIT_U32(17)


#endif	/* __STM32F4_HW_PWR_REG_H__ */
