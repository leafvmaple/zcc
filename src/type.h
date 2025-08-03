#pragma once

#include <string>

#include "llvm/IR/Function.h"

#include "../libkoopa/include/koopa.h"

#include "koopa_ir.h"

using std::string;

class LLVMParams;
class KoopaEnv;

struct BaseType {
    virtual ~BaseType() = default;
    virtual string ToString() const = 0;
    virtual llvm::Type* Codegen(LLVMParams* params) = 0;

    virtual koopa_raw_type_t ToKoopa(KoopaEnv* env) = 0;
    virtual koopa_raw_type_t ToPointer(KoopaEnv* env) = 0;
};

struct IntType : public BaseType {
    string ToString() const override { return "int"; }
    llvm::Type* Codegen(LLVMParams* params) override;

    koopa_raw_type_t ToKoopa(KoopaEnv* env) override {
        return new koopa_raw_type_kind_t { KOOPA_RTT_INT32 };
    }

    koopa_raw_type_t ToPointer(KoopaEnv* env) override {
        return new koopa_raw_type_kind_t {
            .tag = KOOPA_RTT_POINTER,
            .data.pointer = {
                .base = koopa_pointer(KOOPA_RTT_INT32)
            }
        };
    }
};