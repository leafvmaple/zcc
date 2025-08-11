#pragma once

#include <string>
#include <cassert>

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

// template

template<typename Type, typename Value, typename BasicBlock, typename Function>
Type* BaseType::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
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