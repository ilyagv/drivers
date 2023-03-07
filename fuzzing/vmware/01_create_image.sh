#!/bin/bash

USERSPACE=./userspace

sudo debootstrap --include=openssh-server,curl,tar,gcc,libc6-dev,time,strace,sudo,less,psmisc,selinux-utils,policycoreutils,checkpolicy,selinux-policy-default,firmware-atheros,open-vm-tools --components=main,contrib,non-free stretch $USERSPACE

