# zcc

## Install

```shell
sudo apt install flex bison
sudo apt install clang
sudo apt install llvm-dev
```

## Build

```shell
make
```

## Test

```shell
make test
```

## Freestanding user programs

`-x64`, `-riscv64` and `-aarch64` compile user C through zcc's parser and
LLVM IR generator. LLVM's `llc`, assembler and linker produce the final static
ELF. Clang is used for preprocessing and for building the existing low-level
runtime library, not as a substitute C frontend for applications.

```sh
make
make lib
build/compiler -aarch64 app.c -o app.elf -sysroot lib/aarch64
```

The same runtime interface (`src/runtime/minilib.h`) and initial stack layout
are used on every target: 64-bit argc, argv pointers, NULL, empty environment.
`crt0.S` passes argc/argv to main and exits with its result. The runtime includes
file calls, spawn/wait/kill, PID/yield, stat/readdir and basic string operations.
RISC-V uses the integer RV64IMAC/lp64 ABI; AArch64 does not require FP/SIMD state.

The frontend remains a C subset. The user-process foundation supports char,
int, 64-bit long, pointer/array declarations, prototypes, named structures,
address/dereference, casts and sizeof(type). Fixed-size automatic storage is
allocated once per function invocation, including declarations inside loops.
Unsupported syntax is an error; there is no fallback to another C compiler.
The OS and standalone runtime ABI header copies must be synchronized.
