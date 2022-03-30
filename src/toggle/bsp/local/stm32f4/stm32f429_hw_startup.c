
#include <stddef.h>
#include <stdint.h>

#include <os/reg.h>
#include <os/bitmask.h>
#include <hw/scb_defs.h>

#include <stm32f4/stm32f429_hw_map.h>
#include <stm32f4/stm32f429_hw_rcc_reg.h>
#include <stm32f4/stm32f429_hw_pwr_reg.h>
#include <stm32f4/stm32f429_hw_flash_reg.h>

/*
 * This is the startup code specific for the STM32F429 part on the DISCO
 * board.  (I haven't tried it with the DISCO1 / DISC1 board, I'll have
 * to acquire one to test.)
 *
 * I've reimplemented the bare minimum here based on what the STM32 HAL
 * code did.  Notably during early bootstrap it doesn't try to do anything
 * fun through other driver modules, it just directly pokes all the
 * registers as needed.
 */


#if !defined (PLL_M)
 /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N */
 #define PLL_M      25
#endif /* PLL_M */

/* USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ */
#define PLL_Q      7

#define PLL_N      360
/* SYSCLK = PLL_VCO / PLL_P */
#define PLL_P      2

static void SetSysClock(void);

void SystemInit(void)
{
	uint32_t reg;

	/* Enable the FPU if it's present */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	reg = os_reg_read32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_CPACR);
	reg |= (3UL << 20); // CP10 full access
	reg |= (3UL << 22); // CP11 full access
	os_reg_write32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_CPACR, reg);
#endif

	/*
	 * Next up - reset the RCC clock selection / PLL configuration
	 * to a suitable power on setup.  Once it's setup on the internal
	 * oscillator the basic PLL bits will be programmed and then
	 * will eventually cut over to running off of the desired
	 * PLL configuration and oscillator source.
	 */

	/* Enable internal high speed oscillator */
	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CR);
	reg |= STM32F429_RCC_REG_RCC_CR_HSI_ON;
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_CR, reg);

	/* Reset RCC PLL configuration */
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_CFGR, 0);

	/* Clear HSE_ON, CSS_ON and PLL_ON bits */
	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CR);
	reg &= ~ (STM32F429_RCC_REG_RCC_CR_HSE_ON
	    | STM32F429_RCC_REG_RCC_CR_CSS_ON
	    | STM32F429_RCC_REG_RCC_CR_PLL_ON);
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_CR, reg);

	/* Reset PLLCFGR register to a fixed initial PLL config */
	/* XXX TODO: decompose these into the fields! */
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_PLLCFGR, 0x24003010);

	/* Clear HSE_BYP bit; don't bypass w/ the external clock */
	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CR);
	reg &= ~STM32F429_RCC_REG_RCC_CR_HSE_BYP;
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_CR, reg);

	/* Disable all RCC interrupts */
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_CIR, 0);

	/*
	 * Note: this is where the SRAM/SDRAM controller setup would be done.
	 *
	 * We don't have to do it this early in startup as right now we don't
	 * support booting up /into/ the external SRAM/SDRAM.  Instead, we'll
	 * complete booting in on-chip RAM and then bring up external SRAM/SDRAM
	 * as a device driver.
	 */

	/*
	 * Setup the system clocks with our static configuration for
	 * the STM32F429.
	 */
	SetSysClock();

	/*
	 * Configure the vector table to be at the given offset in flash.
	 * Right now we're assuming it's at the beginning of flash in the
	 * linker scripts/startup assembly.
	 */
	os_reg_write32(ARM_M4_SCB_REG_BASE, ARM_M4_SCB_REG_VTOR,
	    FLASH_BASE | 0);
}

/*
 * Setup the system clock for the STM32F429 (discovery board) using
 * the HSE (external) clock as the clock source, not the internal
 * clock.
 *
 * Yes eventually this should be made generic enough to work on
 * all configurations.
 */
static void
SetSysClock(void)
{
	uint32_t reg;
	int i;

	/** Flip on HSE */
	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CR);
	reg |= STM32F429_RCC_REG_RCC_CR_HSE_ON;
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_CR, reg);

	/* Wait until HSE is ready */
	for (i = 0; i < 32768; i++) {
		reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CR);
		if (reg & STM32F429_RCC_REG_RCC_CR_HSE_RDY)
			break;
	}

	/* If we didn't find ourselves ready then error out */
	if ((reg & STM32F429_RCC_REG_RCC_CR_HSE_RDY) == 0)
		goto error;

	/* Select regulator voltage, output scale 1 mode */
	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_APB1ENR);
	reg |= STM32F429_RCC_REG_RCC_APB1ENR_PWREN;
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_APB1ENR, reg);

	reg = os_reg_read32(PWR_BASE, STM32F429_PWR_REG_CR);
	reg = RMW(reg, STM32F429_PWR_REG_CR_VOS, 3);
	os_reg_write32(PWR_BASE, STM32F429_PWR_REG_CR, reg);

	/*
	 * XXX TODO; constants for these field values, they're
	 * not 1:1 numeric values.
	 */
	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CFGR);
	/* Set HCLK = SYSCLK / 1 */
	reg = RMW(reg, STM32F429_RCC_REG_RCC_CFGR_HPRE, 0);
	/* Set PCLK2 = HCLK / 2 */
	reg = RMW(reg, STM32F429_RCC_REG_RCC_CFGR_PPRE2, 4);
	/* Set PCLK1 = HCLK / 4 */
	reg = RMW(reg, STM32F429_RCC_REG_RCC_CFGR_PPRE1, 5);
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_CFGR, reg);

	/* Setup the main PLL with our board configuration */
	reg = RMW(0, STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLM, PLL_M);
	reg = RMW(reg, STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLN, PLL_N);
	reg = RMW(reg, STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLP,
	    (PLL_P >> 1) - 1);
	reg = RMW(reg, STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC_PLLQ, PLL_Q);
	/* HSE source for PLL/PLLI2S */
	reg |= STM32F429_RCC_REG_RCC_PLLCFGR_PLLSRC;
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_PLLCFGR, reg);

	/* Enable the main PLL */
	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CR);
	reg |= STM32F429_RCC_REG_RCC_CR_PLL_ON;
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_CR, reg);

	/* Wait until the main PLL is ready */
	while ((os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CR) &
	    STM32F429_RCC_REG_RCC_CR_PLL_RDY) == 0)
		;

	/* Enable over-drive to extend clock frequency to 180MHz */
	reg = os_reg_read32(PWR_BASE, STM32F429_PWR_REG_CR);
	reg |= STM32F429_PWR_REG_CR_ODEN;
	os_reg_write32(PWR_BASE, STM32F429_PWR_REG_CR, reg);

	while ((os_reg_read32(PWR_BASE, STM32F429_PWR_REG_CSR) &
	    STM32F429_PWR_REG_CSR_OD_RDY) == 0)
		;

	reg = os_reg_read32(PWR_BASE, STM32F429_PWR_REG_CR);
	reg |= STM32F429_PWR_REG_CR_ODSWEN;
	os_reg_write32(PWR_BASE, STM32F429_PWR_REG_CR, reg);

	while ((os_reg_read32(PWR_BASE, STM32F429_PWR_REG_CSR) &
	    STM32F429_PWR_REG_CSR_ODSW_RDY) == 0)
		;

	/*
	 * Flip on the flash prefetch, instruction/data cache and
	 * wait state config.  According to the manual (section 3.5.1,
	 * "Relation between CPU clock frequency and Flash memory
	 * read time" when increasing the clock frequency, we first
	 * program in the new target latency before we up the frequency.
	 */
	reg = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN;
	reg = RMW(reg, FLASH_ACR_LATENCY, 5); /* 5 wait states */
	os_reg_write32(FLASH_R_BASE, FLASH_ACR, reg);

	/*
	 * Select the main PLL as the system clock source
	 * (2 = PLL, XXX TODO make defines, not magic numbers)
	 */
	reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CFGR);
	reg = RMW(reg, STM32F429_RCC_REG_RCC_CFGR_SW, 2);
	os_reg_write32(RCC_BASE, STM32F429_RCC_REG_RCC_CFGR, reg);

	/* .. and now wait for it to be ready. */
	while (1) {
		reg = os_reg_read32(RCC_BASE, STM32F429_RCC_REG_RCC_CFGR);
		if (MS(reg, STM32F429_RCC_REG_RCC_CFGR_SWS) == 2)
			break;
	}

error:
	/*
	 * If we skipped here then we somehow ended up not being
	 * able to select the HSE clock as our PLL/system clock input.
	 */
	(void) 0;
}
