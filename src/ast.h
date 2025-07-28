#pragma once

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/Function.h"

#include "type.h"

using std::unique_ptr;
using std::vector;
using std::string;

class LLVMParams;

struct BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual string ToString() const = 0;
    virtual llvm::Value* Codegen(LLVMParams* params) = 0;
};

class DefineAST {
public:
    DefineAST(string ident) : ident(std::move(ident)) {}
    DefineAST(string ident, unique_ptr<BaseAST>&& initVal)
        : ident(std::move(ident)), initVal(std::move(initVal)) {}

    string ident;
    unique_ptr<BaseAST> initVal;
};

struct CompUnitAST : public BaseAST {
public:
    void AddFuncDef(unique_ptr<BaseAST>&& funcDef);
    string ToString() const override { return "CompUnitAST { " + funcDef->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    unique_ptr<BaseAST> funcDef;
};

class FuncDefAST : public BaseAST {
public:
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BaseAST>&& block)
        : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}
    string ToString() const override {
        return "FuncDefAST { " + funcType->ToString() + ", " + ident + ", " + block->ToString() + " }";
    }
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    unique_ptr<BaseType> funcType;
    string ident;
    unique_ptr<BaseAST> block;
};

class BlockAST : public BaseAST {
public:
    BlockAST(vector<unique_ptr<BaseAST>>&& blocks) : blocks(std::move(blocks)) {}
    string ToString() const override {
        return "BlockAST {  }";
    }
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    vector<unique_ptr<BaseAST>> blocks;
};

class StmtAST : public BaseAST {
public:
    enum class Type {
        Decl,
        Expr,
        Block,
        Ret,
    };
    StmtAST(Type type) : type(type) {};
    StmtAST(Type type, unique_ptr<BaseAST>&& expr) : type(type), expr1(std::move(expr)) {};
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1, unique_ptr<BaseAST>&& expr2)
        : type(type), expr1(std::move(expr1)), expr2(std::move(expr2)) {}
    string ToString() const override { return "StmtAST { " + expr1->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    Type type;
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class ExprAST : public BaseAST {
public:
    ExprAST(unique_ptr<BaseAST>&& expr) : unaryExpr(std::move(expr)) {};
    string ToString() const override { return "ExprAST { " + unaryExpr->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> unaryExpr;
};

class PrimaryExprAST : public BaseAST {
public:
    enum class Type {
        Expr,
        LVal,
        Number
    };

    PrimaryExprAST(Type type, unique_ptr<BaseAST>&& ast) {
        this->type = type;
        this->ast = std::move(ast);
    }
    string ToString() const override { return "PrimaryExprAST {" + ast->ToString() + "}"; }
    llvm::Value* Codegen(LLVMParams* params) override;

private:
    Type type;
    unique_ptr<BaseAST> ast;
};

class NumberAST : public BaseAST {
public:
    NumberAST(int value) : value(value) {}
    string ToString() const override { return "NumberAST {" + std::to_string(value); + "}"; }
    llvm::Value* Codegen(LLVMParams* params) override;
protected:
    int value;
};

class UnaryExprAST : public BaseAST {
public:
    UnaryExprAST(unique_ptr<BaseAST>&& expr) {
        this->primaryExpr = std::move(expr);
    }
    UnaryExprAST(string op, unique_ptr<BaseAST>&& expr) {
        this->op = std::move(op);
        this->unaryExpr = std::move(expr);
    }
    string ToString() const override {
        auto content = primaryExpr ? primaryExpr->ToString() : (op + ", " + unaryExpr->ToString());
        return "UnaryExprAST { " + content + " }";
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> primaryExpr;
    string op;
    unique_ptr<BaseAST> unaryExpr;
};

class MulExprAST : public BaseAST {
public:
    MulExprAST(unique_ptr<BaseAST>&& expr) : unaryExpr(std::move(expr)) {}
    MulExprAST(unique_ptr<BaseAST>&& left, string op, unique_ptr<BaseAST>&& right)
        : unaryExpr(std::move(right)), mulExpr(std::move(left)), op(std::move(op)) {}
    string ToString() const override {
        if (mulExpr) {
            return "MulExprAST { " + mulExpr->ToString() + ", " + op + ", " + unaryExpr->ToString() + " }";
        } else {
            return "MulExprAST { " + unaryExpr->ToString() + " }";
        }
    }
    llvm::Value* Codegen(LLVMParams* params) override;

private:
    unique_ptr<BaseAST> unaryExpr;
    unique_ptr<BaseAST> mulExpr;
    string op;
};

class AddExprAST : public BaseAST {
public:
    AddExprAST(unique_ptr<BaseAST>&& expr) : mulExpr(std::move(expr)) {}
    AddExprAST(unique_ptr<BaseAST>&& left, string op, unique_ptr<BaseAST>&& right)
        : mulExpr(std::move(right)), addExpr(std::move(left)), op(std::move(op))  {}
    string ToString() const override {
        if (addExpr) {
            return "AddExprAST { " + addExpr->ToString() + ", " + op + ", " + mulExpr->ToString() + " }";
        } else {
            return "AddExprAST { " + mulExpr->ToString() + " }";
        }
    }
    llvm::Value* Codegen(LLVMParams* params) override; 
private:
    unique_ptr<BaseAST> mulExpr;
    unique_ptr<BaseAST> addExpr;
    string op;
};

class RelExprAST : public BaseAST {
public:
    enum class Op {
        LT,
        GT,
        LE,
        GE,
    };
    RelExprAST(unique_ptr<BaseAST>&& expr) : expr1(std::move(expr)) {}
    RelExprAST(unique_ptr<BaseAST>&& left, Op op, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)), op(std::move(op)) {}
    string ToString() const override {
        if (expr2) {
            return "RelExprAST { " + expr1->ToString() + ", " + std::to_string(static_cast<int>(op)) + ", " + expr2->ToString() + " }";
        } else {
            return "RelExprAST { " + expr1->ToString() + " }";
        }
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
    Op op;
};

class EqExprAST : public BaseAST {
public:
    enum class Op {
        EQ,
        NE,
    };
    EqExprAST(unique_ptr<BaseAST>&& expr) : expr1(std::move(expr)) {}
    EqExprAST(unique_ptr<BaseAST>&& left, Op op, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)), op(std::move(op)) {}
    string ToString() const override {
        if (expr2) {
            return "EqExprAST { " + expr1->ToString() + ", " + std::to_string(static_cast<int>(op)) + ", " + expr2->ToString() + " }";
        } else {
            return "EqExprAST { " + expr1->ToString() + " }";
        }
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
    Op op;
};

class LAndExprAST : public BaseAST {
public:
    LAndExprAST(unique_ptr<BaseAST>&& expr) : expr1(std::move(expr)) {}
    LAndExprAST(unique_ptr<BaseAST>&& left, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)) {}
    string ToString() const override {
        if (expr2) {
            return "LAndExprAST { " + expr1->ToString() + ", &&, " + expr2->ToString() + " }";
        } else {
            return "LAndExprAST { " + expr1->ToString() + " }";
        }
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class LOrExprAST : public BaseAST {
public:
    LOrExprAST(unique_ptr<BaseAST>&& expr) : expr1(std::move(expr)) {}
    LOrExprAST(unique_ptr<BaseAST>&& left, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)) {}
    string ToString() const override {
        if (expr2) {
            return "LOrExprAST { " + expr1->ToString() + ", ||, " + expr2->ToString() + " }";
        } else {
            return "LOrExprAST { " + expr1->ToString() + " }";
        }
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class DeclAST : public BaseAST {
public:
    DeclAST(unique_ptr<BaseAST>&& constDecl) : constDecl(std::move(constDecl)) {}
    string ToString() const override { return "DeclAST { " + constDecl->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> constDecl;
};

class ConstDeclAST : public BaseAST {
public:
    ConstDeclAST(unique_ptr<BaseType>&& btype, unique_ptr<DefineAST>&& constDef)
        : btype(std::move(btype)), constDef(std::move(constDef)) {}
    string ToString() const override {
        return "ConstDeclAST { " + btype->ToString() + " }";
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseType> btype;
    unique_ptr<DefineAST> constDef;
};

class VarDeclAST : public BaseAST {
public:
    VarDeclAST(unique_ptr<BaseType>&& btype, unique_ptr<DefineAST>&& localDef)
        : btype(std::move(btype)), localDef(std::move(localDef)) {}
    string ToString() const override {
        return "VarDeclAST { " + btype->ToString() + ", " + localDef->ident + " }";
    };
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseType> btype;
    unique_ptr<DefineAST> localDef;
};

class ConstInitValAST : public BaseAST {
public:
    ConstInitValAST(unique_ptr<BaseAST>&& expr) : expr(std::move(expr)) {}
    string ToString() const override { return "ConstInitValAST { " + expr->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:    
    unique_ptr<BaseAST> expr;
};

class InitValAST : public BaseAST {
public:
    InitValAST(unique_ptr<BaseAST>&& expr) : expr(std::move(expr)) {}
    string ToString() const override { return "InitValAST { " + expr->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr;
};

class BlockItemAST : public BaseAST {
public:
    BlockItemAST(unique_ptr<BaseAST>&& ast) : ast(std::move(ast)) {}
    string ToString() const override {
        return "BlockItemAST { " + ast->ToString() + " }";
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> ast;
};

class LValAST : public BaseAST {
public:
    LValAST(string ident) : ident(std::move(ident)) {}
    string ToString() const override { return "LValAST { " + ident + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    string ident;
};

class ConstExprAST : public BaseAST {
public:
    ConstExprAST(unique_ptr<BaseAST>&& expr) : expr(std::move(expr)) {}
    string ToString() const override { return "ConstExprAST { " + expr->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr;
};