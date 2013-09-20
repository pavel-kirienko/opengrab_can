/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#ifndef CANASCTL_H_
#define CANASCTL_H_

#include <stdbool.h>

enum
{
    CANASCTL_MSGID_GRIPPER_GRIP_CMD     = 450,
    CANASCTL_MSGID_GRIPPER_GRIP_STATE   = 451
};

int canasctlInit(void);

extern bool canasctlReadMagnetFeedback(void);
extern void canasctlSetMagnetState(bool active);

#endif
