#pragma once

#include <cassert>

namespace llvm { class Type; }
class CodeGen;

struct BaseType {
    enum class TYPE { INT, CHAR, VOID };

    BaseType(TYPE type) : type(type) {}
    TYPE GetType() const { return type; }
    llvm::Type* Codegen(CodeGen* cg);

private:
    TYPE type;
};
