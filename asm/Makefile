.PHONY: all
all: boot_code.asm
	nasm -f bin -o boot_code boot_code.asm
	xxd -s 32 -i boot_code > boot_code_generated.h
	rm -rf boot_code
