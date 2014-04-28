/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#pragma once

#if __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void magnetInit(void);

void magnetSetState(bool active);
bool magnetGetRequestedState(void);
bool magnetReadFeedback(void);

#if __cplusplus
}
#endif
