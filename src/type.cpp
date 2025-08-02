#include "type.h"

#include "llvm_ir.h"

llvm::Type* IntType::Codegen(LLVMParams* params) {
    return llvm::Type::getInt32Ty(params->TheContext);
}