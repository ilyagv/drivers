
TARGET=mld_newpack_bot
PUBLIC_KEY_PATH=~/ssd2/test/test_debootstrap/bookworm.id_rsa

CFLAGS=-Wall -static
LDFLAGS=-static-libgcc

OBJS:= repro1.o 

all: clean $(TARGET) install 

%.o: %.c
	$(CC) $(CFLAGS) $(TARGET_ARCH) -c $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET_ARCH) $^ -o $@

install:
	scp -i ${PUBLIC_KEY_PATH} -P 10021 -o "StrictHostKeyChecking=no" ${TARGET} root@localhost://root

clean:
	rm -f ${TARGET}

.PHONY: all build copy clean
