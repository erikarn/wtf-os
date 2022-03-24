#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#include "bsp/local/os/reg.h"
#include "bsp/local/stm32f4/stm32f429_hw_flash.h"
#include "bsp/local/stm32f4/stm32f429_hw_usart_reg.h"

#define GPIO_Pin_n0 GPIO_Pin_13
#define GPIO_Pin_n1 GPIO_Pin_14

static void
setup_usart1_gpios(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Enable clock for GPIOA
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    /**
    * Tell pins PA9 and PA10 which alternating function you will use
    * @important Make sure, these lines are before pins configuration!
    */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    // Initialize pins as alternating function
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void
setup_usart1(void)
{
    /**
     * Enable clock for USART1 peripheral
     */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /* XXX TODO: USART1 setup itself */
}

/* Setup LED GPIOs */
static void
setup_led_gpios(void)
{
    GPIO_InitTypeDef GPIO_InitDef;

    // Enable caches when fetching to/from flash
    stm32f429_hw_flash_enable_icache();
    stm32f429_hw_flash_enable_dcache();

    // Setup GPIO pins for LEDs
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitDef.GPIO_Pin = GPIO_Pin_n0;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOG, &GPIO_InitDef);

    GPIO_InitDef.GPIO_Pin = GPIO_Pin_n1;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOG, &GPIO_InitDef);
}

static void
setup_button_gpios(void)
{
    GPIO_InitTypeDef GPIO_InitDef;

    // Setup GPIO pin for button input
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
 
    GPIO_InitDef.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitDef.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
    GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitDef.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitDef);

}


int main(void) {
 
    volatile uint8_t button_pressed = 0;

    setup_led_gpios();
    setup_button_gpios();

    setup_usart1_gpios();
    setup_usart1();

    GPIO_ToggleBits(GPIOG, GPIO_Pin_n0);
    while (1) {
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {
            if (!button_pressed) {
                button_pressed = 1;
                GPIO_ToggleBits(GPIOG, GPIO_Pin_n0);
                GPIO_ToggleBits(GPIOG, GPIO_Pin_n1);
            }
        } else {
            button_pressed = 0;
        }
    }
}
