#!/bin/bash

USERSPACE=./userspace
KERNEL=/home/ilya/ssd2/fuzzing/build_x86_64

wget https://raw.githubusercontent.com/google/syzkaller/master/tools/create-gce-image.sh -O create-gce-image.sh
chmod +x create-gce-image.sh
./create-gce-image.sh $USERSPACE $KERNEL/arch/x86_64/boot/bzImage
qemu-img convert disk.raw -O vmdk disk.vmdk
