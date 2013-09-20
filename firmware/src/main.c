/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <ch.h>
#include <hal.h>
#include <string.h>
#include "sys/sys.h"
#include "watchdog.h"
#include "config.h"
#include "console.h"
#include "magnet.h"
#include "canasctl.h"
#include "flash_storage.h"

#define WATCHDOG_TIMEOUT_MS             2000
#define DEBUG_PORT_DISABLE_DEADLINE_SEC 20

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

    watchdogInit(WATCHDOG_TIMEOUT_MS);
    const int wdid = watchdogStart();

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

#if RELEASE
    TRACE("init", "debug port will be disabled %d sec later", DEBUG_PORT_DISABLE_DEADLINE_SEC);
#endif

    bool debug_port_disabled = false;
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

        if (!debug_port_disabled)
        {
            if (sysTimestampMicros() / 1000000 > DEBUG_PORT_DISABLE_DEADLINE_SEC)
            {
#if RELEASE
                debugPortDisable();
#endif
                debug_port_disabled = true;
            }
        }

        watchdogReset(wdid);
    }
    return 0;
}
