TARGET=llc_client
PUBLIC_KEY_PATH=~/ssd2/qemu_vms/debian12_debootstrap/bookworm.id_rsa
RUN_SCRIPT=create_veth.sh

ifeq ($(V),1)
Q =
msg =
else
Q = @
msg = @printf '  %-8s%s %s%s\n' "$(1)" "$(if $(2), [$(2)])" "$(notdir $(3))" "$(if $(4), $(4))";
MAKEFLAGS += --no-print-directory
endif

CFLAGS=-Wall -static
LDFLAGS=-static-libgcc

OBJS:= llc_client.o
SOURCES:=$(subst .o,.c, $(OBJS))

all: clean $(TARGET)

%.o: %.c
	$(call msg,CC,,$@)
	$(Q)$(CC) $(CFLAGS) $(TARGET_ARCH) -c $<

$(TARGET): $(OBJS)
	$(call msg,LINK,,$@)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ -o $@

format: $(SOURCES)
	$(call msg,FORMAT,,$^)
	$(Q)clang-format -style=file -i $^

install: $(TARGET) $(RUN_SCRIPT)
	$(call msg,INSTALL,,$(TARGET))
	$(Q)scp -q -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" ${TARGET} root@localhost://root
	$(call msg,INSTALL,,$(RUN_SCRIPT))
	$(Q)scp -q -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" ${RUN_SCRIPT} root@localhost://root

clean:
	$(call msg,CLEAN)
	$(Q)rm -f ${TARGET}
	$(Q)rm -f *.o


.PHONY: all build copy clean install format
