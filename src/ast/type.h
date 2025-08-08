#pragma once

#include <string>

using std::string;

template<typename T, typename V, typename B, typename F>
class Env;
class KoopaEnv;

struct BaseType {
    enum class TYPE {
        INT,
        VOID,
    };
    BaseType(TYPE type) : type(type) {}

    template<typename T, typename V, typename B, typename F>
    T* Codegen(Env<T, V, B, F>* env);

private:
    TYPE type;
};
