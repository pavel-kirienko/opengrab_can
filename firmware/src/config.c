/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <canaerospace/message.h>
#include "sys/sys.h"
#include "config.h"
#include "canasctl.h"

typedef struct
{
    const char* name;
    int default_;
    int min;
    int max;
    bool (*validate)(int);
} ParamDescriptor;

#define PARAM_RNG(name, default_, min, max)     {name, default_, min, max, NULL},
#define PARAM_VLD(name, default_, validator)    {name, default_, 0, 0, validator},

// ======= Application configs =======

bool validateCanasServiceChan(int val)
{
    return (val >= CANAS_SERVICE_CHANNEL_HIGH_MIN && val <= CANAS_SERVICE_CHANNEL_HIGH_MAX)
        || (val >= CANAS_SERVICE_CHANNEL_LOW_MIN  && val <= CANAS_SERVICE_CHANNEL_LOW_MAX);
}

#define MSGID_MIN   CANAS_MSGTYPE_USER_DEFINED_HIGH_MIN
#define MSGID_MAX   CANAS_MSGTYPE_USER_DEFINED_LOW_MAX

static const ParamDescriptor _descriptors[] =
{
    PARAM_RNG("can_bitrate",                1000000,    20000,  1000000)
    PARAM_RNG("canas_node_id",              1,          1,      255)
    PARAM_RNG("canas_redund_chan",          0,          0,      3)
    PARAM_VLD("canas_service_chan",         0,          validateCanasServiceChan)
    PARAM_RNG("canas_num_redund_chans_to_listen",3,     1,      4)
    // messages
    PARAM_RNG("canas_msgid_magnet_state",   CANASCTL_MSGID_GRIPPER_GRIP_STATE,  MSGID_MIN,  MSGID_MAX)
    PARAM_RNG("canas_msgid_magnet_command", CANASCTL_MSGID_GRIPPER_GRIP_CMD,    MSGID_MIN,  MSGID_MAX)
};

// ===================================

#define NUM_PARAMS  ((int)(sizeof(_descriptors) / sizeof(_descriptors[0])))

static int _params[NUM_PARAMS];

static int _indexByName(const char* name)
{
    for (int i = 0; i < NUM_PARAMS; i++)
    {
        if (!strcmp(name, _descriptors[i].name))
            return i;
    }
    return -1;
}

static bool _validate(const ParamDescriptor* pdesc, int value)
{
    if (pdesc->validate)
    {
        if (!pdesc->validate(value))
            return false;
    }
    else
    {
        if (value < pdesc->min || value > pdesc->max)
            return false;
    }
    return true;
}

int cfgGet(const char* name)
{
    ASSERT_ALWAYS(name);

    const int index = _indexByName(name);
    ASSERT(index >= 0);             // Make sure the application doesn't request non-existent parameters
    ASSERT(index < NUM_PARAMS);     // Paranoid check

    if (index < 0)                  // For release builds, return 0 as default
    {
        TRACE("cfg", "non-existent config '%s'", name);
        return 0;
    }

    return _params[index];
}

int cfgTryGet(const char* name, int* out_value)
{
    ASSERT_ALWAYS(name);
    ASSERT_ALWAYS(out_value);

    const int index = _indexByName(name);
    if (index < 0)
        return -ENOENT;

    *out_value = _params[index];
    return 0;
}

int cfgTrySet(const char* name, int value)
{
    ASSERT_ALWAYS(name);

    const int index = _indexByName(name);
    if (index < 0)
        return -ENOENT;

    if (!_validate(_descriptors + index, value))
        return -EINVAL;

    _params[index] = value;
    return 0;
}

const char* cfgGetNameByIndex(int index)
{
    if (index < 0 || index >= NUM_PARAMS)
        return NULL;
    return _descriptors[index].name;
}

void cfgInitDefaults(void)
{
    for (int i = 0; i < NUM_PARAMS; i++)
    {
        // Make sure that the default value is valid:
        ASSERT(_validate(_descriptors + i, _descriptors[i].default_));
        _params[i] = _descriptors[i].default_;
    }
}

bool cfgIsValid(void)
{
    for (int i = 0; i < NUM_PARAMS; i++)
    {
        if (!_validate(_descriptors + i, _descriptors[i].default_))
            return false;
    }
    return true;
}

void cfgGetImagePtrAndSize(void** ppimage, int* psize)
{
    ASSERT_ALWAYS(ppimage);
    ASSERT_ALWAYS(psize);
    *ppimage = _params;
    *psize = sizeof(_params);
}
