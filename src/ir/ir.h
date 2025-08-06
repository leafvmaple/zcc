#pragma once

#include <string>
#include <vector>

enum class VAR_TYPE {
    CONST,
    VAR,
};

class Env {
public:
    virtual ~Env() = default;

    virtual void Print() = 0;
    virtual void Dump(const char* output) = 0;

    virtual void EnterScope() = 0;
    virtual void ExitScope() = 0;

    virtual void EnterWhile(void* entry, void* end) = 0;
    virtual void ExitWhile() = 0;
    virtual void* GetWhileEntry() = 0;
    virtual void* GetWhileEnd() = 0;

    virtual void* CreateFuncType(void* retType) = 0;
    virtual void* CreateFunction(void* funcType, const std::string& name) = 0;
    virtual void* CreateBasicBlock(const std::string& name, void* func) = 0;

    virtual void CreateCondBr(void* cond, void* trueBB, void* falseBB) = 0;
    virtual void CreateBr(void* desc) = 0;

    virtual void CreateStore(void* value, void* dest) = 0;
    virtual void* CreateLoad(void* src) = 0;
    virtual void CreateRet(void* value) = 0;

    virtual void* CreateAnd(void* lhs, void* rhs) = 0;
    virtual void* CreateOr(void* lhs, void* rhs) = 0;

    virtual void* CreateAdd(void* lhs, void* rhs) = 0;
    virtual void* CreateSub(void* lhs, void* rhs) = 0;
    virtual void* CreateMul(void* lhs, void* rhs) = 0;
    virtual void* CreateDiv(void* lhs, void* rhs) = 0;
    virtual void* CreateMod(void* lhs, void* rhs) = 0;

    virtual void* CreateAlloca(void* type, const std::string& name) = 0;

    virtual void* CreateICmpNE(void* lhs, void* rhs) = 0;
    virtual void* CreateICmpEQ(void* lhs, void* rhs) = 0;

    virtual void* CreateICmpLT(void* lhs, void* rhs) = 0;
    virtual void* CreateICmpGT(void* lhs, void* rhs) = 0;
    virtual void* CreateICmpLE(void* lhs, void* rhs) = 0;
    virtual void* CreateICmpGE(void* lhs, void* rhs) = 0;

    virtual void SetInserPointer(void* ptr) = 0;

    virtual void* GetFunction() = 0;
    virtual void* GetInt32Type() = 0;
    virtual void* GetInt32(int value) = 0;

    virtual bool EndWithTerminator() = 0;

    virtual void AddSymbol(const std::string& name, VAR_TYPE type, void* value) = 0;
    virtual void* GetSymbolValue(const std::string& name) = 0;
    virtual VAR_TYPE GetSymbolType(void* value) = 0;
};