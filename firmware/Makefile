#
# Copyright (c) 2014 Pavel Kirienko <pavel.kirienko@gmail.com>
#

PROJECT = opengrab_can

#
# Application
#

CSRC = $(shell find src -name '*.c')

CPPSRC = $(shell find src -name '*.cpp')

UINCDIR = src/sys

UDEFS = -DFW_VERSION_MAJOR=1 -DFW_VERSION_MINOR=0 -DHW_VERSION=1 -DNDEBUG

#
# UAVCAN library
#

UDEFS += -DUAVCAN_STM32_TIMER_NUMBER=2    \
         -DUAVCAN_STM32_NUM_IFACES=1      \
         -DUAVCAN_STM32_CHIBIOS=1         \
         -DUAVCAN_TINY=1                  \
         -DUAVCAN_MEM_POOL_BLOCK_SIZE=56  \
         -DUAVCAN_TOSTRING=0

include uavcan/libuavcan/include.mk
CPPSRC += $(LIBUAVCAN_SRC)
UINCDIR += $(LIBUAVCAN_INC)

include uavcan/libuavcan_drivers/stm32/driver/include.mk
CPPSRC += $(LIBUAVCAN_STM32_SRC)
UINCDIR += $(LIBUAVCAN_STM32_INC)

$(info $(shell $(LIBUAVCAN_DSDLC) $(UAVCAN_DSDL_DIR)))
UINCDIR += dsdlc_generated

#
# ChibiOS
#

SERIAL_CLI_PORT_NUMBER = 1

RELEASE=1
RELEASE_OPT = -Os -fomit-frame-pointer
DEBUG_OPT = $(RELEASE_OPT)

LDSCRIPT = ld.ld

include zubax_chibios/rules_stm32f103.mk
