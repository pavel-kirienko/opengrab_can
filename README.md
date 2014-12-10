OGCAN - CAN bus interface for OpenGrab EPM
============

CAN bus interface for [OpenGrab - electro-permanent cargo holding magnet][1].

Based on [UAVCAN - CAN bus for UAV][2].

## UAVCAN interface
Supported standard data structures (data structs that are mandatory for any UAVCAN node are not listed):

- Configuration: `uavcan.protocol.param.*`
- Magnet control: `uavcan.equipment.hardpoint.*`

OGCAN publishes state of the EPM once a second using `uavcan.equipment.hardpoint.Status`.

EPM can be controlled using `uavcan.equipment.hardpoint.Command`.

Default config:

- CAN bitrate: 1 Mbps.
- UAVCAN Node ID: 125.

## Getting Hardware
Grab your OGCAN hardware here: http://nicadrone.com/index.php?id_product=18&controller=product

Schematics and Gerber files are available in this repository; hardware design files are available upon request.

## Building Firmware
**Prebuilt binary for the latest stable version is available in `firmware/prebuilt/*.bin`.**

The firmware can be built with Embedded ARM GCC or any other GCC-based ARM toolchain.

```shell
git submodule update --init --recursive
cd firmware
make

# Flash. The following helper scripts are available:
# ./zubax_chibios/tools/stlink_flash.sh       # ST-Link v2
# ./zubax_chibios/tools/blackmagic_flash.sh   # Black Magic Debug Probe
```

[1]: https://code.google.com/p/opengrab/
[2]: http://uavcan.org/
[3]: https://github.com/pavel-kirienko/uavcan/
