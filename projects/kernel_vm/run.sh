#!/bin/bash

#KERNEL_IMG=/home/ilya/ssd/share/kernel_stable_5.10_build/linux/arch/x86_64/boot/bzImage
KERNEL_IMG=/home/ilya/ssd/share/kernel_stable_5.10_build/build_bpf/arch/x86_64/boot/bzImage
#KERNEL_IMG=/home/ilya/ssd/share/kernel_stable/linux/arch/x86_64/boot/bzImage

#KERNEL_IMG=/home/ilya/ssd/share/kernel_upstream_build/linux/arch/x86_64/boot/bzImage

#6.1
#KERNEL_IMG=/home/ilya/ssd/share/kernel_stable_6.1_build/build_mld_6.1.87/arch/x86_64/boot/bzImage
#KERNEL_IMG=/home/ilya/ssd/share/kernel_stable_6.1_build/build_mld_6.1.1/arch/x86_64/boot/bzImage
# 6.1.87
#KERNEL_IMG=/home/ilya/ssd/share/kernel_stable_6.1_build/build_lvc_config/arch/x86_64/boot/bzImage
#KERNEL_IMG=/home/ilya/ssd/share/kernel_stable_6.1_build/build_mld_kasan/arch/x86_64/boot/bzImage

#upstream
#KERNEL_IMG=/home/ilya/ssd/share/kernel_upstream_build/build_no_kasan/arch/x86_64/boot/bzImage
#KERNEL_IMG=/home/ilya/ssd/share/kernel_upstream_build/build/arch/x86_64/boot/bzImage
#KERNEL_IMG=/home/ilya/ssd/share/kernel_upstream_build/build_mld/arch/x86_64/boot/bzImage

rm qemu.log

screen -L -Logfile qemu.log \
    qemu-system-x86_64 \
	-m 4G \
	-smp 4,sockets=4,cores=1 \
	-kernel ${KERNEL_IMG} \
        -append "root=/dev/sda debug console=ttyS0 earlyprintk=serial nokaslr printk_ratelimit=0 log_buf_len=16Mib" \
    	-drive file=./bookworm.img,format=raw \
	-net nic,model=e1000 \
	-net user,hostfwd=tcp::10021-:22 -net nic,model=vmxnet3 \
	-enable-kvm \
	-nographic \
	-pidfile vm.pid

#        -net user,host=10.0.2.10,hostfwd=tcp::10022-:22 \
#qemu-system-x86_64 \
#	-kernel ${KERNEL_IMG} \
#	-drive file=bullseye.img,format=raw \
#	-append "root=/dev/sda nokaslr debug console=ttyS0 earlyprintk=serial slub_debug=OUZ" \
#	-net user,hostfwd=tcp::10021-:22 -net nic,model=vmxnet3 \
#	-m 4G \
#	-smp 4 \
#	-enable-kvm \
#	-nographic \
#	-pidfile vm.pid \
#	2>&1 | tee vm.log

