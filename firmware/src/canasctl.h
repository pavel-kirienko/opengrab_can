/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#ifndef CANASCTL_H_
#define CANASCTL_H_

#include <stdbool.h>

int canasctlInit(void);

extern bool canasctlReadMagnetFeedback(void);
extern void canasctlSetMagnetState(bool active);

#endif
