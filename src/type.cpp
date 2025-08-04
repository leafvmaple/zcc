#include "type.h"

#include "llvm_ir.h"

llvm::Type* IntType::Codegen(LLVMEnv* params) {
    return llvm::Type::getInt32Ty(params->TheContext);
}