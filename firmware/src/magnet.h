/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#ifndef MAGNET_H_
#define MAGNET_H_

#include <stdbool.h>

void magnetInit(void);

void magnetSetState(bool active);
bool magnetGetRequestedState(void);
bool magnetReadFeedback(void);

#endif
