#pragma once

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include <map>

#include "ir.h"


class LLVMEnv : public Env {
public:
    LLVMEnv(std::string moduleName);
    
    void EnterScope();
    void ExitScope();

    void* CreateFuncType(void* retType);
    void* CreateFunction(void* funcType, const std::string& name);
    void* CreateBasicBlock(const std::string& name, void* func);

    void CreateCondBr(void* cond, void* thenBB, void* elseBB);
    void CreateBr(void* desc);

    void CreateStore(void* value, void* dest);
    void* CreateLoad(void* src);

    void CreateRet(void* value);
    
    void* CreateAlloca(void* type, const std::string& name);

    void* CreateAnd(void* lhs, void* rhs);
    void* CreateOr(void* lhs, void* rhs);
    void* CreateAdd(void* lhs, void* rhs);
    void* CreateSub(void* lhs, void* rhs);
    void* CreateMul(void* lhs, void* rhs);
    void* CreateDiv(void* lhs, void* rhs);
    void* CreateMod(void* lhs, void* rhs);

    void* CreateICmpNE(void* lhs, void* rhs);
    void* CreateICmpEQ(void* lhs, void* rhs);
    void* CreateICmpLT(void* lhs, void* rhs);
    void* CreateICmpGT(void* lhs, void* rhs);
    void* CreateICmpLE(void* lhs, void* rhs);
    void* CreateICmpGE(void* lhs, void* rhs);

    void SetInserPointer(void* ptr);

    void* GetFunction();

    void* GetInt32Type();

    void* GetInt32(int value);

    bool EndWithTerminator();

    void AddSymbol(const std::string& name, VAR_TYPE type, void* value);
    void* GetSymbolValue(const std::string& name);
    VAR_TYPE GetSymbolType(void* value);

    llvm::LLVMContext TheContext;
    llvm::Module TheModule;
    llvm::IRBuilder<llvm::NoFolder, llvm::IRBuilderDefaultInserter> Builder;

private:
    std::vector<std::map<std::string, llvm::Value*>> locals;
    std::vector<std::map<llvm::Value*, VAR_TYPE>> types;
};