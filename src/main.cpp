#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

using namespace std;

#include "scanner/scanner.h"
#include "ir/llvm_ir.h"
#include "ir/koopa_ir.h"

int main(int argc, const char *argv[]) {
  assert(argc == 5);
  auto mode = std::string(argv[1]);
  auto input = argv[2];
  auto output = argv[4];

  Scanner scanner;

  auto file = fopen(input, "r");
  scanner.parse(file, std::make_unique<CompUnitAST>());

  if (mode == "-llvm") {
    LLVMEnv env(input);
    scanner.ast->Codegen(&env);
    
    env.CleanUp();
    env.Print();
    env.Dump(output);
  }
  else if(mode == "-koopa") {
    KoopaEnv env;
    scanner.ast->Codegen(&env);

    env.Dump(output);
    env.Print();
  }

  return 0;
}