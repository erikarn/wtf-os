#include <stdint.h>

#include "../os/reg.h"
#include "stm32f429_hw_map.h"
#include "stm32f429_hw_flash_reg.h"
#include "stm32f429_hw_flash.h"

void
stm32f429_hw_flash_enable_icache(void)
{
	uint32_t reg;

	reg = os_reg_read32(FLASH_R_BASE, FLASH_ACR);
	reg |= FLASH_ACR_ICEN;
	os_reg_write32(FLASH_R_BASE, FLASH_ACR, reg);
}

void
stm32f429_hw_flash_enable_dcache(void)
{
	uint32_t reg;

	reg = os_reg_read32(FLASH_R_BASE, FLASH_ACR);
	reg |= FLASH_ACR_DCEN;
	os_reg_write32(FLASH_R_BASE, FLASH_ACR, reg);
}

void
stm32f429_hw_flash_disable_icache(void)
{
}

void
stm32f429_hw_flash_disable_dcache(void)
{
}

void
stm32f429_hw_flash_reset_icache(void)
{
}

void
stm32f429_hw_flash_reset_dcache(void)
{
}
