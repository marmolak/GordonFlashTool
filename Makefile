CFLAGS := -std=gnu99 -Wall -Wextra
objects := main.o metadata.o common.o images.o file_dev_ops.o crypt_md5.o fat_driver.o tools.o
files := main.c metadata.c common.c images.c file_dev_ops.c crypt_md5.c fat/fat_driver.c tools.c

ASM_DIR := asm

COMMON_FLAGS := -I. -Wall -Wextra

RELEASE_FLAGS := -DNDEBUG $(COMMON_FLAGS)
DEBUG_FLAGS := -O0 -ggdb3 -Weverything $(COMMON_FLAGS)

.PHONY: all
all:
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -c $(files)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o gordon $(objects)

.PHONY: debug
debug:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $(files)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o gordon $(objects)

.PHONY: all-boot-code
all-boot-code: bootcode
	$(CC) -DBOOT_CODE $(CFLAGS) $(RELEASE_FLAGS) -c $(files)
	$(CC) -DBOOT_CODE $(CFLAGS) $(RELEASE_FLAGS) -o gordon $(objects)

.PHONY: debug-boot-code
debug-boot-code: bootcode
	$(CC) -DBOOT_CODE $(CFLAGS) $(DEBUG_FLAGS) -c $(files)
	$(CC) -DBOOT_CODE $(CFLAGS) $(DEBUG_FLAGS) -o gordon $(objects)

.PHONY: bootcode
bootcode:
	$(MAKE) -C $(ASM_DIR)

.PHONY: clean
clean:
	rm -f *.o
	rm -f gordon

.PHONY: test-boot
test-boot: all-boot-code
	touch disk.img
	./gordon -d ./disk.img -s0 -f
	qemu-system-i386 -cpu 486 -fda ./disk.img
