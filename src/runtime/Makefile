# Build minimal C runtime static libraries for custom OS
#
# Source:  src/runtime/  (this directory)
# Build:   build/lib/    (intermediate .o files)
# Output:  lib/          (final artifacts: libzccrt.a, crt0.o, linker.ld)

SRC_DIR  := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ROOT_DIR := $(SRC_DIR)/../..
BUILD_DIR := $(ROOT_DIR)/build/lib
LIB_DIR   := $(ROOT_DIR)/lib

# Toolchains
X64_CC    := clang
X64_AS    := clang
X64_AR    := ar

RV64_CC   := clang --target=riscv64
RV64_AS   := clang --target=riscv64
RV64_AR   := ar

CFLAGS := -ffreestanding -fno-builtin -nostdlib -O2 -Wall

# Default: use local syscall.h; override ABI_DIR to use an external OS header
ABI_DIR ?= $(SRC_DIR)
ASFLAGS := -I$(ABI_DIR)

.PHONY: all clean x64 riscv64

all: x64 riscv64

# ---- x86_64 ----
x64: $(LIB_DIR)/x64/libzccrt.a $(LIB_DIR)/x64/crt0.o $(LIB_DIR)/x64/linker.ld

$(BUILD_DIR)/x64/printf.o: $(SRC_DIR)/printf.c
	@mkdir -p $(dir $@)
	$(X64_CC) $(CFLAGS) --target=x86_64 -c $< -o $@

$(BUILD_DIR)/x64/syscall.o: $(SRC_DIR)/x64/syscall.S
	@mkdir -p $(dir $@)
	$(X64_AS) --target=x86_64 $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/x64/crt0.o: $(SRC_DIR)/x64/crt0.S
	@mkdir -p $(dir $@)
	$(X64_AS) --target=x86_64 $(ASFLAGS) -c $< -o $@

$(LIB_DIR)/x64/libzccrt.a: $(BUILD_DIR)/x64/printf.o $(BUILD_DIR)/x64/syscall.o
	@mkdir -p $(dir $@)
	$(X64_AR) rcs $@ $^

$(LIB_DIR)/x64/crt0.o: $(BUILD_DIR)/x64/crt0.o
	@mkdir -p $(dir $@)
	cp $< $@

$(LIB_DIR)/x64/linker.ld: $(SRC_DIR)/x64/linker.ld
	@mkdir -p $(dir $@)
	cp $< $@

# ---- RISC-V 64 ----
riscv64: $(LIB_DIR)/riscv64/libzccrt.a $(LIB_DIR)/riscv64/crt0.o $(LIB_DIR)/riscv64/linker.ld

$(BUILD_DIR)/riscv64/printf.o: $(SRC_DIR)/printf.c
	@mkdir -p $(dir $@)
	$(RV64_CC) $(CFLAGS) -march=rv64gc -c $< -o $@

$(BUILD_DIR)/riscv64/syscall.o: $(SRC_DIR)/riscv64/syscall.S
	@mkdir -p $(dir $@)
	$(RV64_AS) -march=rv64gc $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/riscv64/crt0.o: $(SRC_DIR)/riscv64/crt0.S
	@mkdir -p $(dir $@)
	$(RV64_AS) -march=rv64gc $(ASFLAGS) -c $< -o $@

$(LIB_DIR)/riscv64/libzccrt.a: $(BUILD_DIR)/riscv64/printf.o $(BUILD_DIR)/riscv64/syscall.o
	@mkdir -p $(dir $@)
	$(RV64_AR) rcs $@ $^

$(LIB_DIR)/riscv64/crt0.o: $(BUILD_DIR)/riscv64/crt0.o
	@mkdir -p $(dir $@)
	cp $< $@

$(LIB_DIR)/riscv64/linker.ld: $(SRC_DIR)/riscv64/linker.ld
	@mkdir -p $(dir $@)
	cp $< $@

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)
