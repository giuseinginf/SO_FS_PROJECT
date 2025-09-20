CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
SRCS=main.c disk.c shell.c fat.c
OBJ=$(SRCS:.c=.o)

all: myFS

myFS: $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f myFS $(OBJ)
	rm -f virtual_disk.img
