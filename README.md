OGCAN - CAN bus interface for OpenGrab EPM
============

CAN bus interface for [OpenGrab - electro-permanent cargo holding magnet][1].

Based on [libcanaerospace][2] - open-source CANaerospace protocol implementation.

[UAVCAN protocol][4] support is scheduled for April 2014.

## CANaerospace Interface
The set of CANaerospace messages used by this node is documented below. Note that the message IDs can be reconfigured via CLI.

#### Input parameters
###### CANAS_UAV_GRIPPER_GRIP_COMMAND
- ID: 1820
- Type: CHAR
- Values:
    - 0 - turn the magnet off
    - 1+ - activate the magnet

#### Output parameters
###### CANAS_UAV_GRIPPER_GRIP_STATE
- ID: 1821
- Type: CHAR
- Values:
    - 0 - magnet OFF
    - 1+ - magnet ON
- Transmission rate: 1 Hz

## Getting Hardware
Grab your OGCAN hardware here: http://nicadrone.com/index.php?id_product=18&controller=product

Schematics and Gerber files are available in this repository; hardware design files are available upon request.

## Building Firmware
The firmware can be built with ARM GCC or any other GCC-based ARM toolchain.

- Place the [libcanaerospace][2] sources into the same directory with this repository.
- Build:

```shell
cd opengrab_can/firmware
make RELEASE=1
```
- Flash.

## Setup
Use CLI via the TTL UART connector at 115200-8-N-1. Note that CLI will not be available unless the RX pin was pulled up at startup; that means that the UART cable must be connected before power-up.

Execute `help` to get a short command reference.

## Test
Linux only! Clone and install the [pycanbus][3] library. This is the Python module that enables access to the CAN network under Linux.

Use script from the `tools/` directory to switch the magnet or read it's state:
```shell
cd opengrab_can/tools
./canas_control.py <can-iface>       # Simply prints all incoming CAN frames
./canas_control.py <can-iface> <0/1> # Sends the CANaerospace message to turn the magnet OFF/ON
```
[1]: https://code.google.com/p/opengrab/
[2]: https://bitbucket.org/pavel_kirienko/canaerospace
[3]: https://bitbucket.org/pavel_kirienko/pycanbus
[4]: http://www.diydrones.com/profiles/blogs/uavcan-can-bus-for-uav
