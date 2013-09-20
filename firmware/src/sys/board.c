/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <ch.h>
#include <hal.h>

const PALConfig pal_default_config =
{
    {VAL_GPIOAODR, VAL_GPIOACRL, VAL_GPIOACRH},
    {VAL_GPIOBODR, VAL_GPIOBCRL, VAL_GPIOBCRH},
    {VAL_GPIOCODR, VAL_GPIOCCRL, VAL_GPIOCCRH},
    {VAL_GPIODODR, VAL_GPIODCRL, VAL_GPIODCRH},
    {VAL_GPIOEODR, VAL_GPIOECRL, VAL_GPIOECRH},
};

void __early_init(void)
{
    stm32_clock_init();
}

void debugPortDisable(void)
{
    __disable_irq();
    uint32_t mapr = AFIO->MAPR;
    mapr &= ~AFIO_MAPR_SWJ_CFG;  // these bits are write-only

    // Disable both SWD and JTAG:
    mapr |= AFIO_MAPR_SWJ_CFG_DISABLE;

    AFIO->MAPR = mapr;
    __enable_irq();
}

void boardInit(void)
{
    uint32_t mapr = AFIO->MAPR;
    mapr &= ~AFIO_MAPR_SWJ_CFG;  // these bits are write-only

    // Enable SWJ only, JTAG is not needed at all:
    mapr |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

    AFIO->MAPR = mapr;
}
