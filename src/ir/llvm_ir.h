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

    void Optimize() override;
    void Print() override;
    void Dump(const char* output) override;

    llvm::Type* CreateFuncType(llvm::Type* retType, std::vector<llvm::Type*> params) override;
    llvm::BasicBlock* CreateBasicBlock(const std::string& name, llvm::Function* func) override;
    llvm::Function* CreateFunction(llvm::Type* funcType, const std::string& name, std::vector<std::string> names) override;
    llvm::Value* CreateArray(llvm::Type* type, std::vector<llvm::Value*> values) override;
    void CreateBuiltin(const std::string& name, llvm::Type* retType, std::vector<llvm::Type*> params) override;

    void CreateCondBr(llvm::Value* cond, llvm::BasicBlock* thenBB, llvm::BasicBlock* elseBB) override;
    void CreateBr(llvm::BasicBlock* desc) override;

    void CreateStore(llvm::Value* value, llvm::Value* dest) override;
    llvm::Value* CreateLoad(llvm::Value* src) override;
    void CreateRet(llvm::Value* value) override;
    llvm::Value* CreateCall(llvm::Function* func, std::vector<llvm::Value*> args) override;
    
    llvm::Value* CreateAlloca(llvm::Type* type, const std::string& name) override;
    llvm::Value* CreateGlobal(llvm::Type* type, const std::string& name, llvm::Value* init) override;
    llvm::Value* CreateZero(llvm::Type* type) override;

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
    llvm::Type* GetArrayType(llvm::Type* type, int size) override;
    llvm::Type* GetPointerType(llvm::Type* type) override;

    llvm::Value* GetInt32(int value) override;
    llvm::Value* CreateGEP(llvm::Type* type, llvm::Value* array, llvm::Value* index) override;

    llvm::Value* CaculateBinaryOp(const std::function<int(int, int)>& func, llvm::Value* lhs, llvm::Value* rhs) override;

    int GetValueInt(llvm::Value* value) override;

    bool EndWithTerminator() override;

private:

    llvm::LLVMContext TheContext;
    llvm::Module TheModule;
    llvm::IRBuilder<llvm::NoFolder, llvm::IRBuilderDefaultInserter> Builder;
};