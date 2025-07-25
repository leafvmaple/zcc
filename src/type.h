#pragma once

#include <string>

#include "llvm/IR/Function.h"

using std::string;

class LLVMParams;

struct BaseType {
    virtual ~BaseType() = default;
    virtual string ToString() const = 0;
    virtual llvm::Type* Codegen(LLVMParams* params) = 0;
};

struct IntType : public BaseType {
    string ToString() const override { return "int"; }
    llvm::Type* Codegen(LLVMParams* params) override;
};