

COMPILER_PATH=/usr/local/gcc-arm-embedded/bin
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
C_FLAGS += -TUSER_TASK.ld
C_FLAGS += -nostartfiles
C_FLAGS += -msingle-pic-base -mpic-register=r9 -mpic-data-is-text-relative -fPIC

C_FLAGS += -I.
C_FLAGS += --specs=nosys.specs
# The board initialisation routine

SRCS += test.c

# Don't modify below here

CFLAGS := $(GEN_FLAGS) $(C_FLAGS)

ASFLAGS = $(GEN_FLAGS) $(AS_FLAGS)

C_SRCS = $(filter %.c, $(SRCS))
S_SRCS = $(filter %.s, $(SRCS))
SS_SRCS = $(filter %.S, $(SRCS))

C_OBJS = $(C_SRCS:%.c=%.o)
S_OBJS = $(S_SRCS:%.s=%.o)
SS_OBJS = $(SS_SRCS:%.S=%.o)

.PHONY: test

all: test

test: test.elf

test.elf: $(C_OBJS) $(S_OBJS) $(SS_OBJS)
	$(CC) $(CFLAGS) $(C_OBJS) $(S_OBJS) $(SS_OBJS) -o $@
	$(OBJCOPY) -O binary test.elf test.bin

clean:
	rm -f $(C_OBJS) $(S_OBJS) $(SS_OBJS)
	rm -f test.elf test.bin
