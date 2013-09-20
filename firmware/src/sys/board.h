/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "assert.h"
/// needed for SPL
#define assert_param(x) ASSERT(x)

#define STM32_HSECLK            8000000

#define STM32F10X_MD

#define GPIOA_SERIAL_RX     10
#define GPIOB_LED           13
#define GPIOB_DATA          8

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 *
 * The digits have the following meaning:
 *   0 - Analog input.
 *   1 - Push Pull output 10MHz.
 *   2 - Push Pull output 2MHz.
 *   3 - Push Pull output 50MHz.
 *   4 - Digital input.
 *   5 - Open Drain output 10MHz.
 *   6 - Open Drain output 2MHz.
 *   7 - Open Drain output 50MHz.
 *   8 - Digital input with PullUp or PullDown resistor depending on ODR.
 *   9 - Alternate Push Pull output 10MHz.
 *   A - Alternate Push Pull output 2MHz.
 *   B - Alternate Push Pull output 50MHz.
 *   C - Reserved.
 *   D - Alternate Open Drain output 10MHz.
 *   E - Alternate Open Drain output 2MHz.
 *   F - Alternate Open Drain output 50MHz.
 * Please refer to the STM32 Reference Manual for details.
 */

#define VAL_GPIOACRL            0x88888888      // 7..0
#define VAL_GPIOACRH            0x888b88b9      // 15..8
#define VAL_GPIOAODR            0x00000000

#define VAL_GPIOBCRL            0x88888888
#define VAL_GPIOBCRH            0x88288888
#define VAL_GPIOBODR            0x00000100      // Pin DATA is pulled up, thus ON is default state for EPM

// These are not present:
#define VAL_GPIOCCRL            0x88888888
#define VAL_GPIOCCRH            0x88888888
#define VAL_GPIOCODR            0x00000000

#define VAL_GPIODCRL            0x88888888
#define VAL_GPIODCRH            0x88888888
#define VAL_GPIODODR            0x00000000

#define VAL_GPIOECRL            0x88888888
#define VAL_GPIOECRH            0x88888888
#define VAL_GPIOEODR            0x00000000

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
    void boardInit(void);
    void debugPortDisable(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif
