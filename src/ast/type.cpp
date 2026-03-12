#include "type.h"
#include "ir/codegen.h"

llvm::Type* BaseType::Codegen(CodeGen* cg) {
    switch (type) {
        case TYPE::INT:
        case TYPE::CHAR:
            return cg->GetInt32Type();
        case TYPE::VOID:
            return cg->GetVoidType();
        default:
            assert(false && "Unknown type");
            return nullptr;
    }
}
