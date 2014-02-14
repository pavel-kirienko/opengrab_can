/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <ch.h>
#include <hal.h>
#include "sys/sys.h"
#include "magnet.h"
#include "watchdog.h"

#define PWMD    PWMD1

#define PWM_FREQUENCY           50
#define PWM_CLOCK_FREQUENCY     (100000)
#define PWM_PERIOD_USEC         (1000000 / PWM_FREQUENCY)
#define PWM_PERIOD_TICKS        (PWM_CLOCK_FREQUENCY / PWM_FREQUENCY)

static const int TURN_ON_DURATION_SEC  = 9;
static const int TURN_OFF_DURATION_SEC = 3;
static const int INIT_DURATION_SEC     = 1;

static const int PWM_PULSE_LEN_USEC_ON      = 2000;
static const int PWM_PULSE_LEN_USEC_OFF     = 1000;
static const int PWM_PULSE_LEN_USEC_NEUTRAL = 1600;

static const PWMConfig _pwm_cfg =
{
    PWM_CLOCK_FREQUENCY,
    PWM_PERIOD_TICKS,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    },
    0,
    0
};

static int _watchdog_id = -1;
static bool _requested_state = false;

static WORKING_AREA(_wa_magnet_thread, 1024);


static msg_t _thread(void* arg)
{
    watchdogReset(_watchdog_id);
    chThdSleepMilliseconds(1000);     // Giving some time to magnet to initialize
    watchdogReset(_watchdog_id);

    const uint64_t init_deadline = sysTimestampMicros() + INIT_DURATION_SEC * 1000000ul;
    uint64_t switching_deadline = sysTimestampMicros();

    int pulse_len = PWM_PULSE_LEN_USEC_NEUTRAL;
    int prev_pulse_len = pulse_len;

    for (;;)
    {
        const uint64_t timestamp = sysTimestampMicros();

        if ((magnetReadFeedback() != _requested_state) || (timestamp < init_deadline))
        {
            const uint64_t duration = _requested_state ? TURN_ON_DURATION_SEC : TURN_OFF_DURATION_SEC;
            switching_deadline = sysTimestampMicros() + duration * 1000000ul;
            pulse_len = _requested_state ? PWM_PULSE_LEN_USEC_ON : PWM_PULSE_LEN_USEC_OFF;
        }
        else if (timestamp > switching_deadline)
        {
            pulse_len = PWM_PULSE_LEN_USEC_NEUTRAL;
        }

        const pwmcnt_t width = PWM_FRACTION_TO_WIDTH(&PWMD, PWM_PERIOD_USEC, pulse_len);
        pwmEnableChannel(&PWMD, 0, width);

        watchdogReset(_watchdog_id);

        if (prev_pulse_len != pulse_len)
        {
            TRACE("magnet", "PWM pulse len: %i usec", pulse_len);
            prev_pulse_len = pulse_len;
        }

        chThdSleepMilliseconds(50);
    }
    return 0;
}

void magnetInit(void)
{
    pwmObjectInit(&PWMD);
    pwmStart(&PWMD, &_pwm_cfg);

    /*
     * Initial magnet state is defined by the feedback input
     */
    const bool initial_state = magnetReadFeedback();
    magnetSetState(initial_state);

    /*
     * Thread
     */
    _watchdog_id = watchdogStart();
    ASSERT_ALWAYS(chThdCreateStatic(_wa_magnet_thread, sizeof(_wa_magnet_thread), LOWPRIO, _thread, NULL));

    TRACE("magnet", "initial state: %i, watchdog id: %i", (int)initial_state, (int)_watchdog_id);
}

void magnetSetState(bool active)
{
    _requested_state = active;
}

bool magnetGetRequestedState(void)
{
    return _requested_state;
}

bool magnetReadFeedback(void)
{
    return palReadPad(GPIOB, GPIOB_DATA);
}
