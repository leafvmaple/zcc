#include "ir/ir.h"

#include "type.h"

#include "llvm/IR/Value.h"
#include "../libkoopa/include/koopa.h"
#include "../libkoopa/include/function.h"

#include <cassert>

template<typename T, typename V, typename B, typename F>
T* BaseType::Codegen(Env<T, V, B, F>* env) {
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

template llvm::Type* BaseType::Codegen(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>* env);
template koopa::Type* BaseType::Codegen(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>* env);