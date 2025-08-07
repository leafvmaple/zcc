#pragma once

#include "ast/ast.h"
#include "ir/ir.h"

class Generator {
public:
    Generator(Env *env) : env(env) {};

    void Generate(CompUnitAST& ast) {
        for (auto&& funcDef : ast.funcDef) {
            Generate(std::move(funcDef));
        }
    }
    void Generate(std::unique_ptr<FuncDefAST> funcDef) {
        std::vector<void*> types;
        std::vector<std::string> names;
        
        for (auto& param : funcDef->params) {
            types.push_back(param->btype->Codegen(env));
            names.push_back(param->ident);
        }

        auto* type = env->CreateFuncType(funcDef->funcType->Codegen(env), types);
        auto* func = env->CreateFunction(type, funcDef->ident, names);
        auto* bb = env->CreateBasicBlock("entry", func);

        env->SetInserPointer(bb);

        for (size_t i = 0; i < funcDef->params.size(); ++i) {
            env->CreateStore(env->GetFunctionArg(i), funcDef->params[i]->Codegen(env));
        }

        funcDef->block->Codegen(env);
    }

private:
    Env* env;
};