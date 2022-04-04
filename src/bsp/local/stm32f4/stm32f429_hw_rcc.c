
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <os/reg.h>
#include <os/bitmask.h>
#include <hw/scb_defs.h>

#include <stm32f4/stm32f429_hw_map.h>
#include <stm32f4/stm32f429_hw_rcc_reg.h>

#include <stm32f4/stm32f429_hw_rcc.h>
#include <stm32f4/stm32f429_hw_rcc_table.h>

static const uint8_t AHBPrescTable[16] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9 };
static const uint8_t APBPrescTable[16] =
    { 0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9 };

/*
 * This is the root clock / pll / peripheral configuration block
 * for the STM32F429 SoCs.
 *
 * There's plenty of extra fun stuff to implement here when one
 * wishes to start playing with all the more complicated peripherals
 * that need variable clocks (like i2s), and some of the more fun
 * lower power modes and clock sources.  For now however, let's
 * focus on bootstrapping.
 */

/*
 * Hard-coded values for the stm32f429 discovery board.
 * XXX TODO Need to go and un-hardcode these and put them in a config file!
 */
#define	HSE_VALUE	8000000
#define	HSI_VALUE	16000000
#define	PLL_M		8
#define	PLL_Q		7
#define	PLL_N		360
#define	PLL_P		2

/**
 * Get the system core clock in Hz.
 *
 * @retval Core clock frequency in Hz.
 */
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

	/* Update system core clock (HCLK) w/ prescaler */
	SystemCoreClock >>= tmp;
	return SystemCoreClock;
}

/**
 * Get PCLK1 frequency (AHB1).
 *
 * @retval Frequency in Hz.
 */
uint32_t
stm32f429_rcc_get_pclk1_freq(void)
{
	uint32_t hclk_freq, reg, pre;

	hclk_freq = stm32f429_get_system_core_clock();

	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CFGR);
	pre = APBPrescTable[MS(reg, STM32F429_RCC_REG_RCC_CFGR_PPRE1)];

	return (hclk_freq >> pre);
}

/**
 * Get PCLK2 frequency (AHB2).
 *
 * @retval Frequency in Hz.
 */
uint32_t
stm32f429_rcc_get_pclk2_freq(void)
{
	uint32_t hclk_freq, reg, pre;

	hclk_freq = stm32f429_get_system_core_clock();

	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CFGR);
	pre = APBPrescTable[MS(reg, STM32F429_RCC_REG_RCC_CFGR_PPRE2)];

	return (hclk_freq >> pre);
}


/**
 * Enable/disable the given peripheral clock.
 *
 * @param[in] periph Peripheral ID
 * @retval true if successful, false if invalid peripheral ID
 */
bool
stm32f429_rcc_peripheral_enable(stm32f429_rcc_peripheral_name_t periph,
    bool enable)
{
	const struct stm32f429_rcc_table_entry *pe;
	uint32_t reg;

	pe = stm32f429_rcc_table_get_peripheral(periph);
	if (pe == NULL) {
		return (false);
	}

	reg = os_reg_read32(RCC_BASE, pe->rcc_clken_reg);
	if (enable) {
		reg |= pe->rcc_clken_mask;
	} else {
		reg &= ~pe->rcc_clken_mask;
	}
	os_reg_write32(RCC_BASE, pe->rcc_clken_reg, reg);

	return (true);
}

/**
 * Reset the given peripheral.
 */
bool
stm32f429_rcc_peripheral_reset(stm32f429_rcc_peripheral_name_t periph,
    bool reset)
{
	/* XXX TODO */
	return (false);
}

/**
 * Put the given peripheral into a low power state.
 */
