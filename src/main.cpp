#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

using namespace std;

#include "scanner/scanner.h"
#include "ir/llvm_ir.h"
#include "ir/koopa_ir.h"

int main(int argc, const char *argv[]) {
  auto mode = std::string(argv[1]);
  auto input = argv[2];
  auto output = argv[4];
  auto file = fopen(input, "r");

  Scanner scanner{};
  Env* env{};

  if (mode == "-llvm") {
    env = new LLVMEnv(input);
  }
  else if(mode == "-koopa") {
    env = new KoopaEnv();
  }

  scanner.Parse(file, env);

  // env->CleanUp();
  env->Dump(output);
  env->Print();

  fclose(file);
  delete env;

  return 0;
}