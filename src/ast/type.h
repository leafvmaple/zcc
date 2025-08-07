#pragma once

#include <string>

using std::string;

class Env;
class KoopaEnv;

struct BaseType {
    enum class TYPE {
        INT,
        VOID,
    };
    BaseType(TYPE type) : type(type) {}

    void* Codegen(Env* env);

private:
    TYPE type;
};
