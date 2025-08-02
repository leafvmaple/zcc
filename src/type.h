#pragma once

#include <string>

#include "llvm/IR/Function.h"

#include "../libkoopa/include/koopa.h"

using std::string;

class LLVMParams;
class KoopaEnv;

struct BaseType {
    virtual ~BaseType() = default;
    virtual string ToString() const = 0;
    virtual llvm::Type* Codegen(LLVMParams* params) = 0;

    virtual void* ToKoopa(KoopaEnv* env) = 0;
};

struct IntType : public BaseType {
    string ToString() const override { return "int"; }
    llvm::Type* Codegen(LLVMParams* params) override;

    void* ToKoopa(KoopaEnv* env) override {
        return new koopa_raw_type_kind_t { KOOPA_RTT_INT32 };
    }
};