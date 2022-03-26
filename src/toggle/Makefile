SRCS=*.c

# STM_PERIPH=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0

BSP_SUBDIR=bsp

COMPILER_PATH=/usr/local/gcc-arm-embedded/bin
# COMPILER_PATH=/usr/local/bin

CC=$(COMPILER_PATH)/arm-none-eabi-gcc
OBJCOPY=$(COMPILER_PATH)/arm-none-eabi-objcopy

# Stuff from the STM32F4 example build, using the HAL.
# Some are generic config options for this platform, some are
# specific for the STM32F4 HAL.

CFLAGS = -g -O -Wall
CFLAGS += -T$(BSP_SUBDIR)/stm32f4/STM32F439NI_FLASH.ld
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += --specs=nosys.specs
CFLAGS += -DUSE_STDPERIPH_DRIVER
# The stm32f429-disco board has an 8MHz crystal, but the BSP code
# defaults this CPU type to 25MHz.  The PLL_M divisor also needs
# updating; it defaults to 25 but that's too high for an 8MHz clock.
CFLAGS += -DSTM32F429_439xx #Update this to match your board.
CFLAGS += -DHSE_VALUE=8000000 -DPLL_M=8
CFLAGS += -I.
CFLAGS += -I$(BSP_SUBDIR)/stm32f4

SRCS += $(BSP_SUBDIR)/stm32f4/stm32f4xx_gpio.c
SRCS += $(BSP_SUBDIR)/stm32f4/stm32f4xx_rcc.c
SRCS += $(BSP_SUBDIR)/stm32f4/system_stm32f4xx.c
# Note: this isn't the SDRAM or SRAM initialisation startup
SRCS += $(BSP_SUBDIR)/stm32f4/startup_stm32f429_439xx.s

# And my budding random stuff.
CFLAGS += -I$(BSP_SUBDIR)/local
CFLAGS += -I$(BSP_SUBDIR)/local/arm-m4-32

SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_flash.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_usart.c

.PHONY: toggle

all: toggle

toggle: toggle.elf

toggle.elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ 
	$(OBJCOPY) -O binary toggle.elf toggle.bin

flash: toggle
	st-flash write toggle.bin 0x8000000

clean:
	rm -f toggle.elf toggle.bin
