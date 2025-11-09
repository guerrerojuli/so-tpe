#!/bin/bash

if [ "$1" = "gdb" ]; then
    echo "GDB mode enabled: launching QEMU with -s -S -d int"
    qemu-system-x86_64 -s -S -hda Image/x64BareBonesImage.qcow2 -m 512 -d int
else
    qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512
fi

