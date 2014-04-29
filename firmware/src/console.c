/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ch.h>
#include <hal.h>
#include <shell.h>
#include <crdr_chibios/sys/sys.h>
#include <crdr_chibios/config/config.h>
#include "magnet.h"

static void cmd_magnet(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)chp;
    if (argc < 1)
        lowsyslog("%d\n", (int)magnetReadFeedback());
    else if (argv[0][0] == '0')
        magnetSetState(false);
    else
        magnetSetState(true);
}

#define COMMAND(cmd)    {#cmd, cmd_##cmd},
static const ShellCommand _commands[] =
{
    COMMAND(magnet)
    {NULL, NULL}
};

static const ShellConfig _config = {(BaseSequentialStream*)&STDOUT_SD, _commands};

static WORKING_AREA(waShell, 512);

void consoleInit(void)
{
    shellInit();
    ASSERT_ALWAYS(shellCreateStatic(&_config, waShell, sizeof(waShell), LOWPRIO));
}
