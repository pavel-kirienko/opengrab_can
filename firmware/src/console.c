/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ch.h>
#include <hal.h>
#include <shell.h>
#include "sys/sys.h"
#include "config.h"
#include "magnet.h"
#include "flash_storage.h"

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
        PRINTF(prompt);
    if (!shellGetLine((BaseSequentialStream*)&STDIN_SD, _linebuf, sizeof(_linebuf)))
        return _linebuf;
    return NULL;
}

static void _printErrnoStatus(int err)
{
    if (err == 0)
        _puts("OK");
    else
        PRINTF("Failed: %d %s\n", err, strerror(err));
}

// ========= Commands =========

static void cmd_param(BaseSequentialStream *chp, int argc, char *argv[])
{
    const char* const command = (argc < 1) ? "" : argv[0];

    void* pimg = NULL;
    int imglen = 0;
    cfgGetImagePtrAndSize(&pimg, &imglen);

    if (!strcmp(command, "list"))
    {
        for (int i = 0;; i++)
        {
            const char* name = cfgGetNameByIndex(i);
            if (!name)
                break;
            PRINTF("%s \t= %d\n", name, cfgGet(name));
        }
    }
    else if (!strcmp(command, "save"))
    {
        _printErrnoStatus(flashStorageWrite(pimg, imglen));
    }
    else if (!strcmp(command, "delete"))
    {
        _printErrnoStatus(flashStorageErase());
    }
    else if (!strcmp(command, "get"))
    {
        if (argc < 2)
        {
            _puts("Error: Parameter name expected");
            return;
        }
        const char* const name = argv[1];
        int value = 0;
        const int res = cfgTryGet(name, &value);
        if (res == 0)
            PRINTF("%d\n", value);
        else
            _printErrnoStatus(res);
    }
    else if (!strcmp(command, "set"))
    {
        if (argc < 3)
        {
            _puts("Error: Expected parameter name and value");
            return;
        }
        const char* const name = argv[1];
        const int value = atoi(argv[2]);
        const int res = cfgTrySet(name, value);
        PRINTF("%s <-- %d\n", name, value);
        _printErrnoStatus(res);
    }
    else
    {
        _puts("Usage:\n"
              "  param list\n"
              "  param save\n"
              "  param delete\n"
              "  param get <name>\n"
              "  param set <name> <value>");
    }
}

static void cmd_magnet(BaseSequentialStream *chp, int argc, char *argv[])
{
    if (argc < 1)
    {
        PRINTF("Magnet state: %d\n", (int)magnetReadFeedback());
    }
    else if (argv[0][0] == '0')
    {
        _puts("Magnet set OFF");
        magnetSetState(false);
    }
    else
    {
        _puts("Magnet set ON");
        magnetSetState(true);
    }
}

static void cmd_reset(BaseSequentialStream *chp, int argc, char *argv[])
{
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
    if (palReadPad(GPIOA, GPIOA_SERIAL_RX) == 0)
    {
        TRACE("console", "serial RX is not attached, console will not be initialized");
        return;
    }

    shellInit();

    ASSERT_ALWAYS(shellCreateStatic(&_config, waShell, sizeof(waShell), LOWPRIO));
}
