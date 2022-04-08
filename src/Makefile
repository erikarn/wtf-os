BSP_SUBDIR=bsp
KERN_SUBDIR=kern
BOARD_SUBDIR=boards

COMPILER_PATH=/usr/local/gcc-arm-embedded/bin
# COMPILER_PATH=/usr/local/bin

CC=$(COMPILER_PATH)/arm-none-eabi-gcc
AS=$(COMPILER_PATH)/arm-none-eabi-as
OBJCOPY=$(COMPILER_PATH)/arm-none-eabi-objcopy

# Generic flags, for both the assembler and compiler(s).

GEN_FLAGS = -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
GEN_FLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

AS_FLAGS = -g

# C compiler flags, again some of them are specific to the board
# being used right now.  They're here because they're not supported
# if passed into the assembler.

C_FLAGS = -g -O -Wall
C_FLAGS += -T$(BSP_SUBDIR)/local/stm32f4/STM32F439NI_FLASH.ld
# The stm32f429-disco board has an 8MHz crystal, but the BSP code
# defaults this CPU type to 25MHz.  The PLL_M divisor also needs
# updating; it defaults to 25 but that's too high for an 8MHz clock.
C_FLAGS += -I.
C_FLAGS += -I$(BSP_SUBDIR)/stm32f4
C_FLAGS += --specs=nosys.specs
# C_FLAGS += -DUSE_STDPERIPH_DRIVER
# C_FLAGS += -DSTM32F429_439xx #Update this to match your board.

# XXX TODO: startup and rcc code needs these fixed and unified!
C_FLAGS += -DHSE_VALUE=8000000 -DPLL_M=8

# My little hardware / CPU library

C_FLAGS += -I$(BSP_SUBDIR)/local
C_FLAGS += -I$(BSP_SUBDIR)/local/arm-m4-32

SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_flash.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_usart.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_startup.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_rcc.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_rcc_table.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_gpio.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_syscfg.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_hw_exti.c
SRCS += $(BSP_SUBDIR)/local/stm32f4/stm32f429_startup.s

# Testing for now; I haven't implemented a RAM VTOR yet.
SRCS += $(BSP_SUBDIR)/local/arm-m4-32/core/arm_m4_vtor_ram.c
SRCS += $(BSP_SUBDIR)/local/arm-m4-32/core/arm_m4_exception.c
SRCS += $(BSP_SUBDIR)/local/arm-m4-32/core/arm_m4_platform.c
SRCS += $(BSP_SUBDIR)/local/arm-m4-32/core/arm_m4_nvic.c
SRCS += $(BSP_SUBDIR)/local/arm-m4-32/core/arm_m4_systick.c
SRCS += $(BSP_SUBDIR)/local/arm-m4-32/core/arm_m4_switch.S
SRCS += $(BSP_SUBDIR)/local/arm-m4-32/core/arm_m4_svc.S

# And the beginning of some kernel routines!
SRCS += $(KERN_SUBDIR)/console/console.c
SRCS += $(KERN_SUBDIR)/core/exception.c
SRCS += $(KERN_SUBDIR)/core/task.c
SRCS += $(KERN_SUBDIR)/core/timer.c
SRCS += $(KERN_SUBDIR)/core/physmem.c

SRCS += $(KERN_SUBDIR)/libraries/printf/mini_printf.c
SRCS += $(KERN_SUBDIR)/libraries/string/strlen.c
SRCS += $(KERN_SUBDIR)/libraries/string/strnlen.c
SRCS += $(KERN_SUBDIR)/libraries/string/strlcpy.c
SRCS += $(KERN_SUBDIR)/libraries/list/list.c

# The board initialisation routine
SRCS += $(BOARD_SUBDIR)/stm32f429-discovery/main.c
SRCS += $(BOARD_SUBDIR)/stm32f429-discovery/test_userland.c

# Don't modify below here

CFLAGS := $(GEN_FLAGS) $(C_FLAGS)

ASFLAGS = $(GEN_FLAGS) $(AS_FLAGS)

C_SRCS = $(filter %.c, $(SRCS))
S_SRCS = $(filter %.s, $(SRCS))
SS_SRCS = $(filter %.S, $(SRCS))

C_OBJS = $(C_SRCS:%.c=%.o)
S_OBJS = $(S_SRCS:%.s=%.o)
SS_OBJS = $(SS_SRCS:%.S=%.o)

.PHONY: wtfos

all: wtfos

wtfos: wtfos.elf

wtfos.elf: $(C_OBJS) $(S_OBJS) $(SS_OBJS)
	$(CC) $(CFLAGS) $(C_OBJS) $(S_OBJS) $(SS_OBJS) -o $@
	$(OBJCOPY) -O binary wtfos.elf wtfos.bin

flash: wtfos
	st-flash --connect-under-reset write wtfos.bin 0x8000000

clean:
	rm -f $(C_OBJS) $(S_OBJS)
	rm -f wtfos.elf wtfos.bin
