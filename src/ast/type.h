#pragma once

#include <string>

using std::string;

template<typename V>
class Env;
class KoopaEnv;

struct BaseType {
    enum class TYPE {
        INT,
        VOID,
    };
    BaseType(TYPE type) : type(type) {}

    template<typename V>
    void* Codegen(Env<V>* env);

private:
    TYPE type;
};
