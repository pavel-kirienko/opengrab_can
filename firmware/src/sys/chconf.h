/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#pragma once

#define CH_FREQUENCY                    1000

#define CH_USE_HEAP                     TRUE
#define CH_USE_DYNAMIC                  FALSE
#define CH_USE_MAILBOXES                FALSE
#define CH_USE_CONDVARS                 FALSE

#if DEBUG_BUILD
#   define CH_DBG_SYSTEM_STATE_CHECK       TRUE
#   define CH_DBG_ENABLE_CHECKS            TRUE
#   define CH_DBG_ENABLE_ASSERTS           TRUE
#   define CH_DBG_ENABLE_STACK_CHECK       TRUE
#   define CH_DBG_FILL_THREADS             TRUE
#   define CH_DBG_THREADS_PROFILING        TRUE
#elif RELEASE_BUILD
#   define CH_DBG_THREADS_PROFILING        FALSE
#else
#   error "Invalid configuration: Either DEBUG_BUILD or RELEASE_BUILD must be true"
#endif

#include <zubax_chibios/sys/chconf_tail.h>
