#pragma once

#include <string>

using std::string;

template<typename Type, typename Value, typename BasicBlock, typename Function>
class Env;
class KoopaEnv;

struct BaseType {
    enum class TYPE {
        INT,
        VOID,
    };
    BaseType(TYPE type) : type(type) {}

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Type* Codegen(Env<Type, Value, BasicBlock, Function>* env);

private:
    TYPE type;
};
