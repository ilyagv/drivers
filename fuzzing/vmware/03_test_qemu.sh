#!/usr/bin/env bash

qemu-system-x86_64 \
	-hda disk.raw \
	-net user,hostfwd=tcp::10022-:22 -net nic \
	-enable-kvm \
	-m 2G \
	-display none \
	-serial stdio


