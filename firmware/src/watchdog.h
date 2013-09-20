/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

void watchdogInit(int timeout_ms);
int watchdogStart(void);
void watchdogReset(const int id);

#endif
