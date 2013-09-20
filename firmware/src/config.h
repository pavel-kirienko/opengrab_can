/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>

int cfgGet(const char* name);

int cfgTryGet(const char* name, int* out_value);
int cfgTrySet(const char* name, int value);

const char* cfgGetNameByIndex(int index);

void cfgInitDefaults(void);
bool cfgIsValid(void);
void cfgGetImagePtrAndSize(void** ppimage, int* psize);

#endif
