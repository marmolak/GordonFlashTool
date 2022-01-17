CFLAGS = -std=gnu99 -Wall -Wextra
objects = main.o metadata.o common.o images.o
files = main.c metadata.c common.c images.c

DEBUG_FLAGS = -O0 -ggdb3

.PHONY: all
all:
	$(CC) $(CFLAGS) -o gordon $(objects)

.PHONY: debug
debug: $(files)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $(files)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o gordon $(objects)

.PHONY: clean
clean:
	rm -f *.o