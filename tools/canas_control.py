#!/usr/bin/env python

import sys, pycanbus, struct, random

SELF_NODE_ID             = 255
TX_DATA_TYPE             = 9
MSGID_GRIPPER_GRIP_CMD   = 450

if len(sys.argv) < 2:
    print 'Usage:   %s <iface> [0|1]' % sys.argv[0]
    print 'Example: %s slcan0 1' % sys.argv[0]
    exit(1)

sock = pycanbus.Socket(sys.argv[1])

if len(sys.argv) > 2:
    on = int(sys.argv[2])
    msgcode = int(random.random() * 255)
    data = struct.pack('>BBBBB', SELF_NODE_ID, TX_DATA_TYPE, 0, msgcode, on)
    frame = pycanbus.Frame(data, MSGID_GRIPPER_GRIP_CMD, False)
    sock.send(frame)
else:
    while 1:
        frame = sock.recv()
        print frame.pretty()
