#include "ir/ir.h"

#include "ast.h"

void* NumberAST::Codegen(Env* env) {
    return env->GetInt32(value);
}
