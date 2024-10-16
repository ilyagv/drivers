#!/usr/bin/env bash

#qemu-system-i386 boot_sect_raw.bin
#qemu-system-i386 -drive file=boot_sect_raw.bin,format=raw
qemu-system-i386 -drive file=boot_sect.bin,format=raw
