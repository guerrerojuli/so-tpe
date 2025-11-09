#!/bin/bash

echo "================================"
echo "Testing Memory Manager: BUDDY"
echo "================================"
echo ""

# Build with BUDDY memory manager
echo "Building kernel with BUDDY memory manager..."
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
echo "Starting QEMU with BUDDY Memory Manager"
echo "================================"
echo ""
echo "Once the OS boots, run the following command to test the memory manager:"
echo ""
echo "  test-mm 1048576     # Test with 1MB of memory"
echo ""
echo "You can also try different memory sizes:"
echo "  test-mm 10485760    # Test with 10MB"
echo "  test-mm 104857600   # Test with 100MB"
echo ""
echo "Press Ctrl+C to stop the test"
echo "================================"
echo ""

# Detect OS and set audio configuration
OS=$(uname -s)
AUDIO_CONFIG=""

case $OS in
    "Darwin")
        AUDIO_CONFIG="-audiodev coreaudio,id=speaker -machine pcspk-audiodev=speaker"
        ;;
    "Linux")
        if command -v pulseaudio >/dev/null 2>&1; then
            AUDIO_CONFIG="-audiodev pa,id=speaker -machine pcspk-audiodev=speaker"
        else
            AUDIO_CONFIG="-audiodev alsa,id=speaker -machine pcspk-audiodev=speaker"
        fi
        ;;
    CYGWIN*|MINGW*|MSYS*)
        AUDIO_CONFIG="-audiodev dsound,id=speaker -machine pcspk-audiodev=speaker"
        ;;
    *)
        AUDIO_CONFIG=""
        ;;
esac

# Run QEMU
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 $AUDIO_CONFIG