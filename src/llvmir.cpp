#include "ast.h"
#include "llvmir.h"

LLVMParams::LLVMParams(std::string moduleName) {
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>(std::forward<std::string>(moduleName), *TheContext);
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}