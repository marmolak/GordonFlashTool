.PHONY: all
all: boot_code.asm
	nasm -f bin -o boot_code boot_code.asm
	xxd -i boot_code > boot_code_generated.h
	rm -rf boot_code

.PHONY: boot
boot: boot_code.asm
	nasm -f bin -DBOOTTEST -o boot.img boot_code.asm
