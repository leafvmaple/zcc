#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>

#include "scanner/scanner.h"
#include "ir/codegen.h"

namespace fs = std::filesystem;

enum class Arch { NONE, X64, RISCV64, AARCH64 };

struct Options {
    Arch        arch = Arch::NONE;
    const char* input  = nullptr;
    const char* output = nullptr;
    std::string sysroot;         // -sysroot <dir>   (base for lib lookup)
    std::string linkerScript;    // -T <script>
    std::vector<std::string> libDirs;   // -L <dir> (repeatable)
    std::vector<std::string> includes;
    std::vector<std::string> libs;      // -l <name> (repeatable)
};

static void usage(const char* prog) {
    fprintf(stderr,
        "Usage: %s <-llvm|-x64|-riscv64|-aarch64> <input.c> -o <output> [options]\n"
        "\nOptions:\n"
        "  -sysroot <dir>   Runtime library root (default: <compiler>/../lib/<arch>)\n"
        "  -I <dir>         Header search path (repeatable)\n"
        "  -T <script>      Linker script\n"
        "  -L <dir>         Additional library search path (repeatable)\n"
        "  -l <name>        Link library lib<name>.a (repeatable)\n",
        prog);
    exit(1);
}

/* Resolve the default sysroot relative to the compiler binary */
static fs::path default_sysroot(const char* argv0, Arch arch) {
    auto exe = fs::canonical(fs::path(argv0)).parent_path();
    const char* archStr = arch == Arch::X64 ? "x64" : arch == Arch::RISCV64 ? "riscv64" : "aarch64";
    /* Try <exe>/../lib/<arch>  (installed layout) */
    auto p = exe.parent_path() / "lib" / archStr;
    if (fs::exists(p)) return p;
    /* Fallback: <exe>/lib/<arch>  (build-tree layout) */
    p = exe / "lib" / archStr;
    if (fs::exists(p)) return p;
    return exe.parent_path() / "lib" / archStr;
}

static Options parse_args(int argc, const char* argv[]) {
    Options opts;
    if (argc < 5) usage(argv[0]);

    /* First positional: arch mode */
    if (strcmp(argv[1], "-llvm") == 0)       opts.arch = Arch::NONE;
    else if (strcmp(argv[1], "-x64") == 0)   opts.arch = Arch::X64;
    else if (strcmp(argv[1], "-riscv64") == 0) opts.arch = Arch::RISCV64;
    else if (strcmp(argv[1], "-aarch64") == 0) opts.arch = Arch::AARCH64;
    else usage(argv[0]);

    /* Remaining args: input, -o output, then optional flags */
    opts.input = argv[2];
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            opts.output = argv[++i];
        } else if (strcmp(argv[i], "-sysroot") == 0 && i + 1 < argc) {
            opts.sysroot = argv[++i];
        } else if (strcmp(argv[i], "-I") == 0 && i + 1 < argc) {
            opts.includes.push_back(argv[++i]);
        } else if (strncmp(argv[i], "-I", 2) == 0) {
            opts.includes.push_back(argv[i] + 2);
        } else if (strcmp(argv[i], "-T") == 0 && i + 1 < argc) {
            opts.linkerScript = argv[++i];
        } else if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            opts.libDirs.push_back(argv[++i]);
        } else if (strncmp(argv[i], "-L", 2) == 0 && strlen(argv[i]) > 2) {
            opts.libDirs.push_back(argv[i] + 2);
        } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            opts.libs.push_back(argv[++i]);
        } else if (strncmp(argv[i], "-l", 2) == 0 && strlen(argv[i]) > 2) {
            opts.libs.push_back(argv[i] + 2);
        }
    }

    if (!opts.output) usage(argv[0]);
    return opts;
}

static std::string quote(const std::string& value) {
    std::string result = "'";
    for (char c : value) result += c == '\'' ? "'\\''" : std::string(1, c);
    return result + "'";
}

/* Run a shell command; abort on failure */
static void run(const std::string& cmd) {
    fprintf(stderr, "[zcc] %s\n", cmd.c_str());
    int ret = system(cmd.c_str());
    if (ret != 0) {
        fprintf(stderr, "[zcc] command failed (exit %d)\n", ret);
        exit(1);
    }
}

/* Search for a file in sysroot, then -L dirs */
static std::string find_file(const std::string& name, const fs::path& sysroot,
                              const std::vector<std::string>& libDirs) {
    auto p = sysroot / name;
    if (fs::exists(p)) return p.string();
    for (auto& dir : libDirs) {
        p = fs::path(dir) / name;
        if (fs::exists(p)) return p.string();
    }
    fprintf(stderr, "[zcc] cannot find %s\n", name.c_str());
    exit(1);
}

/* After LLVM IR is generated, produce a static ELF via llc + clang -c + ld */
static void link_elf(const Options& opts, const char* llFile, const char* argv0) {
    fs::path sysroot;
    if (!opts.sysroot.empty())
        sysroot = fs::path(opts.sysroot);
    else
        sysroot = default_sysroot(argv0, opts.arch);

    std::string target = opts.arch == Arch::X64 ? "x86_64-none-elf" : opts.arch == Arch::RISCV64 ? "riscv64-none-elf" : "aarch64-none-elf";
    // Match the kernel's saved register state: RV64 integer ABI, no AArch64 FP/SIMD.
    std::string llvm_flags = opts.arch == Arch::RISCV64 ? " -mattr=+m,+a,+c -target-abi=lp64" :
                             opts.arch == Arch::AARCH64 ? " -mattr=-fp-armv8,-neon" : " -mattr=+sse2,-avx";
    std::string assembler_flags = opts.arch == Arch::RISCV64 ? " -march=rv64imac -mabi=lp64 -mno-relax" : "";

    /* Resolve linker script, crt0.o, libzccrt.a */
    std::string linkerScript = opts.linkerScript.empty()
        ? find_file("linker.ld", sysroot, opts.libDirs)
        : opts.linkerScript;
    std::string crt0  = find_file("crt0.o",      sysroot, opts.libDirs);
    std::string rtLib = find_file("libzccrt.a",   sysroot, opts.libDirs);

    std::string sFile = std::string(opts.output) + ".s";
    std::string oFile = std::string(opts.output) + ".o";

    /* LLVM IR → assembly */
    run("llc -mtriple=" + target + llvm_flags + " -filetype=asm -O0 " + quote(llFile) + " -o " + quote(sFile));

    /* assembly → object */
    run("clang --target=" + target + assembler_flags + " -c " + quote(sFile) + " -o " + quote(oFile));

    /* link: crt0.o + user.o + libzccrt.a + extra -l libs → ELF */
    std::string ldCmd = "ld.lld --no-relax -T " + quote(linkerScript) + " -o " + quote(opts.output)
                      + " " + quote(crt0) + " " + quote(oFile) + " " + quote(rtLib);
    for (auto& dir : opts.libDirs)
        ldCmd += " -L" + quote(dir);
    for (auto& lib : opts.libs)
        ldCmd += " -l" + quote(lib);
    run(ldCmd);

    /* Cleanup intermediate files */
    std::remove(sFile.c_str());
    std::remove(oFile.c_str());
}

int main(int argc, const char *argv[]) {
    Options opts = parse_args(argc, argv);

    /* Frontend: source → LLVM IR */
    Scanner scanner{};
    CodeGen cg(opts.input);
    const std::string target = opts.arch == Arch::X64 ? "x86_64-none-elf" : opts.arch == Arch::RISCV64 ? "riscv64-none-elf" : "aarch64-none-elf";
    cg.SetTarget(target);
    const fs::path sysroot = opts.sysroot.empty() ? default_sysroot(argv[0], opts.arch) : fs::path(opts.sysroot);
    const std::string preprocessed = std::string(opts.output) + ".i";
    // Clang only preprocesses directives; parsing and C-to-IR generation remain zcc's.
    std::string cpp = "clang --target=" + target + " -E -P -x c -nostdinc -D__ZCC__=1 -I" + quote(sysroot.string());
    for (const auto& include : opts.includes) cpp += " -I" + quote(include);
    run(cpp + " " + quote(opts.input) + " -o " + quote(preprocessed));
    auto* file = fopen(preprocessed.c_str(), "r");
    if (!file) {
        fprintf(stderr, "Cannot open input: %s\n", opts.input);
        return 1;
    }
    try { scanner.Parse(file, &cg); cg.Optimize(); }
    catch (const std::exception& error) {
        fclose(file);
        std::remove(preprocessed.c_str());
        fprintf(stderr, "[zcc] %s\n", error.what());
        return 1;
    }
    fclose(file);
    std::remove(preprocessed.c_str());


    if (opts.arch == Arch::NONE) {
        /* -llvm: just dump IR */
        cg.Dump(opts.output);
        cg.Print();
    } else {
        /* -x64 / -riscv64: generate ELF */
        std::string tmpLL = std::string(opts.output) + ".ll";
        cg.Dump(tmpLL.c_str());

        link_elf(opts, tmpLL.c_str(), argv[0]);
        std::remove(tmpLL.c_str());

        fprintf(stderr, "[zcc] Generated ELF: %s\n", opts.output);
    }

    return 0;
}
