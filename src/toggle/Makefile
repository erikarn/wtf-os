BSP_SUBDIR=bsp
KERN_SUBDIR=kern

COMPILER_PATH=/usr/local/gcc-arm-embedded/bin
# COMPILER_PATH=/usr/local/bin

CC=$(COMPILER_PATH)/arm-none-eabi-gcc
AS=$(COMPILER_PATH)/arm-none-eabi-as
OBJCOPY=$(COMPILER_PATH)/arm-none-eabi-objcopy

# Generic flags, for both the assembler and compiler(s).

GEN_FLAGS = -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
GEN_FLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

# C compiler flags, again some of them are specific to the board
# being used right now.  They're here because they're not supported
# if passed into the assembler.

C_FLAGS = -g -O -Wall
C_FLAGS += -T$(BSP_SUBDIR)/stm32f4/STM32F439NI_FLASH.ld
# The stm32f429-disco board has an 8MHz crystal, but the BSP code
# defaults this CPU type to 25MHz.  The PLL_M divisor also needs
# updating; it defaults to 25 but that's too high for an 8MHz clock.
C_FLAGS += -I.
C_FLAGS += -I$(BSP_SUBDIR)/stm32f4
C_FLAGS += --specs=nosys.specs
C_FLAGS += -DUSE_STDPERIPH_DRIVER
C_FLAGS += -DSTM32F429_439xx #Update this to match your board.
C_FLAGS += -DHSE_VALUE=8000000 -DPLL_M=8

# stm32f4 HAL Sources being built.

SRCS += $(BSP_SUBDIR)/stm32f4/stm32f4xx_gpio.c
SRCS += $(BSP_SUBDIR)/stm32f4/stm32f4xx_rcc.c
SRCS += $(BSP_SUBDIR)/stm32f4/system_stm32f4xx.c
# Note: this isn't the SDRAM or SRAM initialisation startup
SRCS += $(BSP_SUBDIR)/stm32f4/startup_stm32f429_439xx.s

# And my budding random stuff.
C_FLAGS += -I$(BSP_SUBDIR)/local
C_FLAGS += -I$(BSP_SUBDIR)/local/arm-m4-32

SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_flash.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_usart.c

SRCS += $(KERN_SUBDIR)/console/console.c

# The top-level program!
SRCS += toggle.c

# Don't modify below here

CFLAGS := $(GEN_FLAGS) $(C_FLAGS)

ASFLAGS = $(GEN_FLAGS)

C_SRCS = $(filter %.c, $(SRCS))
S_SRCS = $(filter %.s, $(SRCS))

C_OBJS = $(C_SRCS:%.c=%.o)
S_OBJS = $(S_SRCS:%.s=%.o)

.PHONY: toggle

all: toggle

toggle: toggle.elf

toggle.elf: $(C_OBJS) $(S_OBJS)
	$(CC) $(CFLAGS) $(C_OBJS) $(S_OBJS) -o $@
	$(OBJCOPY) -O binary toggle.elf toggle.bin

flash: toggle
	st-flash write toggle.bin 0x8000000

clean:
	rm -f $(C_OBJS) $(S_OBJS)
	rm -f toggle.elf toggle.bin
