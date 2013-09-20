/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <stm32f10x.h>
#include "watchdog.h"
#include "sys/sys.h"

#define KR_KEY_ACCESS   0x5555
#define KR_KEY_RELOAD   0xAAAA
#define KR_KEY_ENABLE   0xCCCC

static const int MAX_NUM_WATCHDOGS = 31;

static uint32_t _mask __attribute__((section (".noinit")));
static int _num_watchdogs __attribute__((section (".noinit")));

void watchdogInit(int timeout_ms)
{
    DBGTRACE("watchdog", "mask: 0x%08x, num: %d", _mask, _num_watchdogs);
    _mask = 0;
    _num_watchdogs = 0;

    __disable_irq();
    DBGMCU->CR |= DBGMCU_CR_DBG_IWDG_STOP;
    __enable_irq();

    ASSERT(timeout_ms > 0);
    if (timeout_ms < 0)
        timeout_ms = 0;

    int reload_value = timeout_ms / 6;
    if (reload_value > 0xFFF)
        reload_value = 0xFFF;

    IWDG->KR = KR_KEY_ACCESS;
    IWDG->PR = 6;               // Div 256
    IWDG->RLR = reload_value;
    IWDG->KR = KR_KEY_RELOAD;
    IWDG->KR = KR_KEY_ENABLE;
}

int watchdogStart(void)
{
    __disable_irq();
    const int new_id = _num_watchdogs++;
    // Reset immediately:
    if (new_id < MAX_NUM_WATCHDOGS)
        _mask |= 1 << new_id;
    __enable_irq();

    if (new_id >= MAX_NUM_WATCHDOGS)
    {
        ASSERT(0);
        return -1;
    }
    return new_id;
}

void watchdogReset(const int id)
{
    ASSERT(id >= 0);
    ASSERT(id < _num_watchdogs);

    if (id < 0)
        return;

    __disable_irq();
    _mask |= 1 << id;
    const uint32_t valid_bits_mask = (1 << _num_watchdogs) - 1;
    if ((_mask & valid_bits_mask) == valid_bits_mask)
    {
        IWDG->KR = KR_KEY_RELOAD;
        _mask = 0;
    }
    __enable_irq();
}
