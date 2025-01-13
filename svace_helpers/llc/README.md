## LLC sockets
----------                           ----------
| host   | <-----------------------> | quest  |
|        | tap2                 eth1 | (qemu) |
----------                           ----------
llc_client                           llc_server


# Define guest interface MAC address
readonly GUEST_MAC=fc:ac:14:e8:8f:32

# Add host -> MAX mapping into /etc/ethers
The llc_client needs the quest interface MAC address
On host: 

$ echo "${GUEST_MAC} quest_host_name" > /etc/ethers

/etc/ethers
fc:ac:14:e8:8f:32 virthost

# Install VDE

To enable VDE, vde2 packet needs to be installed
$ apt install vde2

# Start VDE deamon

$ sudo vde_switch --mode 660 --group ilya --tap tap2 -d


# Start QEMU
Add these lines to qemu command line:

    qemu-system-x86_64 \
        ...
        -net vde \
        -net nic,macaddr=${GUEST_MAC}  \
        ...

# Enable interfaces
Host
$ ip link set dev tap2 u
Guest
$ ip link set dev eth1 u
