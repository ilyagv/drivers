#!/bin/bash

readonly NS_NAME=ns1
readonly HOST=veth0_host

# Delete namespace
ip netns exec ${NS_NAME} ip link set veth1 netns 1
#ip netns pids ${NS_NAME} | xargs kill
ip netns delete ${NS_NAME}
ip link delete veth0


ip netns add ${NS_NAME}

# Create a veth pair (veth0 <=> veth1)
ip link add veth0 type veth peer name veth1

# Move veth1 into new namespace
ip link set veth1 netns ${NS_NAME}

MAC=$(ip add show dev veth0 | grep "link/ether" | awk '{print $2}')

# Override /eth/ethers
echo "${MAC} ${HOST}" > /etc/ethers

ip netns exec ${NS_NAME} ./llc_client

