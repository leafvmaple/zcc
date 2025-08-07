#pragma once

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include <map>

#include "ir.h"


class LLVMEnv : public Env<llvm::Value> {
public:
    LLVMEnv(std::string moduleName);

    void Pass() override;
    
    void EnterScope() override;
    void ExitScope() override;

    void EnterWhile(void* entry, void* end) override {
        whiles.push_back({(llvm::BasicBlock*)entry, (llvm::BasicBlock*)end});
    }
    void ExitWhile() override {
        whiles.pop_back();
    }
    void* GetWhileEntry() override {
        return (void*)whiles.back().entry;
    }
    void* GetWhileEnd() override { 
        return (void*)whiles.back().end;
    }

    void Print() override;
    void Dump(const char* output) override;

    void* CreateFuncType(void* retType, std::vector<void*> params) override;
    void* CreateFunction(void* funcType, const std::string& name, std::vector<std::string> names) override;
    void* CreateBasicBlock(const std::string& name, void* func) override;

    void CreateCondBr(void* cond, void* thenBB, void* elseBB) override;
    void CreateBr(void* desc) override;

    void CreateStore(void* value, void* dest) override;
    llvm::Value* CreateLoad(void* src) override;
    void CreateRet(void* value) override;
    void* CreateCall(void* func, std::vector<void*> args) override;
    
    void* CreateAlloca(void* type, const std::string& name) override;

    void* CreateAnd(void* lhs, void* rhs) override;
    void* CreateOr(void* lhs, void* rhs) override;
    void* CreateAdd(void* lhs, void* rhs) override;
    void* CreateSub(void* lhs, void* rhs) override;
    void* CreateMul(void* lhs, void* rhs) override;
    void* CreateDiv(void* lhs, void* rhs) override;
    void* CreateMod(void* lhs, void* rhs) override;

    void* CreateICmpNE(void* lhs, void* rhs) override;
    void* CreateICmpEQ(void* lhs, void* rhs) override;
    void* CreateICmpLT(void* lhs, void* rhs) override;
    void* CreateICmpGT(void* lhs, void* rhs) override;
    void* CreateICmpLE(void* lhs, void* rhs) override;
    void* CreateICmpGE(void* lhs, void* rhs) override;

    void SetInserPointer(void* ptr) override;

    void* GetFunction() override;
    void* GetFunctionArg(int index) override;

    void* GetInt32Type() override;
    void* GetVoidType() override;

    void* GetInt32(int value) override;

    bool EndWithTerminator() override;

    void AddSymbol(const std::string& name, VAR_TYPE type, void* value) override;
    void* GetSymbolValue(const std::string& name) override;
    VAR_TYPE GetSymbolType(void* value) override;

private:
    struct while_data_t {
        llvm::BasicBlock* entry;
        llvm::BasicBlock* end;
    };

    llvm::LLVMContext TheContext;
    llvm::Module TheModule;
    llvm::IRBuilder<llvm::NoFolder, llvm::IRBuilderDefaultInserter> Builder;

    std::vector<std::map<std::string, llvm::Value*>> locals;
    std::vector<std::map<llvm::Value*, VAR_TYPE>> types;
    std::vector<while_data_t> whiles;
};