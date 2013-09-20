/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include <can_driver.h>
#include <canaerospace/canaerospace.h>
#include <canaerospace/services/std_identification.h>
#include "canasctl.h"
#include "sys/sys.h"
#include "config.h"

static const int MY_HARDWARE_REVISION = 1;
static const int MY_SOFTWARE_REVISION = 1;

static CanasInstance _canas_instance;

static WORKING_AREA(waCanas, 2048);

static char _strbuf[CANAS_DUMP_BUF_LEN];

static int _msgid_magnet_state   = -1;
static int _msgid_magnet_command = -1;

/*
 * ---
 */

static void _blockingPollReadUpdate(int timeout_usec)
{
    CanasCanFrame frm;
    int iface = -1;
    int ret = canReceive(&iface, &frm, timeout_usec);
    if (ret < 0)
    {
        TRACE("canasctl", "CAN RX failed: %i\n", ret);
        return;
    }

    if (ret == 0)
        ret = canasUpdate(&_canas_instance, -1, NULL);     // No data received
    else
    {
        ASSERT(iface >= 0 && iface < CAN_IFACE_COUNT);
        ret = canasUpdate(&_canas_instance, iface, &frm);
    }

    if (ret != 0)
    {
        TRACE("canasctl", "canas update failed: %i", ret);
        TRACE("canasctl", "offending frame: %s", canasDumpCanFrame(&frm, _strbuf));
    }
}

static void _checkErrors(void)
{
    const unsigned int errmask = canYieldErrors(0);
    if (errmask)
        TRACE("canasctl", "CAN errmask %04x", errmask);
}

static void _1hz(void);

static msg_t _thread(void* arg)
{
    static const int UPDATE_INTERVAL_USEC = 10 * 1000;

    uint64_t prevCb1hz = sysTimestampMicros();

    chRegSetThreadName("canasctl");

    while (1)
    {
        _blockingPollReadUpdate(UPDATE_INTERVAL_USEC);

        const uint64_t current_timestamp = sysTimestampMicros();
        if (current_timestamp - prevCb1hz > 1000000)
        {
            prevCb1hz = current_timestamp;
            _1hz();
            _checkErrors();
        }
    }
    return 0;
}

/*
 * libcanaerospace support
 */

static int _drvSend(CanasInstance* pi, int iface, const CanasCanFrame* pframe)
{
    ASSERT(pi);
    ASSERT(iface >= 0 && iface < CAN_IFACE_COUNT);
    ASSERT(pframe);
    return canSend(iface, pframe);
}

static int _drvFilter(CanasInstance* pi, int iface, const CanasCanFilterConfig* pfilters, int nfilters)
{
    ASSERT(pi);
    ASSERT(iface >= 0 && iface < CAN_IFACE_COUNT);
    ASSERT(pfilters);
    ASSERT(nfilters > 0);
    return canFilterSetup(iface, pfilters, nfilters);
}

static void* _implMalloc(CanasInstance* pi, int size)
{
    ASSERT(pi);
    return chCoreAlloc(size);
}

static uint64_t _implTimestamp(CanasInstance* pi)
{
    ASSERT(pi);
    return sysTimestampMicros();
}

/*
 * ---
 */

static void _1hz(void)
{
    bool state = canasctlReadMagnetFeedback();

    CanasMessageData msgd;
    msgd.type = CANAS_DATATYPE_CHAR;
    msgd.container.CHAR = state;
    int res = canasParamPublish(&_canas_instance, _msgid_magnet_state, &msgd, 0);
    if (res)
        TRACE("canasctl", "failed to publish magnet state: %d", res);
}

static void _cbMagnetCommand(CanasInstance* pi, CanasParamCallbackArgs* args)
{
    ASSERT(pi);
    ASSERT(args);

    // Accept any data type; magnet will be turned OFF if all payload bytes are zero, ON otherwise.
    bool all_zeros = true;
    ASSERT(args->message.data.length <= 4);
    for (int i = 0; i < args->message.data.length; i++)
    {
        if (args->message.data.container.CHAR4[i])
        {
            all_zeros = false;
            break;
        }
    }

    const bool new_state = !all_zeros;
    DBGTRACE("canasctl", "new magnet state %d, msg: %s", (int)new_state, canasDumpMessage(&args->message, _strbuf));
    canasctlSetMagnetState(new_state);
}

#define CHECKERR(x, msg) \
    do { \
        if ((x) != 0) { \
            TRACE("canasctl", "init failure (%d): " msg, (int)(x)); \
            return (x); \
        } \
    } while (0)

int canasctlInit(void)
{
    /*
     * CAN driver
     */
    int res = canInit(cfgGet("can_bitrate"), 4000);
    CHECKERR(res, "CAN driver");

    /*
     * libcanaerospace
     */
    CanasConfig cfg = canasMakeConfig();
    cfg.filters_per_iface = CAN_FILTERS_PER_IFACE;
    cfg.iface_count  = CAN_IFACE_COUNT;
    cfg.fn_send      = _drvSend;
    cfg.fn_filter    = _drvFilter;
    cfg.fn_malloc    = _implMalloc;
    cfg.fn_timestamp = _implTimestamp;

    cfg.node_id           = cfgGet("canas_node_id");
    cfg.redund_channel_id = cfgGet("canas_redund_chan");
    cfg.service_channel   = cfgGet("canas_service_chan");

    res = canasInit(&_canas_instance, &cfg, NULL);
    CHECKERR(res, "libcanaerospace");

    /*
     * CANaerospace logic
     */
    // IDS service (required):
    CanasSrvIdsPayload ids_selfdescr;
    ids_selfdescr.hardware_revision = MY_HARDWARE_REVISION;
    ids_selfdescr.software_revision = MY_SOFTWARE_REVISION;
    ids_selfdescr.id_distribution   = CANAS_SRV_IDS_ID_DISTRIBUTION_STD;  // These two are standard
    ids_selfdescr.header_type       = CANAS_SRV_IDS_HEADER_TYPE_STD;
    res = canasSrvIdsInit(&_canas_instance, &ids_selfdescr, 0);
    CHECKERR(res, "IDS service");

    // Magnet control
    const int num_redund_channels_to_subscribe = cfgGet("canas_num_redund_chans_to_listen");
    _msgid_magnet_state   = cfgGet("canas_msgid_magnet_state");
    _msgid_magnet_command = cfgGet("canas_msgid_magnet_command");

    res = canasParamAdvertise(&_canas_instance, _msgid_magnet_state);
    CHECKERR(res, "magnet state advertisement");

    res = canasParamSubscribe(&_canas_instance, _msgid_magnet_command, num_redund_channels_to_subscribe,
                              _cbMagnetCommand, NULL);
    CHECKERR(res, "magnet control subscription");

    /*
     * OS
     */
    ASSERT_ALWAYS(chThdCreateStatic(waCanas, sizeof(waCanas), HIGHPRIO, _thread, NULL));

    return 0;
}

