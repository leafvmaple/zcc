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
        CHAR,
        VOID,
    };
    BaseType(TYPE type) : type(type) {}

    TYPE GetType() const { return type; }

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
        case TYPE::CHAR:
            return env->GetInt32Type();
        case TYPE::VOID:
            return env->GetVoidType();
        default:
            assert(false && "Unknown type");
            return nullptr;
    }
}