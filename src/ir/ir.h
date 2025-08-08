#pragma once

#include <string>
#include <vector>

enum class VAR_TYPE {
    CONST,
    VAR,
    FUNC,
};

template<typename T, typename V, typename B, typename F>
class Env {
public:
    virtual ~Env() = default;

    virtual void Pass() = 0;
    virtual void Print() = 0;
    virtual void Dump(const char* output) = 0;

    virtual void EnterScope() = 0;
    virtual void ExitScope() = 0;

    virtual void EnterWhile(B* entry, B* end) = 0;
    virtual void ExitWhile() = 0;
    virtual B* GetWhileEntry() = 0;
    virtual B* GetWhileEnd() = 0;

    virtual T* CreateFuncType(T* retType, std::vector<T*> params) = 0;
    virtual F* CreateFunction(T* funcType, const std::string& name, std::vector<std::string> names) = 0;
    virtual B* CreateBasicBlock(const std::string& name, F* func) = 0;

    virtual void CreateCondBr(V* cond, B* trueBB, B* falseBB) = 0;
    virtual void CreateBr(B* desc) = 0;

    virtual void CreateStore(V* value, V* dest) = 0;
    virtual V* CreateLoad(V* src) = 0;
    virtual void CreateRet(V* value) = 0;
    virtual V* CreateCall(void* func, std::vector<V*> args) = 0;

    virtual V* CreateAnd(V* lhs, V* rhs) = 0;
    virtual V* CreateOr(V* lhs, V* rhs) = 0;

    virtual V* CreateAdd(V* lhs, V* rhs) = 0;
    virtual V* CreateSub(V* lhs, V* rhs) = 0;
    virtual V* CreateMul(V* lhs, V* rhs) = 0;
    virtual V* CreateDiv(V* lhs, V* rhs) = 0;
    virtual V* CreateMod(V* lhs, V* rhs) = 0;

    virtual V* CreateAlloca(T* type, const std::string& name) = 0;

    virtual V* CreateICmpNE(V* lhs, V* rhs) = 0;
    virtual V* CreateICmpEQ(V* lhs, V* rhs) = 0;

    virtual V* CreateICmpLT(V* lhs, V* rhs) = 0;
    virtual V* CreateICmpGT(V* lhs, V* rhs) = 0;
    virtual V* CreateICmpLE(V* lhs, V* rhs) = 0;
    virtual V* CreateICmpGE(V* lhs, V* rhs) = 0;
    
    virtual void SetInserPointer(B* ptr) = 0;

    virtual F* GetFunction() = 0;
    virtual V* GetFunctionArg(int index) = 0;

    virtual T* GetInt32Type() = 0;
    virtual T* GetVoidType() = 0;

    virtual V* GetInt32(int value) = 0;

    virtual bool EndWithTerminator() = 0;

    virtual void AddSymbol(const std::string& name, VAR_TYPE type, void* value) = 0;
    virtual void* GetSymbolValue(const std::string& name) = 0;
    virtual VAR_TYPE GetSymbolType(void* value) = 0;
};
