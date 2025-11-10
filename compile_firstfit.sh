#!/bin/bash

set -e

# Usage:
#   ./compile_firstfit.sh         # build inside Docker (FIRSTFIT)
#   ./compile_firstfit.sh PVS     # build and run PVS (requires so-pvs image)

RUN_PVS=0
if [ "$1" == "PVS" ]; then
	RUN_PVS=1
fi

IMAGE=${IMAGE:-so-pvs}
CONTAINER=${CONTAINER:-arqui-tpe-pvs}
HOST_ROOT="${PWD}"
SRC_IN_CONTAINER=/src

echo "Building with memory manager: FIRSTFIT (Docker image: ${IMAGE})"

# Ensure no stale container
docker rm -f "${CONTAINER}" >/dev/null 2>&1 || true

# Start container (detached) with source and license mounted
docker run -d --platform=linux/amd64 \
	--name "${CONTAINER}" \
	-v "${HOST_ROOT}:${SRC_IN_CONTAINER}" \
	-v "${HOST_ROOT}/.config/PVS-Studio:/root/.config/PVS-Studio" \
	--privileged \
	--security-opt seccomp:unconfined \
	-it "${IMAGE}" tail -f /dev/null >/dev/null

set +e
# Clean and build inside the container
docker exec -it "${CONTAINER}" bash -lc "
	set -e
	cd ${SRC_IN_CONTAINER}
	make clean -C ./Toolchain
	make clean -C ./
	make -C ./Toolchain
	make -C ./ MM=FIRSTFIT
"
BUILD_STATUS=$?
set -e

if [ ${BUILD_STATUS} -ne 0 ]; then
	echo "Build failed."
	docker rm -f "${CONTAINER}" >/dev/null 2>&1 || true
	exit ${BUILD_STATUS}
fi

if [ ${RUN_PVS} -eq 1 ]; then
	echo "Running PVS-Studio analysis inside Docker..."
	set +e
	docker exec -it "${CONTAINER}" bash -lc "
		set -e
		cd ${SRC_IN_CONTAINER}
		# Clean and capture build with trace
		make clean -C ./Toolchain
		make clean -C ./
		export MAKEFLAGS=
		pvs-studio-analyzer trace -- sh -c 'make -C ./Toolchain && make -C ./ MM=FIRSTFIT all'
		pvs-studio-analyzer analyze
		plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
	"
	PVS_STATUS=$?
	set -e
	if [ ${PVS_STATUS} -ne 0 ]; then
		echo "PVS-Studio analysis encountered issues (see PVS-Studio.log)."
	fi
	echo "PVS tasklist: ${HOST_ROOT}/report.tasks"
fi

# Stop and remove container
docker rm -f "${CONTAINER}" >/dev/null 2>&1 || true
echo "Done."


