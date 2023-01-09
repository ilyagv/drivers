#!/bin/bash

KERNEL_IMG=./build_x86_64/arch/x86_64/boot/bzImage

qemu-system-x86_64 \
	-kernel ${KERNEL_IMG} \
	-drive file=bullseye.img,format=raw \
	-append "root=/dev/sda nokaslr debug console=ttyS0 earlyprintk=serial slub_debug=QUZ" \
	-net user,hostfwd=tcp::10021-:22 -net nic \
	-m 4G \
	-smp 4 \
	-enable-kvm \
	-nographic \
	-pidfile vm.pid \
	2>&1 | tee vm.log

