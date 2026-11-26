#include "type.h"
#include "ir/codegen.h"
llvm::Type* BaseType::Codegen(CodeGen* cg) const {
    switch (type) {
    case TYPE::INT: return cg->GetInt32Type();
    case TYPE::CHAR: return cg->GetInt8Type();
    case TYPE::LONG: return cg->GetInt64Type();
    case TYPE::VOID: return cg->GetVoidType();
    case TYPE::POINTER: return cg->GetPointerType(element->Codegen(cg));
    case TYPE::ARRAY: return cg->GetArrayType(element->Codegen(cg), count);
    case TYPE::STRUCT: return cg->StructType(tag);
    }
    throw std::runtime_error("unknown source type");
}
