#include "type.h"
#include "ir/codegen.h"

llvm::Type* BaseType::Codegen(CodeGen* cg) {
    switch (type) {
        case TYPE::INT:
            return cg->GetInt32Type();
        case TYPE::CHAR:
            return cg->GetInt8Type();
        case TYPE::VOID:
            return cg->GetVoidType();
        default:
            assert(false && "Unknown type");
            return nullptr;
    }
}
