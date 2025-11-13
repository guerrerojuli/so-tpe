#!/bin/bash

echo "================================"
echo "Compiling with BUDDY Memory Manager"
echo "================================"

docker pull agodio/itba-so:2.0
docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -it --name arqui-tpe agodio/itba-so:2.0
docker start arqui-tpe
docker exec -it arqui-tpe make clean -C /root/Toolchain
docker exec -it arqui-tpe make clean -C /root/
docker exec -it arqui-tpe make -C /root/Toolchain
docker exec -it arqui-tpe make -C /root/ MM=BUDDY
docker stop arqui-tpe

echo ""
echo "================================"
echo "Running with BUDDY Memory Manager"
echo "================================"

QEMU_DBG_FLAGS=""

if [ "$1" = "gdb" ]; then
    QEMU_DBG_FLAGS="-s -S -d int"
    echo "GDB mode enabled: launching QEMU with $QEMU_DBG_FLAGS"
fi

echo "Ejecutando: qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 $QEMU_DBG_FLAGS"
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 $QEMU_DBG_FLAGS
