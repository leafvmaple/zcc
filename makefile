# Based on https://matansilver.com/2017/08/29/universal-makefile/
# Modified by MaxXing

# Settings
# Set to 0 to enable C mode
CPP_MODE := 1
ifeq ($(CPP_MODE), 0)
FB_EXT := .c
else
FB_EXT := .cpp
endif

# Flags
CFLAGS := -Wall -std=c11
CXXFLAGS := -Wall -Wno-register -std=c++17 -gdwarf-4 -Wno-c99-designator -Wno-unused-but-set-variable
FFLAGS :=
BFLAGS := -d
LDFLAGS :=

# Debug flags
DEBUG ?= 1
ifeq ($(DEBUG), 0)
CFLAGS += -O2
CXXFLAGS += -O2
else
CFLAGS += -g -O0
CXXFLAGS += -g -O0
endif

# Compilers
CC := clang
CXX := clang++
FLEX := flex
BISON := bison

# Directories
TOP_DIR := $(shell pwd)
TARGET_EXEC := compiler
SRC_DIR := $(TOP_DIR)/src
FB_DIR := $(TOP_DIR)/parser
BUILD_DIR ?= $(TOP_DIR)/build
LIB_DIR ?= $(CDE_LIBRARY_PATH)/native
INC_DIR ?= $(CDE_INCLUDE_PATH)
# CFLAGS += -I$(INC_DIR)
# CXXFLAGS += -I$(INC_DIR)
LDFLAGS += -L$(LIB_DIR)
LLVM_LDFLAGS := $(shell llvm-config --ldflags --system-libs --libs core)

# Source files & target files
FB_SRCS := $(patsubst $(FB_DIR)/%.l, $(BUILD_DIR)/%.lex$(FB_EXT), $(shell find $(FB_DIR) -name "*.l"))
FB_SRCS += $(patsubst $(FB_DIR)/%.y, $(BUILD_DIR)/%.tab$(FB_EXT), $(shell find $(FB_DIR) -name "*.y"))
SRCS := $(FB_SRCS) $(shell find $(SRC_DIR) -path $(SRC_DIR)/runtime -prune -o \( -name "*.c" -or -name "*.cpp" -or -name "*.cc" \) -print)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(SRCS))
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.cpp.o, $(OBJS))
OBJS := $(patsubst $(SRC_DIR)/%.cc, $(BUILD_DIR)/%.cc.o, $(OBJS))
OBJS := $(patsubst $(BUILD_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(OBJS))
OBJS := $(patsubst $(BUILD_DIR)/%.cpp, $(BUILD_DIR)/%.cpp.o, $(OBJS))
OBJS := $(patsubst $(BUILD_DIR)/%.cc, $(BUILD_DIR)/%.cc.o, $(OBJS))

# Header directories & dependencies
INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_DIRS += $(INC_DIRS:$(SRC_DIR)%=$(BUILD_DIR)%)
INC_FLAGS := $(addprefix -I, $(INC_DIRS))
DEPS := $(OBJS:.o=.d)
CPPFLAGS = $(INC_FLAGS) -MMD -MP
LLVM_CXXFLAGS := $(shell llvm-config --cxxflags)

# Main target
all: $(BUILD_DIR)/$(TARGET_EXEC)
$(BUILD_DIR)/$(TARGET_EXEC): $(FB_SRCS) $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LLVM_LDFLAGS) -DYYDEBUG=1 -lpthread -ldl -o $@

# C source
define c_recipe
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
endef
$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c; $(c_recipe)
$(BUILD_DIR)/%.c.o: $(BUILD_DIR)/%.c; $(c_recipe)

# C++ source
define cxx_recipe
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LLVM_CXXFLAGS) -c $< -o $@
endef
$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp; $(cxx_recipe)
$(BUILD_DIR)/%.cpp.o: $(BUILD_DIR)/%.cpp; $(cxx_recipe)
$(BUILD_DIR)/%.cc.o: $(SRC_DIR)/%.cc; $(cxx_recipe)

# Flex
$(BUILD_DIR)/%.lex$(FB_EXT): $(FB_DIR)/%.l
	mkdir -p $(dir $@)
	$(FLEX) $(FFLAGS) --header-file=$(BUILD_DIR)/$*.lex.hpp -o $@ $<

# Bison
$(BUILD_DIR)/%.tab$(FB_EXT): $(FB_DIR)/%.y
	mkdir -p $(dir $@)
	$(BISON) $(BFLAGS) -o $@ $<


.PHONY: clean test lib-x64 lib-riscv64 lib elf-x64 elf-riscv64

clean:
	-rm -rf $(BUILD_DIR)
	$(MAKE) -C $(TOP_DIR)/src/runtime clean

# ---- Regression tests ----
# Compiles each test/cases/*.c to LLVM IR, builds it with the host clang
# (libc as the runtime oracle), runs it, and diffs stdout against *.expected.
test: all
	@bash $(TOP_DIR)/test/run_tests.sh

# ---- Runtime library targets ----
lib-x64:
	$(MAKE) -C $(TOP_DIR)/src/runtime x64

lib-riscv64:
	$(MAKE) -C $(TOP_DIR)/src/runtime riscv64

lib: lib-x64 lib-riscv64

# ---- Compile test programs ----
llvm: all
	$(BUILD_DIR)/$(TARGET_EXEC) -llvm test/hello.c -o test/hello.ll

elf-x64: all lib-x64
	$(BUILD_DIR)/$(TARGET_EXEC) -x64 test/hello.c -o test/hello_x64 -sysroot $(TOP_DIR)/lib/x64

elf-riscv64: all lib-riscv64
	$(BUILD_DIR)/$(TARGET_EXEC) -riscv64 test/hello.c -o test/hello_riscv64 -sysroot $(TOP_DIR)/lib/riscv64

riscv: llvm
	llc -march=riscv32 -filetype=asm -O0 test/hello.ll -o test/hello.s

-include $(DEPS)