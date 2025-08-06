#pragma once

#include <string>

using std::string;

class Env;
class KoopaEnv;

struct BaseType {
    virtual ~BaseType() = default;

    virtual void* Codegen(Env* params) = 0;
};

struct IntType : public BaseType {
    void* Codegen(Env* params) override;
};