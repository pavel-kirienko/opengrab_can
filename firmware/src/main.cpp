/*
 * Copyright (c) 2013-2014 Pavel Kirienko <pavel.kirienko@gmail.com>
 */

#include <ch.h>
#include <hal.h>
#include <string.h>
#include <unistd.h>
#include <zubax_chibios/sys/sys.h>
#include <zubax_chibios/watchdog/watchdog.hpp>
#include <zubax_chibios/config/config.hpp>
#include <uavcan_stm32/uavcan_stm32.hpp>
#include <uavcan/protocol/param_server.hpp>
#include <uavcan/equipment/hardpoint/Command.hpp>
#include <uavcan/equipment/hardpoint/Status.hpp>
#include "console.h"
#include "magnet.h"

namespace
{

zubax_chibios::config::Param<unsigned> param_can_bitrate("can_bitrate", 1000000, 20000, 1000000);
zubax_chibios::config::Param<unsigned> param_node_id("uavcan_node_id", 125, 1, 125);

uavcan_stm32::CanInitHelper<> can;

typedef uavcan::Node<3584> Node;


class ParamManager : public uavcan::IParamManager
{
    void convert(float native_value, ConfigDataType native_type, uavcan::protocol::param::Value& out_value) const
    {
        if (native_type == CONFIG_TYPE_BOOL)
        {
            out_value.value_bool.push_back(native_value != 0);
        }
        else if (native_type == CONFIG_TYPE_INT)
        {
            out_value.value_int.push_back(native_value);
        }
        else if (native_type == CONFIG_TYPE_FLOAT)
        {
            out_value.value_float.push_back(native_value);
        }
        else { }
    }

    void getParamNameByIndex(ParamIndex index, ParamName& out_name) const override
    {
        const char* name = configNameByIndex(index);
        if (name != nullptr)
        {
            out_name = name;
        }
    }

    void assignParamValue(const ParamName& name, const ParamValue& value) override
    {
        const float native_value = (!value.value_bool.empty()) ? (value.value_bool[0] ? 1 : 0) :
                                   (!value.value_int.empty()) ? value.value_int[0] : value.value_float[0];
        (void)configSet(name.c_str(), native_value);
    }

    void readParamValue(const ParamName& name, ParamValue& out_value) const override
    {
        ConfigParam descr;
        const int res = configGetDescr(name.c_str(), &descr);
        if (res >= 0)
        {
            convert(configGet(name.c_str()), descr.type, out_value);
        }
    }

    void readParamDefaultMaxMin(const ParamName& name, ParamValue& out_default,
                                        ParamValue& out_max, ParamValue& out_min) const override
    {
        ConfigParam descr;
        const int res = configGetDescr(name.c_str(), &descr);
        if (res >= 0)
        {
            convert(descr.default_, descr.type, out_default);
            convert(descr.max, descr.type, out_max);
            convert(descr.min, descr.type, out_min);
        }
    }

    int saveAllParams()  override { return configSave(); }
    int eraseAllParams() override { return configErase(); }
} param_manager;


Node& getNode()
{
    static Node node(can.driver, uavcan_stm32::SystemClock::instance());
    return node;
}

void setLed(bool status)
{
    palWritePad(GPIOB, GPIOB_LED, status);
}

void handleHardpointCommand(const uavcan::ReceivedDataStructure<uavcan::equipment::hardpoint::Command>& msg)
{
    lowsyslog("Cmd %u NID %u\n", unsigned(msg.command), unsigned(msg.getSrcNodeID().get()));
    magnetSetState(msg.command > 0);
}

void publishHardpointStatus(const uavcan::TimerEvent&)
{
    uavcan::equipment::hardpoint::Status msg;
    msg.cargo_weight_variance = -1;
    msg.status = magnetReadFeedback() ? 1 : 0;
    static uavcan::Publisher<uavcan::equipment::hardpoint::Status> pub(getNode());
    (void)pub.broadcast(msg);
}

void init()
{
    halInit();
    chSysInit();
    sdStart(&STDOUT_SD, nullptr);

    watchdogInit();
    while (configInit() < 0) { }
    magnetInit();
    consoleInit();
    ::usleep(50000);

    /*
     * CAN bus
     */
    while (true)
    {
        int can_res = can.init(param_can_bitrate.get());
        if (can_res >= 0)
        {
            lowsyslog("CAN %u bps\n", unsigned(param_can_bitrate.get()));
            break;
        }
        lowsyslog("CAN init [%i], trying defaults...\n", can_res);
        can_res = can.init(param_can_bitrate.default_);
        if (can_res >= 0)
        {
            lowsyslog("CAN %u bps\n", unsigned(param_can_bitrate.default_));
            break;
        }
    }

    /*
     * UAVCAN node
     */
    Node& node = getNode();

    node.setNodeID(param_node_id.get());
    node.setName("com.zubax.opengrab_can");

    uavcan::protocol::SoftwareVersion swver;
    swver.major = FW_VERSION_MAJOR;
    swver.minor = FW_VERSION_MINOR;
    node.setSoftwareVersion(swver);

    uavcan::protocol::HardwareVersion hwver;
    hwver.major = HW_VERSION;
    node.setHardwareVersion(hwver);

    while (true)
    {
        const int res = node.start();
        if (res >= 0)
        {
            break;
        }
        lowsyslog("UAVCAN start: %i\n", res);
        ::sleep(1);
    }

    /*
     * Param server
     */
    static uavcan::ParamServer param_server(node);
    while (param_server.start(&param_manager) < 0) { }

    /*
     * Magnet control
     */
    static uavcan::Subscriber<uavcan::equipment::hardpoint::Command> sub_cmd(node);
    while (sub_cmd.start(handleHardpointCommand) < 0) { }

    static uavcan::Timer tmr_status(node);
    tmr_status.setCallback(publishHardpointStatus);
    tmr_status.startPeriodic(uavcan::MonotonicDuration::fromMSec(1000));
}

}

int main(void)
{
    init();

    zubax_chibios::watchdog::Timer wdt;
    wdt.startMSec(100);

    getNode().setStatusOk();

    while (1)
    {
        wdt.reset();
        const int spin_res = getNode().spin(uavcan::MonotonicDuration::fromMSec(25));
        if (spin_res < 0)
        {
            lowsyslog("Spin %i\n", spin_res);
        }
        setLed(can.driver.hadActivity());
    }
    return 0;
}
