#include "type.h"

#include "ir.h"

void* IntType::Codegen(Env* env) {
    return env->GetInt32Type();
}