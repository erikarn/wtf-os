
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <os/bit.h>
#include <os/reg.h>

#include <kern/console/console.h>

#include "hw/types.h"

#include "stm32f429_hw_map.h"

#include "stm32f429_hw_gpio_reg.h"
#include "stm32f429_hw_gpio.h"

/**
 * TODO: maybe break this out into the header file and inline it
 * so the compiler can have a crack at inlining the mapping?
 * Maybe?
 */
static paddr_t
stm32f429_hw_gpio_block_to_base(stm32f429_hw_gpio_block_t block)
{
	switch (block) {
	case STM32F429_HW_GPIO_BLOCK_GPIOA:
		return GPIOA_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOB:
		return GPIOB_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOC:
		return GPIOC_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOD:
		return GPIOD_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOE:
		return GPIOE_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOF:
		return GPIOF_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOG:
		return GPIOG_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOH:
		return GPIOH_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOI:
		return GPIOI_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOJ:
		return GPIOJ_BASE;
	case STM32F429_HW_GPIO_BLOCK_GPIOK:
		return GPIOK_BASE;
	default:
		return (0);
	}
}

/**
 * Configure the given GPIO configuration with the power-on
 * defaults.
 *
 * @param[in] config GPIO paramter block to configure.
 */
void
stm32f429_hw_gpio_config_init(struct stm32f429_hw_gpio_pin_config *config)
{
	config->mode = STM32F429_HW_GPIO_PIN_MODE_INPUT;
	config->speed = STM32F429_HW_GPIO_PIN_SPEED_LOW;
	config->out_type = STM32F429_HW_GPIO_PIN_OUTPUT_TYPE_PUPD;
	config->out_pupd = STM32F429_HW_GPIO_PIN_PUPD_NONE;
	config->out_value = 0;
}

void
stm32f429_hw_gpio_config_get(stm32f429_hw_gpio_block_t block, int pin,
    struct stm32f429_hw_gpio_pin_config *config)
{
	/* XXX TODO - i don't need this yet! */
}

void
stm32f429_hw_gpio_config_set(stm32f429_hw_gpio_block_t block, int pin,
    const struct stm32f429_hw_gpio_pin_config *config)
{
	uint32_t reg;
	paddr_t base;

	base = stm32f429_hw_gpio_block_to_base(block);
	if (base == 0) {
		return;
	}

	/* If the mode is output, set the output value here first */
	if (config->mode == STM32F429_HW_GPIO_PIN_MODE_OUTPUT) {
		stm32f429_hw_gpio_set_pin(block, pin, config->out_value);
	}

	/*
	 * XXX TODO: get rid of magic values here!
	 */

	/* Mode */
	reg = os_reg_read32(base, STM32F429_HW_GPIO_REG_MODER);
	reg &= ~ (0x3 << (pin * 2));
	reg |= (config->mode & 0x3) << (pin * 2);
	os_reg_write32(base, STM32F429_HW_GPIO_REG_MODER, reg);

	/* If we're output or AF, setup other stuff */
	if ((config->mode == STM32F429_HW_GPIO_PIN_MODE_OUTPUT) ||
	    (config->mode == STM32F429_HW_GPIO_PIN_MODE_ALTERNATE)) {
		/* Speed */
		reg = os_reg_read32(base, STM32F429_HW_GPIO_REG_OSPEEDR);
		reg &= ~ (0x3 << (pin * 2));
		reg |= (config->speed & 0x3) << (pin * 2);
		os_reg_write32(base, STM32F429_HW_GPIO_REG_OSPEEDR, reg);

		/* Output mode */
		reg = os_reg_read32(base, STM32F429_HW_GPIO_REG_OTYPER);
		reg &= ~(BIT_U32(pin));
		if (config->out_type == STM32F429_HW_GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN) {
			reg |= BIT_U32(pin);
		}
		os_reg_write32(base, STM32F429_HW_GPIO_REG_OTYPER, reg);
	}

	/* Pull-up / down resistor config */
	reg = os_reg_read32(base, STM32F429_HW_GPIO_REG_PUPDR);
	reg &= ~ (0x3 << (pin * 2));
	reg |= ((config->out_pupd) << (pin * 2));
	os_reg_write32(base, STM32F429_HW_GPIO_REG_PUPDR, reg);
}

void
stm32f429_hw_gpio_alternate_set(stm32f429_hw_gpio_block_t block, int pin,
    uint32_t alt)
{
	uint32_t reg;
	paddr_t base;

	base = stm32f429_hw_gpio_block_to_base(block);
	if (base == 0) {
		return;
	}

	if (pin < 8) {
		/* 0..7 */
		reg = os_reg_read32(base, STM32F429_HW_GPIO_REG_AFRL);
		reg &= ~ (0xf << (pin * 4));
		reg |= ((alt & 0xf) << (pin * 4));
		os_reg_write32(base, STM32F429_HW_GPIO_REG_AFRL, reg);
	} else {
		/* 8..15 */
		reg = os_reg_read32(base, STM32F429_HW_GPIO_REG_AFRH);
		reg &= ~ (0xf << ((pin - 8) * 4));
		reg |= ((alt & 0xf) << ((pin - 8) * 4));
		os_reg_write32(base, STM32F429_HW_GPIO_REG_AFRH, reg);
	}
}

/**
 * Get all of the input pin states of the given GPIO block.
 *
 * @param[in] block GPIO block
 * @retval bitmap of GPIO pin state
 */
uint32_t
stm32f429_hw_gpio_get_block(stm32f429_hw_gpio_block_t block)
{
	paddr_t base;

	base = stm32f429_hw_gpio_block_to_base(block);
	if (base == 0) {
		return (0);
	}

	return (os_reg_read32(base, STM32F429_HW_GPIO_REG_IDR));
}

/**
 * Set all of the output pin states of the given GPIO block.
 *
 * @param[in] block GPIO block
 * @param[in] val state for all pins
 */
void
stm32f429_hw_gpio_set_block(stm32f429_hw_gpio_block_t block, uint32_t val)
{
	paddr_t base;

	base = stm32f429_hw_gpio_block_to_base(block);
	if (base == 0) {
		return;
	}

	os_reg_write32(base, STM32F429_HW_GPIO_REG_ODR, val);
}

void
stm32f429_hw_gpio_reset_block(stm32f429_hw_gpio_block_t block)
{
	/* XXX TODO */
	/*
	 * Yeah, it's todo because I don't want to write another mapping
	 * function between the GPIO block labels and the RCC GPIO peripheral
	 * labels.
	 *
	 * Instead I'll wait until I need this, and/or I get enough of
	 * the BSP up and working to move the RCC peripherals labels to
	 * a top-level stm32f4 BSP peripheral label, so I can cleanly
	 * use them here without /another/ mapping.
	 */
}

bool
stm32f429_hw_gpio_get_pin(stm32f429_hw_gpio_block_t block, int pin)
{
	paddr_t base;

	base = stm32f429_hw_gpio_block_to_base(block);
	if (base == 0) {
		return (0);
	}

	return !! (os_reg_read32(base, STM32F429_HW_GPIO_REG_IDR) &
	    BIT_U32(pin));
}

/**
 * Set the given pin.
 *
 * @param[in] block GPIO block
 * @param[in] pin GPIO pin
 * @param[in] val true if high, false if low
 */
void
stm32f429_hw_gpio_set_pin(stm32f429_hw_gpio_block_t block, int pin, bool val)
{
	paddr_t base;

	base = stm32f429_hw_gpio_block_to_base(block);
	if (base == 0) {
		return;
	}

	if (val) {
		os_reg_write32(base, STM32F429_HW_GPIO_REG_BSRR,
		    BIT_U32(pin));
	} else {
		os_reg_write32(base, STM32F429_HW_GPIO_REG_BSRR,
		    BIT_U32(pin + 16));
	}
}

/**
 * Toggle the given pin.
 *
 * @param[in] block GPIO block
 * @param[in] pin GPIO pin
 * @retval -1 on error; the current pin state after toggling (1 or 0).
 */
int
stm32f429_hw_gpio_toggle_pin(stm32f429_hw_gpio_block_t block, int pin)
{
	paddr_t base;
	uint32_t reg;

	base = stm32f429_hw_gpio_block_to_base(block);
	if (base == 0) {
		return -1;
	}

	reg = os_reg_read32(base, STM32F429_HW_GPIO_REG_ODR);
	reg ^= (BIT_U32(pin));
	os_reg_write32(base, STM32F429_HW_GPIO_REG_ODR, reg);

	return !! (reg & BIT_U32(pin));
}
