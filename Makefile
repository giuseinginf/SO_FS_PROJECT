CC = gcc
SRC_DIR = src
BIN_DIR = bin
OBJS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/shell/shell.c \
       $(SRC_DIR)/shell/shell_commands.c \
       $(SRC_DIR)/fs/disk.c \
       $(SRC_DIR)/fs/fat.c \
       $(SRC_DIR)/fs/entry.c \
       $(SRC_DIR)/utils/utils.c
CFLAGS = -Wall -Wextra -g

$(BIN_DIR)/fs-shell: $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN_DIR)/fs-shell

clean:
	rm -rf $(BIN_DIR)