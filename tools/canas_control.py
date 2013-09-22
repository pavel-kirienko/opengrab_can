#!/usr/bin/env python

import sys, pycanbus, struct, random

SELF_NODE_ID             = 255
TX_DATA_TYPE             = 9
CANAS_UAV_GRIPPER_GRIP_COMMAND = 1820

if len(sys.argv) < 2:
    print 'Usage:   %s <iface> [0|1]' % sys.argv[0]
    print 'Example: %s slcan0 1' % sys.argv[0]
    exit(1)

sock = pycanbus.Socket(sys.argv[1])

if len(sys.argv) > 2:
    on = int(sys.argv[2])
    try:
        msgcode = int(open('/tmp/opengrab_canas_control_msgcode').read())
    except IOError, ValueError:
        msgcode = 0
    msgcode += 1
    with open('/tmp/opengrab_canas_control_msgcode', 'w') as f:
        f.write(str(msgcode))
    data = struct.pack('>BBBBB', SELF_NODE_ID, TX_DATA_TYPE, 0, msgcode, on)
    frame = pycanbus.Frame(data, CANAS_UAV_GRIPPER_GRIP_COMMAND, False)
    sock.send(frame)
else:
    while 1:
        frame = sock.recv()
        print frame.pretty()
