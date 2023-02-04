TARGET=test_mod

PWD := $(shell pwd)
obj-m += ${TARGET}.o
${TARGET}-y := test_module.o exec_context.o
CFLAGS_test_module.o := -DDEBUG

KDIR := /lib/modules/$(shell uname -r)/build
#KDIR := /home/ilya/share/linux-5.15.71

all:
	make -C ${KDIR} M=$(PWD) modules

clean:
	make -C ${KDIR} M=$(PWD) clean

modules_install:
	make -C ${KDIR} M=$(PWD) modules_install

install:
	insmod ${TARGET}.ko && lsmod | grep ${TARGET}

uninstall:
	rmmod ${TARGET}

sa_sparse:
	make clean
	make C=2 CHECK="/usr/bin/sparse" -C ${KDIR} M=$(PWD) modules

sa_gcc:
	make clean
	make W=1 -C ${KDIR} M=$(PWD) modules

sa_flawfinder:
	make clean
	flawfinder *.[ch]

.PHONY: clean install uninstall
