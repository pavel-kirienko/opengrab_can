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

static void _puts(const char* str)
{
    sdWrite(&STDOUT_SD, (uint8_t*)str, strlen(str));
    sdWrite(&STDOUT_SD, (uint8_t*)"\n", 1);
}

static char* _getline(const char* prompt)
{
    static char _linebuf[32];
    memset(_linebuf, 0, sizeof(_linebuf));
    if (prompt)
        lowsyslog(prompt);
    if (!shellGetLine((BaseSequentialStream*)&STDIN_SD, _linebuf, sizeof(_linebuf)))
        return _linebuf;
    return NULL;
}

// ========= Commands =========

static void cmd_param(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)chp;
    (void)argc;
    (void)argv;
    // TODO
}

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

static void cmd_reset(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)chp;
    (void)argc;
    (void)argv;
    const char* line = _getline("Really reset? y/(n) ");
    if (line && (line[0] == 'Y' || line[0] == 'y'))
    {
        _puts("RESTART\n\n");
        chThdSleep(MS2ST(100));  // Flush the serial buffers
        NVIC_SystemReset();
    }
    else
        _puts("Command aborted");
}

#define COMMAND(cmd)    {#cmd, cmd_##cmd},
static const ShellCommand _commands[] =
{
    COMMAND(param)
    COMMAND(magnet)
    COMMAND(reset)
    {NULL, NULL}
};

// ============================

static const ShellConfig _config = {(BaseSequentialStream*)&STDOUT_SD, _commands};

static WORKING_AREA(waShell, 2048);

void consoleInit(void)
{
    shellInit();
    ASSERT_ALWAYS(shellCreateStatic(&_config, waShell, sizeof(waShell), LOWPRIO));
}
