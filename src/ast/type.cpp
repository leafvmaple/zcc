#include "ir/ir.h"

#include "type.h"

void* IntType::Codegen(Env* env) {
    return env->GetInt32Type();
}

void* VoidType::Codegen(Env* env) {
    return env->GetVoidType();
}