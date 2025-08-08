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
    void CreateRet(llvm::Value* value) override;
    llvm::Value* CreateCall(llvm::Function* func, std::vector<llvm::Value*> args) override;
    
    llvm::Value* CreateAlloca(llvm::Type* type, const std::string& name) override;

    llvm::Value* CreateAnd(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateOr(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateAdd(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateSub(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateMul(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateDiv(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateMod(llvm::Value* lhs, llvm::Value* rhs) override;

    llvm::Value* CreateICmpNE(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateICmpEQ(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateICmpLT(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateICmpGT(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateICmpLE(llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* CreateICmpGE(llvm::Value* lhs, llvm::Value* rhs) override;

    void SetInserPointer(llvm::BasicBlock* ptr) override;

    llvm::Function* GetFunction() override;
    llvm::Value* GetFunctionArg(int index) override;

    llvm::Type* GetInt32Type() override;
    llvm::Type* GetVoidType() override;

    llvm::Value* GetInt32(int value) override;

    bool EndWithTerminator() override;

private:
    struct while_data_t {
        llvm::BasicBlock* entry;
        llvm::BasicBlock* end;
    };

    llvm::LLVMContext TheContext;
    llvm::Module TheModule;
    llvm::IRBuilder<llvm::NoFolder, llvm::IRBuilderDefaultInserter> Builder;

    std::vector<while_data_t> whiles;
};