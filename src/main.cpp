#include <cstdio>
#include <string>

#include "scanner/scanner.h"
#include "ir/codegen.h"

int main(int argc, const char *argv[]) {
    auto input = argv[2];
    auto output = argv[4];

    Scanner scanner{};
    CodeGen cg(input);

    auto* file = fopen(input, "r");
    scanner.Parse(file, &cg);
    cg.Optimize();
    cg.Dump(output);
    cg.Print();

    fclose(file);
    return 0;
}
