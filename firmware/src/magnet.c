/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <ch.h>
#include <hal.h>
#include "sys/sys.h"
#include "magnet.h"

#define PWMD    PWMD1

#define PWM_FREQUENCY           50
#define PWM_CLOCK_FREQUENCY     (100000)
#define PWM_PERIOD_USEC         (1000000 / PWM_FREQUENCY)
#define PWM_PERIOD_TICKS        (PWM_CLOCK_FREQUENCY / PWM_FREQUENCY)

static const int PWM_PULSE_LEN_USEC_ON  = 2000;
static const int PWM_PULSE_LEN_USEC_OFF = 1000;

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

static bool _requested_state_active = true;

void magnetInit(void)
{
    pwmObjectInit(&PWMD);
    pwmStart(&PWMD, &_pwm_cfg);

    /*
     * Initial magnet state is defined by the feedback input
     */
    const bool initial_state = magnetReadFeedback();
    TRACE("magnet", "initial state: %i", (int)initial_state);
    magnetSetState(initial_state);
}

void magnetSetState(bool active)
{
    const int plen = active ? PWM_PULSE_LEN_USEC_ON : PWM_PULSE_LEN_USEC_OFF;

    const pwmcnt_t width = PWM_FRACTION_TO_WIDTH(&PWMD, PWM_PERIOD_USEC, plen);
    //DBGTRACE("magnet", "PWM period %u, width %u", (unsigned int)PWMD.period, (unsigned int)width);

    pwmEnableChannel(&PWMD, 0, width);
    _requested_state_active = active;
}

bool magnetGetRequestedState(void)
{
    return _requested_state_active;
}

bool magnetReadFeedback(void)
{
    return palReadPad(GPIOB, GPIOB_DATA);
}
