#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

#include "Scanner.h"
#include "llvmir.h"

#include "llvm/Support/raw_os_ostream.h"

int main(int argc, const char *argv[]) {
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  Scanner scanner;
  LLVMParams llvmParams(input);

  auto file = fopen(input, "r");
  assert(file);
  
  scanner.parse(file, std::make_unique<CompUnitAST>());

  cout << "AST: " << scanner.ast->ToString() << endl;


  std::ofstream outFile(output);
  llvm::raw_os_ostream rawOutFile(outFile);

  scanner.ast->Codegen(&llvmParams);
  llvmParams.TheModule.print(llvm::outs(), nullptr);
  llvmParams.TheModule.print(rawOutFile, nullptr);
  return 0;
}