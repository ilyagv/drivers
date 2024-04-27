
PUBLIC_KEY_PATH=~/ssd2/test/test_debootstrap/bookworm.id_rsa

CC=clang

Q = @

APP_FLAGS=-DTEST_BPF_PUSH_DATA

BPF_SOCKOPS_V4=bpf_sockops_v4.o
BPF_TCPIP_BYPASS=bpf_tcpip_bypass.o
VMLINUX_DEFS=vmlinux.h

all: clean build

build: $(BPF_SOCKOPS_V4) $(BPF_TCPIP_BYPASS)

# To build Linux kernel definitions header it needs to make sure that file
# /sys/kernel/btf/vmlinux exists in the kernel. Check if CONFIG_DEBUG_INFO_BTF
# option is enabled.
$(VMLINUX_DEFS):
	$(Q) sudo bpftool btf dump file /sys/kernel/btf/vmlinux format c > $(VMLINUX_DEFS)


$(BPF_SOCKOPS_V4): $(VMLINUX_DEFS)
	$(Q) $(CC) -O2 -g $(APP_FLAGS) -target bpf -c bpf_sockops_v4.c -o $(BPF_SOCKOPS_V4)

$(BPF_TCPIP_BYPASS): $(VMLINUX_DEFS)
	$(Q) $(CC) -O2 -g -Wall $(APP_FLAGS) -target bpf -c bpf_tcpip_bypass.c -o $(BPF_TCPIP_BYPASS)

install:
	scp -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" ${BPF_SOCKOPS_V4} root@localhost://root
	scp -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" ${BPF_TCPIP_BYPASS} root@localhost://root
	scp -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" load.sh root@localhost://root
	scp -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" unload.sh root@localhost://root


clean:
	$(Q) rm -f *.o
	$(Q) rm -f $(VMLINUX_DEFS)

.PHONY: all build copy clean
