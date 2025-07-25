#pragma once

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/Function.h"

using std::unique_ptr;
using std::string;

class LLVMParams;

class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual string ToString() const = 0;
    virtual llvm::Value* Codegen(LLVMParams* params) = 0;
};

class CompUnitAST : public BaseAST {
public:
    void AddFuncDef(unique_ptr<BaseAST>&& funcDef);
    string ToString() const override;
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    unique_ptr<BaseAST> funcDef;
};

class FuncDefAST : public BaseAST {
public:
    FuncDefAST(unique_ptr<BaseAST>&& funcType, string ident, unique_ptr<BaseAST>&& block)
    : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}
    string ToString() const override;
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    unique_ptr<BaseAST> funcType;
    string ident;
    unique_ptr<BaseAST> block;
};

class FuncTypeAST : public BaseAST {
public:
    FuncTypeAST(string type) : type(std::move(type)) {}
    string ToString() const override;
    llvm::Value* Codegen(LLVMParams* params) override {};
protected:
    string type;
};

class BlockAST : public BaseAST {
public:
    BlockAST(unique_ptr<BaseAST>&& stmts) : stmts(std::move(stmts)) {}
    string ToString() const override;
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    unique_ptr<BaseAST> stmts;
};

class StmtAST : public BaseAST {
public:
    StmtAST(unique_ptr<BaseAST>&& num) : num(std::move(num)) {};
    string ToString() const override;
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    unique_ptr<BaseAST> num;
};

class NumAST : public BaseAST {
public:
    NumAST(int value) : value(value) {}
    string ToString() const override;
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    int value;
};