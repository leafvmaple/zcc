#include "type.h"

#include "llvmir.h"

llvm::Type* IntType::Codegen(LLVMParams* params) {
    return llvm::Type::getInt32Ty(params->TheContext);
}