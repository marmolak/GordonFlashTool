CFLAGS = -std=gnu99 -Wall -Wextra
objects = metadata.o

DEBUG_FLAGS = -O0 -ggdb3

ifeq '' '$(findstring clang,$(CC))'
	DEBUG_FLAGS += -fsanitize=undefined
endif

.PHONY: all
all: $(objects)
	$(CC) $(CFLAGS) -o gordon metadata.o main.c

.PHONY: debug
debug: $(objects)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o gordon metadata.o main.c
