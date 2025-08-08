#pragma once

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include <map>

#include "ir.h"


class LLVMEnv : public Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function> {
public:
    LLVMEnv(std::string moduleName);

    void Pass() override;
    
    void EnterScope() override;
    void ExitScope() override;

    void EnterWhile(llvm::BasicBlock* entry, llvm::BasicBlock* end) override {
        whiles.push_back({(llvm::BasicBlock*)entry, (llvm::BasicBlock*)end});
    }
    void ExitWhile() override {
        whiles.pop_back();
    }
    llvm::BasicBlock* GetWhileEntry() override {
        return whiles.back().entry;
    }
    llvm::BasicBlock* GetWhileEnd() override { 
        return whiles.back().end;
    }

    void Print() override;
    void Dump(const char* output) override;

    llvm::Type* CreateFuncType(llvm::Type* retType, std::vector<llvm::Type*> params) override;
    llvm::Function* CreateFunction(llvm::Type* funcType, const std::string& name, std::vector<std::string> names) override;
    llvm::BasicBlock* CreateBasicBlock(const std::string& name, llvm::Function* func) override;

    void CreateCondBr(llvm::Value* cond, llvm::BasicBlock* thenBB, llvm::BasicBlock* elseBB) override;
    void CreateBr(llvm::BasicBlock* desc) override;

    void CreateStore(llvm::Value* value, llvm::Value* dest) override;
    llvm::Value* CreateLoad(llvm::Value* src) override;
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

    void SetInserPointer(llvm::BasicBlock* ptr) override;

    llvm::Function* GetFunction() override;
    llvm::Value* GetFunctionArg(int index) override;

    llvm::Type* GetInt32Type() override;
    llvm::Type* GetVoidType() override;

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