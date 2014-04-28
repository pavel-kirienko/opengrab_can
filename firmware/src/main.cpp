/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <ch.h>
#include <hal.h>
#include <string.h>
#include <unistd.h>
#include <crdr_chibios/watchdog/watchdog.h>
#include <crdr_chibios/config/config.h>
#include "console.h"
#include "magnet.h"

namespace
{

void ledOn(void)
{
    palSetPad(GPIOB, GPIOB_LED);
}

void ledOff(void)
{
    palClearPad(GPIOB, GPIOB_LED);
}

}

int main(void)
{
    halInit();
    chSysInit();
    sdStart(&STDOUT_SD, nullptr);

    watchdogInit();
    while (configInit() < 0) { }
    magnetInit();
    consoleInit();

    while (1)
    {
        const bool
            feedback = magnetReadFeedback(),
            requested = magnetGetRequestedState();

        ledOn();

        if (feedback != requested)
        {
            ::usleep(70000);
            ledOff();
            ::usleep(70000);
        }
        else if (feedback)
        {
            ::usleep(500000);
            ledOff();
            ::usleep(500000);
        }
        else
        {
            ::usleep(70000);
            ledOff();
            ::usleep(930000);
        }
    }
    return 0;
}
