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

# Detectar el sistema operativo
OS=$(uname -s)
AUDIO_CONFIG=""

case $OS in
    "Darwin")
        # macOS - usar CoreAudio
        AUDIO_CONFIG="-audiodev coreaudio,id=speaker -machine pcspk-audiodev=speaker"
        echo "Detectado macOS - usando CoreAudio"
        ;;
    "Linux")
        # Linux - comprobar si PulseAudio está disponible, sino usar ALSA
        if command -v pulseaudio >/dev/null 2>&1; then
            AUDIO_CONFIG="-audiodev pa,id=speaker -machine pcspk-audiodev=speaker"
            echo "Detectado Linux con PulseAudio"
        else
            AUDIO_CONFIG="-audiodev alsa,id=speaker -machine pcspk-audiodev=speaker"
            echo "Detectado Linux - usando ALSA"
        fi
        ;;
    CYGWIN*|MINGW*|MSYS*)
        # Windows con Cygwin/MinGW/MSYS
        AUDIO_CONFIG="-audiodev dsound,id=speaker -machine pcspk-audiodev=speaker"
        echo "Detectado Windows - usando DirectSound"
        ;;
    *)
        # Sistema desconocido - usar configuración por defecto sin audio específico
        AUDIO_CONFIG=""
        echo "Sistema operativo desconocido ($OS) - ejecutando sin configuración de audio específica"
        ;;
esac

# Ejecutar QEMU con la configuración de audio apropiada
echo "Ejecutando: qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 $AUDIO_CONFIG"
qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 $AUDIO_CONFIG

