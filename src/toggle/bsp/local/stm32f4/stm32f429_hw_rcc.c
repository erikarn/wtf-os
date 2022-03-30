
#include <stddef.h>
#include <stdint.h>

#include <os/reg.h>
#include <os/bitmask.h>
#include <hw/scb_defs.h>

#include <stm32f4/stm32f429_hw_map.h>
#include <stm32f4/stm32f429_hw_rcc_reg.h>

#include <stm32f4/stm32f429_hw_rcc.h>

static const uint8_t AHBPrescTable[16] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9 };

/*
 * This is the root clock / pll / peripheral configuration block
 * for the STM32F429 SoCs.
 */

/*
 * Hard-coded values for the stm32f429 discovery board.
 * Need to go and un-hardcode these and put them in a config file somewhere.
 */
#define	HSE_VALUE	8000000
#define	HSI_VALUE	16000000
#define	PLL_M		8
#define	PLL_Q		7
#define	PLL_N		360
#define	PLL_P		2

uint32_t
stm32f429_get_system_core_clock(void)
{
	uint32_t tmp = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;
	uint32_t SystemCoreClock = 0;

	/*
	 * Get the current system clock source (HSI, HSE, PLL)
	 */
	tmp = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CFGR);
	tmp = MS(tmp, STM32F429_RCC_REG_RCC_CFGR_SWS);

	switch (tmp)
	{
	case 0:
		/* HSI used as system clock source */
		SystemCoreClock = HSI_VALUE;
		break;
	case 1:
		/* HSE used as system clock source */
		SystemCoreClock = HSE_VALUE;
		break;
	case 2:
		/* PLL P used as system clock source */
		/*
		 * PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N
		 * SYSCLK = PLL_VCO / PLL_P
		 */
		tmp = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_PLLCFGR);
		pllsource = !! (tmp & STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC);
		pllm = MS(tmp, STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLM);

		if (pllsource != 0) {
			/* HSE used as PLL clock source */
			pllvco = (HSE_VALUE / pllm) *
			    MS(tmp, STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLN);
		} else {
			/* HSI used as PLL clock source */
			pllvco = (HSI_VALUE / pllm) *
			    MS(tmp, STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLN);
		}
		pllp = MS(tmp, STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLP) + 1;
		pllp = pllp * 2;
		SystemCoreClock = pllvco/pllp;
		break;
	default:
		SystemCoreClock = HSI_VALUE;
		break;
	}

	/* Get the HPRE (AHB prescaler) */
	tmp = AHBPrescTable[MS(os_reg_read32(RCC_BASE,
	    STM32F429_RCC_REG_RCC_CFGR),
	    STM32F429_RCC_REG_RCC_CFGR_HPRE)];

	/* Update system core clock w/ prescaler */
	SystemCoreClock >>= tmp;
	return SystemCoreClock;
}
