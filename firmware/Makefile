#
# Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
#

CSRC = src/main.c          \
       src/flash_storage.c \
       src/watchdog.c      \
       src/config.c        \
       src/console.c       \
       src/magnet.c        \
       src/canasctl.c

UINCDIR = src/sys

UDEFS =

# CANaerospace
include canaerospace.mk
include ../../canaerospace/drivers/stm32/rules.mk
CSRC += $(CANAS_SRC) $(CAN_SRC)
UINCDIR += $(CANAS_INC) $(CAN_INC)
UDEFS += $(CANAS_DEF) $(CAN_DEF) -DCAN_CHIBIOS=1

# System stuff
CSRC += src/sys/board.c src/sys/sys.c \
        spl/src/stm32f10x_can.c       \
        spl/src/stm32f10x_rcc.c       \
        spl/src/stm32f10x_tim.c       \
        spl/src/stm32f10x_flash.c

UINCDIR += spl/inc

UDEFS += -DCORTEX_ENABLE_WFI_IDLE=1 -DSTDOUT_SD=SD1 -DSTDIN_SD=STDOUT_SD

# ---------------------------

PROJECT = opengrab_can

USE_OPT = -falign-functions=16 --std=c99 -Wno-unused-parameter -Wno-strict-aliasing -U__STRICT_ANSI__
USE_COPT =
USE_CPPOPT = -fno-rtti

RELEASE ?= 0
ifneq ($(RELEASE),0)
    DDEFS += -DRELEASE
    USE_OPT += -O1 -fomit-frame-pointer
else
    DDEFS += -DDEBUG
    #DDEFS += -DSPL_ASSERTIONS=1     # SPL asserts use like 16k of flash
    USE_OPT += -O1 -g3
endif

USE_LINK_GC = yes
USE_THUMB = yes
USE_VERBOSE_COMPILE = no
USE_FWLIB = no

# asm statement fix
DDEFS += -Dasm=__asm

# ---------------------------

CHIBIOS = chibios
include $(CHIBIOS)/os/hal/platforms/STM32F1xx/platform.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/ports/GCC/ARMCMx/STM32F1xx/port.mk
include $(CHIBIOS)/os/kernel/kernel.mk

LDSCRIPT= $(PORTLD)/STM32F103xB.ld

VARIOUSSRC = $(CHIBIOS)/os/various/syscalls.c $(CHIBIOS)/os/various/chprintf.c $(CHIBIOS)/os/various/shell.c

CSRC += $(PORTSRC) $(KERNSRC) $(HALSRC) $(PLATFORMSRC) $(VARIOUSSRC)

ASMSRC = $(PORTASM)

INCDIR = $(PORTINC) $(KERNINC) $(HALINC) $(PLATFORMINC) $(CHIBIOS)/os/various

# ---------------------------

MCU  = cortex-m3

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

CWARN = -Wall -Wextra -Werror -Wstrict-prototypes
CPPWARN = -Wall -Wextra -Werror

include $(CHIBIOS)/os/ports/GCC/ARMCMx/rules.mk
