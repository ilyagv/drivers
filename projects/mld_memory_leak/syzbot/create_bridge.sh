#!/usr/bin/env bash

echo scan > /sys/kernel/debug/kmemleak
sleep 5
echo scan > /sys/kernel/debug/kmemleak

ip link add br0 type bridge
ip link set dev br0 address aa:aa:aa:aa:aa:0a
ip -6 addr add fe80::0a/120 dev br0
ip link set br0 up

ip link add veth0 type veth peer name br_slave_0
ip link set dev veth0 address aa:aa:aa:aa:aa:0b
ip -6 addr add fe80::0b/120 dev veth0
ip link set veth0 up

ip link add veth1 type veth peer name br_slave_1
ip link set dev veth1 address aa:aa:aa:aa:aa:0c
ip -6 addr add fe80::0c/120 dev veth1
ip link set veth1 up

ip link set br_slave_0 up
ip link set br_slave_1 up

ip link set br_slave_0 master br0
ip link set br_slave_1 master br0
ip link set veth1 master br0


sleep 10 

ip link delete veth0
#ip link delete br_slave_0

ip link delete veth1
#ip link delete br_slave_1

ip link delete br0
ip link set br_slave_0 up
