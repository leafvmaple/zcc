#pragma once

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

class LLVMParams {
public:
    LLVMParams(std::string moduleName)
        : TheModule(std::forward<std::string>(moduleName), TheContext), Builder(TheContext){};
    ~LLVMParams() = default;

    llvm::LLVMContext TheContext;
    llvm::Module TheModule;
    llvm::IRBuilder<llvm::NoFolder, llvm::IRBuilderDefaultInserter> Builder;
};