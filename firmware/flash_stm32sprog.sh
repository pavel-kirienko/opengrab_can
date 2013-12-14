#!/bin/bash

PORT=${1:-'/dev/ttyUSB0'}

# Find the firmware image
bin=$(ls -1 build/*.bin)
if [ -z "$bin" ]; then
    echo "No firmware found"
    exit 1
fi

echo "Flashing $bin - $(du -b $bin | cut -f 1) bytes"

# https://code.google.com/p/stm32sprog/
stm32sprog -d $PORT -ervw $bin
