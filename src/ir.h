#pragma once

#include <string>
#include <vector>

enum class VAR_TYPE {
    CONST,
    VAR,
};

class Env {
public:

    virtual void EnterScope() = 0;
    virtual void ExitScope() = 0;

    virtual void* CreateFuncType(void* retType) = 0;

    virtual void CreateFunction(void* funcType, const std::string& name) = 0;
    virtual void CreateBasicBlock(const std::string& name) = 0;

    virtual void CreateStore(void* value, void* dest) = 0;
    virtual void* CreateLoad(void* src) = 0;
    virtual void CreateRet(void* value) = 0;

    virtual void* CreateAnd(void* lhs, void* rhs) = 0;
    virtual void* CreateOr(void* lhs, void* rhs) = 0;

    virtual void* CreateAlloca(void* type, const std::string& name) = 0;

    virtual void* CreateICmpNE(void* lhs, void* rhs) = 0;
    virtual void* CreateICmpEQ(void* lhs, void* rhs) = 0;

    virtual void* CreateICmpLT(void* lhs, void* rhs) = 0;
    virtual void* CreateICmpGT(void* lhs, void* rhs) = 0;
    virtual void* CreateICmpLE(void* lhs, void* rhs) = 0;
    virtual void* CreateICmpGE(void* lhs, void* rhs) = 0;

    virtual void AddSymbol(const std::string& name, VAR_TYPE type, void* value) = 0;
    virtual void* GetSymbolValue(const std::string& name) = 0;
    virtual VAR_TYPE GetSymbolType(void* value) = 0;
};