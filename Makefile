all: metadata.o
	$(CC) -std=gnu99 -DNDEBUG -Wall -Wextra -o gordon metadata.c main.c

debug: metadata.o
	$(CC) -std=gnu99 -O0 -ggdb3 -Wall -Wextra -o gordon metadata.c main.c
