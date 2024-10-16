
BOOT_SECT_FILE=boot_sect.bin

all: build_boot_sect

build_boot_sect: boot_sect.asm
	nasm boot_sect.asm -f bin -o ${BOOT_SECT_FILE}
	od -t x1 -A n ${BOOT_SECT_FILE}

clean:
	rm -f ${BOOT_SECT_FILE}
