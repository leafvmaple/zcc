#include "ir/ir.h"

#include "type.h"

#include <cassert>

void* BaseType::Codegen(Env* env) {
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
