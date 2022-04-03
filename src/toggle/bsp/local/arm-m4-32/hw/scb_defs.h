#ifndef	__ARM_SCB_DEFS_H__
#define	__ARM_SCB_DEFS_H__

/*
 * Definitions of System Control Block (SCB).
 *
 * These are based on the STM32F4 Cortex-M4 documentation
 * (PM0214), Rev 10, Table 50.
 */
#include <os/bit.h>


#define	ARM_M4_SCB_REG_BASE		0xe000e000

#define	ARM_M4_SCB_REG_ACTLR		0x008
#define	ARM_M4_SCB_REG_CPUID		0xd00

#define	ARM_M4_SCB_REG_ICSR			0xd04
#define		ARM_M4_SCB_REG_ICSR_PENDSVCLR	BIT_U32(27)
#define		ARM_M4_SCB_REG_ICSR_PENDSVSET	BIT_U32(28)

#define	ARM_M4_SCB_REG_VTOR		0xd08
#define	ARM_M4_SCB_REG_AIRCR		0xd0c
#define	ARM_M4_SCB_REG_SCR		0xd10
#define	ARM_M4_SCB_REG_CCR		0xd14
#define	ARM_M4_SCB_REG_SHPR1		0xd18
#define	ARM_M4_SCB_REG_SHPR2		0xd1c
#define	ARM_M4_SCB_REG_SHPR3		0xd20
#define	ARM_M4_SCB_REG_SHCSR		0xd24
#define	ARM_M4_SCB_REG_CFSR		0xd28
#define	ARM_M4_SCB_REG_HFSR		0xd2c
#define	ARM_M4_SCB_REG_MMAR		0xd34
#define	ARM_M4_SCB_REG_BFAR		0xd38
#define	ARM_M4_SCB_REG_AFSR		0xd3c

/*
 * Floating point coprocessor region. (Table 56.)
 */
#define	ARM_M4_SCB_REG_CPACR		0xd88
#define	ARM_M4_SCB_REG_FPCCR		0xf34
#define	ARM_M4_SCB_REG_FPCAR		0xf38
#define	ARM_M4_SCB_REG_FPDCSR		0xf3c

#endif	/* __ARM_SCB_DEFS_H__ */
