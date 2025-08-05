#pragma once

#include <string>

#include "llvm/IR/Function.h"

#include "../libkoopa/include/koopa.h"

#include "koopa_ir.h"

using std::string;

class Env;
class KoopaEnv;

struct BaseType {
    virtual ~BaseType() = default;

    virtual void* Codegen(Env* params) = 0;
    virtual void* ToKoopa(KoopaEnv* env) = 0;
    virtual void* ToPointer(KoopaEnv* env) = 0;
};

struct IntType : public BaseType {
    void* Codegen(Env* params) override;

    void* ToKoopa(KoopaEnv* env) override {
        return new koopa_raw_type_kind_t { KOOPA_RTT_INT32 };
    }

    void* ToPointer(KoopaEnv* env) override {
        return new koopa_raw_type_kind_t {
            .tag = KOOPA_RTT_POINTER,
            .data.pointer = {
                .base = koopa_pointer(KOOPA_RTT_INT32)
            }
        };
    }
};