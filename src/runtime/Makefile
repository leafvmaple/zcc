SRC_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ROOT_DIR := $(SRC_DIR)/../..
BUILD_DIR := $(ROOT_DIR)/build/lib
LIB_DIR := $(ROOT_DIR)/lib
ABI_DIR ?= $(SRC_DIR)
CLANG ?= clang
AR := llvm-ar
ARCHES := x64 riscv64 aarch64
TARGET_x64 := x86_64-none-elf
TARGET_riscv64 := riscv64-none-elf
TARGET_aarch64 := aarch64-none-elf
FLAGS_x64 := -m64 -mno-red-zone -march=x86-64 -msse2 -mno-avx
FLAGS_riscv64 := -march=rv64imac -mabi=lp64 -mno-relax
FLAGS_aarch64 := -mgeneral-regs-only
CFLAGS := -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -nostdlib -O2 -Wall
HEADERS := $(SRC_DIR)/minilib.h $(ABI_DIR)/syscall.h $(ABI_DIR)/process.h $(ABI_DIR)/fcntl.h $(ABI_DIR)/terminal.h
.PHONY: all clean $(ARCHES)
all: $(ARCHES)

define runtime
$(1): $(LIB_DIR)/$(1)/libzccrt.a $(LIB_DIR)/$(1)/crt0.o $(LIB_DIR)/$(1)/linker.ld $(addprefix $(LIB_DIR)/$(1)/,$(notdir $(HEADERS)))
$(BUILD_DIR)/$(1)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $$(@D)
	$(CLANG) --target=$(TARGET_$(1)) $(FLAGS_$(1)) $(CFLAGS) -I$(ABI_DIR) -I$(SRC_DIR) -c $$< -o $$@
$(BUILD_DIR)/$(1)/%.o: $(SRC_DIR)/$(1)/%.S $(HEADERS)
	@mkdir -p $$(@D)
	$(CLANG) --target=$(TARGET_$(1)) $(FLAGS_$(1)) -I$(ABI_DIR) -c $$< -o $$@
$(LIB_DIR)/$(1)/libzccrt.a: $(addprefix $(BUILD_DIR)/$(1)/,printf.o string.o syscall.o)
	@mkdir -p $$(@D)
	$(AR) rcs $$@ $$^
$(LIB_DIR)/$(1)/crt0.o: $(BUILD_DIR)/$(1)/crt0.o
	@mkdir -p $$(@D)
	cp $$< $$@
$(LIB_DIR)/$(1)/linker.ld: $(SRC_DIR)/$(1)/linker.ld
	@mkdir -p $$(@D)
	cp $$< $$@
$(LIB_DIR)/$(1)/minilib.h: $(SRC_DIR)/minilib.h
	@mkdir -p $$(@D)
	cp $$< $$@
$(LIB_DIR)/$(1)/%.h: $(ABI_DIR)/%.h
	@mkdir -p $$(@D)
	cp $$< $$@
endef
$(foreach arch,$(ARCHES),$(eval $(call runtime,$(arch))))
clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)
