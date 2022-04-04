#ifndef	__STM32F4_HW_RCC_REG_H__
#define	__STM32F4_HW_RCC_REG_H__

#include <os/bit.h>

/*
 * These are the RCC registers for the STM32F42xxx
 * and STM32F43xxx.
 */

#define	STM32F429_RCC_REG_RCC_CR			0x000
#define		STM32F429_RCC_REG_RCC_CR_HSI_ON		BIT_U32(0)
#define		STM32F429_RCC_REG_RCC_CR_HSI_RDY	BIT_U32(1)
#define		STM32F429_RCC_REG_RCC_CR_HSE_ON		BIT_U32(16)
#define		STM32F429_RCC_REG_RCC_CR_HSE_RDY	BIT_U32(17)
#define		STM32F429_RCC_REG_RCC_CR_HSE_BYP	BIT_U32(18)
#define		STM32F429_RCC_REG_RCC_CR_CSS_ON		BIT_U32(19)
#define		STM32F429_RCC_REG_RCC_CR_PLL_ON		BIT_U32(24)
#define		STM32F429_RCC_REG_RCC_CR_PLL_RDY	BIT_U32(25)
#define		STM32F429_RCC_REG_RCC_CR_PLLI2S_ON	BIT_U32(26)
#define		STM32F429_RCC_REG_RCC_CR_PLLI2S_RDY	BIT_U32(27)
#define		STM32F429_RCC_REG_RCC_CR_PLLSAI_ON	BIT_U32(28)
#define		STM32F429_RCC_REG_RCC_CR_PLLSAI_RDY	BIT_U32(29)

#define	STM32F429_RCC_REG_RCC_PLLCFGR				0x004
#define		STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLM_M	0x0000003f
#define		STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLM_S	0
#define		STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLN_M	0x00007fc0
#define		STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLN_S	6
#define		STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLP_M	0x00030000
#define		STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLP_S	16
#define		STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC		BIT_U32(22)
#define		STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLQ_M	0x0f000000
#define		STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLQ_S	24

#define	STM32F429_RCC_REG_RCC_CFGR		0x008
#define		STM32F429_RCC_REG_RCC_CFGR_SW_M		0x00000003
#define		STM32F429_RCC_REG_RCC_CFGR_SW_S		0
#define		STM32F429_RCC_REG_RCC_CFGR_SWS_M		0x0000000c
#define		STM32F429_RCC_REG_RCC_CFGR_SWS_S		2
#define		STM32F429_RCC_REG_RCC_CFGR_HPRE_M		0x000000f0
#define		STM32F429_RCC_REG_RCC_CFGR_HPRE_S		4
#define		STM32F429_RCC_REG_RCC_CFGR_PPRE1_M		0x00001c00
#define		STM32F429_RCC_REG_RCC_CFGR_PPRE1_S		10
#define		STM32F429_RCC_REG_RCC_CFGR_PPRE2_M		0x0000e000
#define		STM32F429_RCC_REG_RCC_CFGR_PPRE2_S		13

#define	STM32F429_RCC_REG_RCC_CIR		0x00c

#define	STM32F429_RCC_REG_RCC_AHB1RSTR		0x010
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOA_RST	BIT_U32(0)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOB_RST	BIT_U32(1)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOC_RST	BIT_U32(2)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOD_RST	BIT_U32(3)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOE_RST	BIT_U32(4)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOF_RST	BIT_U32(5)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOG_RST	BIT_U32(6)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOH_RST	BIT_U32(7)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOI_RST	BIT_U32(8)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOJ_RST	BIT_U32(9)
#define		STM32F429_RCC_REG_RCC_AHB1RSTR_GPIOK_RST	BIT_U32(10)

#define	STM32F429_RCC_REG_RCC_AHB2RSTR		0x014
#define	STM32F429_RCC_REG_RCC_AHB3RSTR		0x018

#define	STM32F429_RCC_REG_RCC_APB1RSTR		0x020
#define	STM32F429_RCC_REG_RCC_APB2RSTR				0x024
#define		STM32F429_RCC_REG_RCC_APB2RSTR_USART1		BIT_U32(4)

#define	STM32F429_RCC_REG_RCC_AHB1ENR				0x030
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOA_EN		BIT_U32(0)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOB_EN		BIT_U32(1)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOC_EN		BIT_U32(2)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOD_EN		BIT_U32(3)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOE_EN		BIT_U32(4)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOF_EN		BIT_U32(5)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOG_EN		BIT_U32(6)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOH_EN		BIT_U32(7)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOI_EN		BIT_U32(8)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOJ_EN		BIT_U32(9)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_GPIOK_EN		BIT_U32(10)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_CRC_EN		BIT_U32(12)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_BKPSRAM_EN	BIT_U32(18)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_CCMDATARAM_EN	BIT_U32(20)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_DMA1_EN		BIT_U32(21)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_DMA2_EN		BIT_U32(22)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_DMA2D_EN		BIT_U32(23)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_ETH_MAC_EN	BIT_U32(25)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_ETH_MAC_TX_EN	BIT_U32(26)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_ETH_MAC_RX_EN	BIT_U32(27)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_ETH_MAC_PTP_EN	BIT_U32(28)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_OTG_HS_EN		BIT_U32(29)
#define		STM32F429_RCC_REG_RCC_AHB1ENR_OTG_HSULPIE_EN	BIT_U32(30)

#define	STM32F429_RCC_REG_RCC_AHB2ENR				0x034
#define		STM32F429_RCC_REG_RCC_AHB2ENR_DCMI_EN		BIT_U32(0)
#define		STM32F429_RCC_REG_RCC_AHB2ENR_CRYP_EN		BIT_U32(4)
#define		STM32F429_RCC_REG_RCC_AHB2ENR_HASH_EN		BIT_U32(5)
#define		STM32F429_RCC_REG_RCC_AHB2ENR_RNG_EN		BIT_U32(6)
#define		STM32F429_RCC_REG_RCC_AHB2ENR_OTG_FS_EN		BIT_U32(7)

#define	STM32F429_RCC_REG_RCC_AHB3ENR				0x038
#define		STM32F429_RCC_REG_RCC_AHB3ENR_FMC_EN		BIT_U32(0)

#define	STM32F429_RCC_REG_RCC_APB1ENR				0x040
#define		STM32F429_RCC_REG_RCC_APB1ENR_TIM2_EN		BIT_U32(0)
#define		STM32F429_RCC_REG_RCC_APB1ENR_TIM3_EN		BIT_U32(1)
#define		STM32F429_RCC_REG_RCC_APB1ENR_TIM4_EN		BIT_U32(2)
#define		STM32F429_RCC_REG_RCC_APB1ENR_TIM5_EN		BIT_U32(3)
#define		STM32F429_RCC_REG_RCC_APB1ENR_TIM6_EN		BIT_U32(4)
#define		STM32F429_RCC_REG_RCC_APB1ENR_TIM7_EN		BIT_U32(5)
#define		STM32F429_RCC_REG_RCC_APB1ENR_TIM12_EN		BIT_U32(6)
#define		STM32F429_RCC_REG_RCC_APB1ENR_TIM13_EN		BIT_U32(7)
#define		STM32F429_RCC_REG_RCC_APB1ENR_TIM14_EN		BIT_U32(8)
#define		STM32F429_RCC_REG_RCC_APB1ENR_WWDG_EN		BIT_U32(11)
#define		STM32F429_RCC_REG_RCC_APB1ENR_SPI2_EN		BIT_U32(14)
#define		STM32F429_RCC_REG_RCC_APB1ENR_SPI3_EN		BIT_U32(15)
#define		STM32F429_RCC_REG_RCC_APB1ENR_USART2_EN		BIT_U32(17)
#define		STM32F429_RCC_REG_RCC_APB1ENR_USART3_EN		BIT_U32(18)
#define		STM32F429_RCC_REG_RCC_APB1ENR_UART4_EN		BIT_U32(19)
#define		STM32F429_RCC_REG_RCC_APB1ENR_UART5_EN		BIT_U32(20)
#define		STM32F429_RCC_REG_RCC_APB1ENR_I2C1_EN		BIT_U32(21)
#define		STM32F429_RCC_REG_RCC_APB1ENR_I2C2_EN		BIT_U32(22)
#define		STM32F429_RCC_REG_RCC_APB1ENR_I2C3_EN		BIT_U32(23)
#define		STM32F429_RCC_REG_RCC_APB1ENR_CAN1_EN		BIT_U32(25)
#define		STM32F429_RCC_REG_RCC_APB1ENR_CAN2_EN		BIT_U32(26)
#define		STM32F429_RCC_REG_RCC_APB1ENR_PWREN		BIT_U32(28)
#define		STM32F429_RCC_REG_RCC_APB1ENR_DAC_EN		BIT_U32(29)
#define		STM32F429_RCC_REG_RCC_APB1ENR_UART7_EN		BIT_U32(30)
#define		STM32F429_RCC_REG_RCC_APB1ENR_UART8_EN		BIT_U32(31)

#define	STM32F429_RCC_REG_RCC_APB2ENR		0x044
#define		STM32F429_RCC_REG_RCC_APB2ENR_TIM1_EN		BIT_U32(0)
#define		STM32F429_RCC_REG_RCC_APB2ENR_TIM8_EN		BIT_U32(1)
#define		STM32F429_RCC_REG_RCC_APB2ENR_USART1		BIT_U32(4)
#define		STM32F429_RCC_REG_RCC_APB2ENR_USART6_EN		BIT_U32(6)
#define		STM32F429_RCC_REG_RCC_APB2ENR_ADC1_EN		BIT_U32(8)
#define		STM32F429_RCC_REG_RCC_APB2ENR_ADC2_EN		BIT_U32(9)
#define		STM32F429_RCC_REG_RCC_APB2ENR_ADC3_EN		BIT_U32(10)
#define		STM32F429_RCC_REG_RCC_APB2ENR_SDIO_EN		BIT_U32(11)
#define		STM32F429_RCC_REG_RCC_APB2ENR_SPI1_EN		BIT_U32(12)
#define		STM32F429_RCC_REG_RCC_APB2ENR_SPI4_EN		BIT_U32(13)
#define		STM32F429_RCC_REG_RCC_APB2ENR_SYSCFG_EN		BIT_U32(14)
#define		STM32F429_RCC_REG_RCC_APB2ENR_TIM9_EN		BIT_U32(16)
#define		STM32F429_RCC_REG_RCC_APB2ENR_TIM10_EN		BIT_U32(17)
#define		STM32F429_RCC_REG_RCC_APB2ENR_TIM11_EN		BIT_U32(18)
#define		STM32F429_RCC_REG_RCC_APB2ENR_SPI5_EN		BIT_U32(20)
#define		STM32F429_RCC_REG_RCC_APB2ENR_SPI6_EN		BIT_U32(21)
#define		STM32F429_RCC_REG_RCC_APB2ENR_SAI1_EN		BIT_U32(22)
#define		STM32F429_RCC_REG_RCC_APB2ENR_LTDC_EN		BIT_U32(26)

#define	STM32F429_RCC_REG_RCC_AHB1LPENR		0x050
#define	STM32F429_RCC_REG_RCC_AHB2LPENR		0x054
#define	STM32F429_RCC_REG_RCC_AHB3LPENR		0x058

#define	STM32F429_RCC_REG_RCC_APB1LPENR		0x060
#define	STM32F429_RCC_REG_RCC_APB2LPENR		0x064

#define	STM32F429_RCC_REG_RCC_BDCR		0x070
#define	STM32F429_RCC_REG_RCC_CSR		0x074
#define	STM32F429_RCC_REG_RCC_SSCGR		0x080
#define	STM32F429_RCC_REG_RCC_PLLI2SCFGR	0x084
#define	STM32F429_RCC_REG_RCC_PLLSAICFGR	0x088
#define	STM32F429_RCC_REG_RCC_DCKCFGR		0x08c


#endif	/* __STM32F4_HW_RCC_REG_H__ */