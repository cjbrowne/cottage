CFLAGS := -Wall -Werror -nostdinc -fno-builtin -nostartfiles -nodefaultlibs -ffreestanding
LDFLAGS := -nostdlib -nodefaultlibs -nostartfiles -z noexecstack
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
SRC := $(wildcard $(SRC_DIR)/**/*.c $(SRC_DIR)/*.c)
ASM_SRC := $(wildcard $(SRC_DIR)/**/*.S $(SRC_DIR)/*.S)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC)) $(patsubst $(SRC_DIR)/%.S, $(OBJ_DIR)/%.o, $(ASM_SRC))
# use clang as linker and compiler
CC := clang
LD := clang

LDFLAGS_LINUX :=
LDFLAGS_COTTAGE := 

.PHONY: all
all: $(BIN_DIR)/libc-linux.so $(BIN_DIR)/libc-cottage.so

$(BIN_DIR)/libc-linux.so: $(OBJS) | always
	$(LD) $(LDFLAGS) $(LDFLAGS_LINUX) -o $@ $^

$(BIN_DIR)/libc-cottage.so: $(OBJS) | always
	$(LD) $(LDFLAGS) $(LDFLAGS_COTTAGE) -o $@ $^

.PHONY: always
always:
	mkdir -p $(BIN_DIR) 
	mkdir -p $(OBJ_DIR)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | always
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S | always
	$(CC) $(CFLAGS) -c $< -o $@

