#pragma once

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include <string>
#include <vector>
#include <map>
#include <functional>

enum class VAR_TYPE { CONST, VAR, GLOBAL, FUNC };

class CodeGen {
public:
    union SymbolValue {
        llvm::Value* value;
        llvm::Function* function;
        bool operator<(const SymbolValue& other) const { return value < other.value; }
    };

    CodeGen(const std::string& moduleName);

    void Optimize();
    void Print();
    void Dump(const char* output);

    // Types
    llvm::FunctionType* CreateFuncType(llvm::Type* retType, std::vector<llvm::Type*> params);
    llvm::Type* GetInt32Type();
    llvm::Type* GetInt8Type();
    llvm::Type* GetVoidType();
    llvm::Type* GetArrayType(llvm::Type* type, int size);
    llvm::Type* GetPointerType(llvm::Type* type);
    llvm::Type* GetValueType(llvm::Value* value);
    llvm::Type* GetElementType(llvm::Type* type);
    llvm::Type* GetAllocatedType(llvm::Value* value);
    bool IsArrayType(llvm::Type* type);
    bool IsPointerType(llvm::Type* type);

    // Functions
    llvm::BasicBlock* CreateBasicBlock(const std::string& name, llvm::Function* func);
    llvm::Function* CreateFunction(llvm::FunctionType* funcType, const std::string& name, std::vector<std::string> names);
    void CreateBuiltin(const std::string& name, llvm::Type* retType, std::vector<llvm::Type*> params);
    llvm::Function* GetFunction();
    llvm::Value* GetFunctionArg(int index);

    // Memory
    llvm::Value* CreateAlloca(llvm::Type* type, const std::string& name);
    llvm::Value* CreateGlobal(llvm::Type* type, const std::string& name, llvm::Value* init);
    void CreateStore(llvm::Value* value, llvm::Value* dest);
    llvm::Value* CreateLoad(llvm::Value* src);
    llvm::Value* CreateGEP(llvm::Type* type, llvm::Value* array, std::vector<llvm::Value*> index);

    // Constants
    llvm::Value* GetInt32(int value);
    llvm::Value* GetInt8(int value);
    llvm::Value* CreateZero(llvm::Type* type);
    llvm::Value* CreateArray(llvm::Type* type, std::vector<llvm::Value*> values);
    llvm::Value* CreateGlobalString(const std::string& str);

    // Arithmetic
    llvm::Value* CreateAdd(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateSub(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateMul(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateDiv(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateMod(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateAnd(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateOr(llvm::Value* lhs, llvm::Value* rhs);

    // Comparisons
    llvm::Value* CreateICmpNE(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateICmpEQ(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateICmpLT(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateICmpGT(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateICmpLE(llvm::Value* lhs, llvm::Value* rhs);
    llvm::Value* CreateICmpGE(llvm::Value* lhs, llvm::Value* rhs);

    // Control flow
    void CreateCondBr(llvm::Value* cond, llvm::BasicBlock* trueBB, llvm::BasicBlock* falseBB);
    void CreateBr(llvm::BasicBlock* dest);
    void CreateRet(llvm::Value* value);
    llvm::Value* CreateCall(llvm::Function* func, std::vector<llvm::Value*> args);
    void SetInsertPoint(llvm::BasicBlock* bb);
    bool EndWithTerminator();

    // Type conversions
    llvm::Value* CreateTrunc(llvm::Value* value, llvm::Type* type);
    llvm::Value* CreateZExt(llvm::Value* value, llvm::Type* type);

    // Value utilities
    llvm::Value* CalculateBinaryOp(const std::function<int(int, int)>& func, llvm::Value* lhs, llvm::Value* rhs);
    int GetValueInt(llvm::Value* value);
    llvm::Value* GetArrayElement(llvm::Value* array, int index);
    llvm::Value* GetBaseValue(llvm::Value* value);

    // Scope management
    void EnterScope();
    void ExitScope();
    bool IsGlobalScope() const;
    void AddSymbol(const std::string& name, VAR_TYPE type, SymbolValue value);
    SymbolValue GetSymbolValue(const std::string& name);
    VAR_TYPE GetSymbolType(SymbolValue value);

    // While loop tracking
    void EnterWhile(llvm::BasicBlock* entry, llvm::BasicBlock* end);
    void ExitWhile();
    llvm::BasicBlock* GetWhileEntry();
    llvm::BasicBlock* GetWhileEnd();

private:
    llvm::LLVMContext Context;
    llvm::Module Module;
    llvm::IRBuilder<llvm::NoFolder> Builder;

    struct WhileData { llvm::BasicBlock* entry; llvm::BasicBlock* end; };
    std::vector<std::map<std::string, SymbolValue>> locals;
    std::vector<std::map<SymbolValue, VAR_TYPE>> types;
    std::vector<WhileData> whiles;
};
