/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#ifndef SYS_SYS_H_
#define SYS_SYS_H_

#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <stdint.h>
#include "assert.h"

#ifndef STDOUT_SD
#   error "STDOUT_SD must be defined"
#endif

#define PRINTF(...)     chprintf((BaseSequentialStream*)&(STDOUT_SD), __VA_ARGS__)

// http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
#define TRACE(src, fmt, ...)   PRINTF(src ": " fmt "\n", ##__VA_ARGS__)

#ifndef USE_DBGTRACE
#   define USE_DBGTRACE  DEBUG
#endif
#if USE_DBGTRACE
#   define DBGTRACE(src, fmt, ...)   TRACE(src, fmt, ##__VA_ARGS__)
#else
#   define DBGTRACE(src, fmt, ...)   ((void)0)
#endif

uint64_t sysTimestampMicros(void);

#endif
