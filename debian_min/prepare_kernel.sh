#!/usr/bin/env bash

KERNEL_URL=https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
KERNEL_DIR=linux
ARCH=$(uname -m)
BUILD_DIR=../build_${ARCH}

if [ ! -d "${KERNEL_DIR}" ]; then
	git clone ${KERNEL_URL}
fi

cd ${KERNEL_DIR}

make O=${BUILD_DIR} defconfig
make O=${BUILD_DIR} kvm_guest.config

scripts/config --file "${BUILD_DIR}/.config" -e CONFIGFS_FS

make O=${BUILD_DIR} -j $(nproc)
