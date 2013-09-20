/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <ch.h>
#include <hal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "sys/sys.h"
#include "config.h"
#include "console.h"
#include "magnet.h"
#include "canasctl.h"
#include "flash_storage.h"
#include <can_driver.h>
#include <canaerospace/canaerospace.h>
#include <canaerospace/services/std_identification.h>

static void restoreConfig(void)
{
    cfgInitDefaults();

    void* pdata = NULL;
    int len = 0;
    cfgGetImagePtrAndSize(&pdata, &len);

    int res = flashStorageRead(pdata, len);
    if (res != 0)
    {
        TRACE("init", "no config found (%d) %s", res, strerror(res));
        // flashStorageRead() alters the buffer content even on failure, so we need to repair it:
        cfgInitDefaults();
    }

    if (!cfgIsValid())
    {
        TRACE("init", "invalid config, using defaults");
        cfgInitDefaults();
    }
}

bool canasctlReadMagnetFeedback(void)
{
    return magnetReadFeedback();
}

void canasctlSetMagnetState(bool active)
{
    magnetSetState(active);
}

static void ledOn(void)  { palSetPad(GPIOB, GPIOB_LED); }
static void ledOff(void) { palClearPad(GPIOB, GPIOB_LED); }

int main(void)
{
    halInit();
    chSysInit();
    sdStart(&STDOUT_SD, NULL);

    restoreConfig();
    magnetInit();
    consoleInit();

    int res = canasctlInit();
    if (res)
    {
        TRACE("init", "CANAS NOT INITED (%d), FIX CONFIG AND RESTART", res);
        ledOn();
        while (1)
            chThdSleepSeconds(999);
    }

    while (1)
    {
        const bool
            feedback = magnetReadFeedback(),
            requested = magnetGetRequestedState();

        ledOn();

        if (feedback != requested)
        {
            chThdSleepMilliseconds(70);
            ledOff();
            chThdSleepMilliseconds(70);
        }
        else if (feedback)
        {
            chThdSleepMilliseconds(500);
            ledOff();
            chThdSleepMilliseconds(500);
        }
        else
        {
            chThdSleepMilliseconds(70);
            ledOff();
            chThdSleepMilliseconds(930);
        }
    }
    return 0;
}
