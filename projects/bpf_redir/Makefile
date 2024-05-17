
PUBLIC_KEY_PATH=~/ssd2/qemu_vms/debian12_debootstrap/bookworm.id_rsa

CC=clang

ifeq ($(V),1)
Q =
msg =
else
Q = @
msg = @printf '  %-8s%s %s%s\n' "$(1)" "$(if $(2), [$(2)])" "$(notdir $(3))" "$(if $(4), $(4))";
MAKEFLAGS += --no-print-directory
endif


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
	$(call msg,MKDUMP,,$(VMLINUX_DEFS))
	$(Q) sudo bpftool btf dump file /sys/kernel/btf/vmlinux format c > $(VMLINUX_DEFS)


$(BPF_SOCKOPS_V4): $(VMLINUX_DEFS)
	$(call msg,CC,,$@)
	$(Q) $(CC) -O2 -g $(APP_FLAGS) -target bpf -c bpf_sockops_v4.c -o $(BPF_SOCKOPS_V4)

$(BPF_TCPIP_BYPASS): $(VMLINUX_DEFS)
	$(call msg,CC,,$@)
	$(Q) $(CC) -O2 -g -Wall $(APP_FLAGS) -target bpf -c bpf_tcpip_bypass.c -o $(BPF_TCPIP_BYPASS)

install:
	$(call msg,INSTALL,,${BPF_SOCKOPS_V4})
	$(Q)scp -q -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" ${BPF_SOCKOPS_V4} root@localhost://root
	$(call msg,INSTALL,,${BPF_TCPIP_BYPASS})
	$(Q)scp -q -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" ${BPF_TCPIP_BYPASS} root@localhost://root
	$(call msg,INSTALL,,load.sh)
	$(Q)scp -q -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" load.sh root@localhost://root
	$(call msg,INSTALL,,unload.sh)
	$(Q)scp -q -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" unload.sh root@localhost://root


clean:
	$(call msg,CLEAN)
	$(Q) rm -f *.o
	$(Q) rm -f $(VMLINUX_DEFS)

.PHONY: all build copy clean
