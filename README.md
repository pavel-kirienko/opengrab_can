CANaerospace Controller for OpenGrab
============

CANaerospace controller for [OpenGrab - electro-permanent cargo holding magnet][1].

Based on [libcanaerospace][2] - open-source CANaerospace protocol implementation.

## CANaerospace Interface
The set of CANaerospace messages used by this node is documented below. Note that the message IDs can be reconfigured.

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
Coming late 2013.

## Building Firmware
The firmware can be built with ARM GCC or any other GCC-based ARM toolchain.

1. Place the [libcanaerospace][2] sources into the same directory with this repository.
2. Build:
```shell
cd opengrab_can/firmware
make RELEASE=1
```
3. Flash.

## Setup
Use CLI via the TTL UART connector at 115200-8-N-1. Note that CLI will not be available unless the RX pin was pulled up at startup; that means that the UART cable must be connected before power-up.

Execute `help` to get a short command reference.

## Test
Clone and install the [pycanbus][3] library. This is the Python module that enables access to the CAN network.

Use script from the `tools/` directory to switch the magnet or read it's state:
```shell
cd opengrab_can/tools
./canas_control.py <can-iface>       # Simply prints all incoming CAN frames
./canas_control.py <can-iface> <0/1> # Sends the CANaerospace message to turn the magnet OFF/ON
```
[1]: https://code.google.com/p/opengrab/
[2]: https://bitbucket.org/pavel_kirienko/canaerospace
[3]: https://bitbucket.org/pavel_kirienko/pycanbus
