#include "ir/ir.h"

#include "type.h"

#include "llvm/IR/Value.h"
#include "../libkoopa/include/koopa.h"

#include <cassert>

template<typename V>
void* BaseType::Codegen(Env<V>* env) {
    switch (type) {
        case TYPE::INT:
            return env->GetInt32Type();
        case TYPE::VOID:
            return env->GetVoidType();
        default:
            assert(false && "Unknown type");
            return nullptr;
    }
}

template void* BaseType::Codegen<llvm::Value>(Env<llvm::Value>* env);
template void* BaseType::Codegen<koopa_raw_value_data>(Env<koopa_raw_value_data>* env);