CFLAGS = -std=gnu99 -Wall -Wextra
objects = main.o metadata.o common.o images.o file_dev_ops.o crypt_md5.o fat_driver.o tools.o
files = main.c metadata.c common.c images.c file_dev_ops.c crypt_md5.c fat/fat_driver.c tools.c

RELEASE_FLAGS = -DNDEBUG -I.
DEBUG_FLAGS = -O0 -ggdb3 -I.

.PHONY: all
all:
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -c $(files)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o gordon $(objects)

.PHONY: debug
debug: $(files)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $(files)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o gordon $(objects)

.PHONY: clean
clean:
	rm -f *.o
	rm -f gordon
