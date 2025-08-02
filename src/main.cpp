#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

#include "Scanner.h"
#include "llvm_ir.h"
#include "koopa_ir.h"

#include "llvm/Support/raw_os_ostream.h"

int main(int argc, const char *argv[]) {
  assert(argc == 5);
  auto mode = std::string(argv[1]);
  auto input = argv[2];
  auto output = argv[4];

  Scanner scanner;

  auto file = fopen(input, "r");
  scanner.parse(file, std::make_unique<CompUnitAST>());

  if (mode == "-llvm") {
    LLVMParams llvmParams(input);

    std::ofstream outFile(output);
    llvm::raw_os_ostream rawOutFile(outFile);

    scanner.ast->Codegen(&llvmParams);
    llvmParams.TheModule.print(llvm::outs(), nullptr);
    llvmParams.TheModule.print(rawOutFile, nullptr);
  }
  else if(mode == "-koopa") {
    KoopaEnv env;
    auto rawProgram = (koopa_raw_program_t*)scanner.ast->Parse(&env);

    koopa_program_t program = nullptr;
    auto res = koopa_generate_raw_to_koopa(rawProgram, &program);
    if (res != KOOPA_EC_SUCCESS) {
      cerr << "Error generating Koopa IR: " << res << endl;
      return 1;
    }
    
    koopa_dump_to_stdout(program);
    koopa_dump_to_file(program, output);
  }

  return 0;
}