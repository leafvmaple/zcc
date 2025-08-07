#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

using namespace std;

#include "scanner/scanner.h"
#include "ir/llvm_ir.h"
#include "ir/koopa_ir.h"

template<typename EnvType>
void processFile(const char* input, const char* output) {
    Scanner scanner{};
    EnvType env(input);
    
    auto file = fopen(input, "r");
    scanner.Parse(file, &env);
    env.Pass();
    env.Dump(output);
    env.Print();

    fclose(file);
}

int main(int argc, const char *argv[]) {
    auto mode = std::string(argv[1]);
    auto input = argv[2];
    auto output = argv[4];

    if (mode == "-llvm") {
        processFile<LLVMEnv>(input, output);
    }
    else if(mode == "-koopa") {
        processFile<KoopaEnv>(input, output);
    }

    return 0;
}