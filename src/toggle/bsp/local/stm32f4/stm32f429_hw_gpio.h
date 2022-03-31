#ifndef	__STM32F429_HW_GPIO_H__
#define	__STM32F429_HW_GPIO_H__

typedef enum {
	STM32F429_HW_GPIO_BLOCK_GPIOA = 0,
	STM32F429_HW_GPIO_BLOCK_GPIOB,
	STM32F429_HW_GPIO_BLOCK_GPIOC,
	STM32F429_HW_GPIO_BLOCK_GPIOD,
	STM32F429_HW_GPIO_BLOCK_GPIOE,
	STM32F429_HW_GPIO_BLOCK_GPIOF,
	STM32F429_HW_GPIO_BLOCK_GPIOG,
	STM32F429_HW_GPIO_BLOCK_GPIOH,
	STM32F429_HW_GPIO_BLOCK_GPIOI,
	STM32F429_HW_GPIO_BLOCK_GPIOJ,
	STM32F429_HW_GPIO_BLOCK_GPIOK,

	STM32F429_HW_GPIO_BLOCK_NUM
} stm32f429_hw_gpio_block_t;

/**
 * The GPIO operating speed for output/alternate function.
 *
 * There are four speeds.  For the two highest ones it requires
 * a compensation cell in the syscfg block to be enabled for
 * the signal edges to be nice and sharp.
 */
typedef enum {
	STM32F429_HW_GPIO_PIN_SPEED_LOW = 0,
	STM32F429_HW_GPIO_PIN_SPEED_MED = 1,
	STM32F429_HW_GPIO_PIN_SPEED_FAST = 2,
	STM32F429_HW_GPIO_PIN_SPEED_HIGH = 3,
} stm32f429_hw_gpio_pin_speed_t;

/**
 * The GPIO pin operating mode.
 */
typedef enum {
	STM32F429_HW_GPIO_PIN_MODE_INPUT = 0,
	STM32F429_HW_GPIO_PIN_MODE_OUTPUT = 1,
	STM32F429_HW_GPIO_PIN_MODE_ALTERNATE = 2,
	STM32F429_HW_GPIO_PIN_MODE_ANALOG = 3,
} stm32f429_hw_gpio_pin_mode_t;

typedef enum {
	STM32F429_HW_GPIO_PIN_OUTPUT_TYPE_PUPD = 0,
	STM32F429_HW_GPIO_PIN_OUTPUT_TYPE_OPEN_DRAIN = 1,
} stm32f429_hw_gpio_pin_output_type_t;

typedef enum {
	STM32F429_HW_GPIO_PIN_PUPD_NONE = 0,
	STM32F429_HW_GPIO_PIN_PUPD_PULL_UP = 1,
	STM32F429_HW_GPIO_PIN_PUPD_PULL_DOWN = 2,
} stm32f429_hw_gpio_pin_pupd_t;

struct stm32f429_hw_gpio_pin_config {
	stm32f429_hw_gpio_pin_mode_t mode;
	stm32f429_hw_gpio_pin_speed_t speed;
	stm32f429_hw_gpio_pin_output_type_t out_type;
	stm32f429_hw_gpio_pin_pupd_t out_pupd;
	bool out_value;
};

extern	void stm32f429_hw_gpio_config_init(
	    struct stm32f429_hw_gpio_pin_config *config);
extern	void stm32f429_hw_gpio_config_get(stm32f429_hw_gpio_block_t, int pin,
	    struct stm32f429_hw_gpio_pin_config *config);
extern	void stm32f429_hw_gpio_config_set(stm32f429_hw_gpio_block_t, int pin,
	    const struct stm32f429_hw_gpio_pin_config *config);
extern	void stm32f429_hw_gpio_alternate_set(stm32f429_hw_gpio_block_t, int pin,
	    uint32_t alternate);
extern	uint32_t stm32f429_hw_gpio_get_block(stm32f429_hw_gpio_block_t block);
extern	void stm32f429_hw_gpio_set_block(stm32f429_hw_gpio_block_t block,
	    uint32_t val);
extern	void stm32f429_hw_gpio_reset_block(stm32f429_hw_gpio_block_t block);

extern	bool stm32f429_hw_gpio_get_pin(stm32f429_hw_gpio_block_t, int pin);
extern	void stm32f429_hw_gpio_set_pin(stm32f429_hw_gpio_block_t, int pin,
	    bool val);
extern	int stm32f429_hw_gpio_toggle_pin(stm32f429_hw_gpio_block_t, int pin);

#endif	/* __STM32F429_HW_GPIO_H__ */
