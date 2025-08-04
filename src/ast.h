#pragma once

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/Function.h"

#include "type.h"
#include "koopa_ir.h"

using std::unique_ptr;
using std::vector;
using std::string;

class LLVMParams;
class FuncDefAST;

struct BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual string ToString() const { return ""; };
    virtual llvm::Value* Codegen(LLVMParams* params) = 0;
    virtual koopa_raw_value_t ToKoopa(KoopaEnv* env) {
        // Default implementation does nothing
        return nullptr;
    }
};

class DefineAST {
public:
    DefineAST(string ident)
        : ident(std::move(ident)) {}
    DefineAST(string ident, unique_ptr<BaseAST>&& initVal)
        : ident(std::move(ident)), initVal(std::move(initVal)) {}

    string ident;
    unique_ptr<BaseAST> initVal;
};

struct CompUnitAST {
public:
    void AddFuncDef(unique_ptr<FuncDefAST>&& funcDef);

    llvm::Value* Codegen(LLVMParams* params);
    koopa_raw_program_t ToKoopa(KoopaEnv* env);
protected:
    vector<unique_ptr<FuncDefAST>> funcDef;
};

class FuncDefAST {
public:
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BaseAST>&& block)
        : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}
        
    llvm::Value* Codegen(LLVMParams* params);
    koopa_raw_function_t ToKoopa(KoopaEnv* env);
protected:
    unique_ptr<BaseType> funcType;
    string ident;
    unique_ptr<BaseAST> block;
};

class BlockAST : public BaseAST {
public:
    BlockAST(vector<unique_ptr<BaseAST>>&& blocks)
        : items(std::move(blocks)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
protected:
    vector<unique_ptr<BaseAST>> items;
};

class StmtAST : public BaseAST {
public:
    enum class Type {
        Assign,
        Expr,
        Block,
        If,
        Ret,
    };
    StmtAST(Type type)
        : type(type) {};
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1)
        : type(type), expr1(std::move(expr1)) {};
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1, unique_ptr<BaseAST>&& expr2)
        : type(type), expr1(std::move(expr1)), expr2(std::move(expr2)) {}
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1, unique_ptr<BaseAST>&& expr2, unique_ptr<BaseAST>&& expr3)
        : type(type), expr1(std::move(expr1)), expr2(std::move(expr2)), expr3(std::move(expr3)) {}
 
    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
protected:
    Type type;
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
    unique_ptr<BaseAST> expr3;
};

class ExprAST : public BaseAST {
public:
    ExprAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {};

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseAST> expr;
};

class PrimaryExprAST : public BaseAST {
public:
    enum class Type {
        Expr,
        LVal,
        Number
    };

    PrimaryExprAST(Type type, unique_ptr<BaseAST>&& ast)
        : type(type), ast(std::move(ast)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    Type type;
    unique_ptr<BaseAST> ast;
};

class NumberAST : public BaseAST {
public:
    NumberAST(int value)
        : value(value) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
protected:
    int value;
};

class UnaryExprAST : public BaseAST {
public:
    enum class Type {
        Primary,
        Unary
    };
    UnaryExprAST(Type type, unique_ptr<BaseAST>&& expr)
        : type(type), expr(std::move(expr)) {}
    UnaryExprAST(Type type, string op, unique_ptr<BaseAST>&& expr)
        : type(type), op(std::move(op)), expr(std::move(expr)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    Type type;
    string op;
    unique_ptr<BaseAST> expr;
};

class MulExprAST : public BaseAST {
public:
    MulExprAST(unique_ptr<BaseAST>&& expr)
        : expr1(std::move(expr)) {}
    MulExprAST(unique_ptr<BaseAST>&& left, string op, unique_ptr<BaseAST>&& right)
        : op(std::move(op)), expr1(std::move(left)), expr2(std::move(right)) {}
    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    string op;
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class AddExprAST : public BaseAST {
public:
    AddExprAST(unique_ptr<BaseAST>&& expr)
        : expr1(std::move(expr)) {}
    AddExprAST(unique_ptr<BaseAST>&& left, string op, unique_ptr<BaseAST>&& right)
        : op(std::move(op)), expr1(std::move(left)), expr2(std::move(right)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    string op;
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class RelExprAST : public BaseAST {
public:
    enum class Op {
        LT,
        GT,
        LE,
        GE,
    };
    RelExprAST(unique_ptr<BaseAST>&& expr)
        : expr1(std::move(expr)) {}
    RelExprAST(unique_ptr<BaseAST>&& left, Op op, unique_ptr<BaseAST>&& right)
        : op(std::move(op)), expr1(std::move(left)), expr2(std::move(right)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    Op op;
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class EqExprAST : public BaseAST {
public:
    enum class Op {
        EQ,
        NE,
    };
    EqExprAST(unique_ptr<BaseAST>&& expr)
        : expr1(std::move(expr)) {}
    EqExprAST(unique_ptr<BaseAST>&& left, Op op, unique_ptr<BaseAST>&& right)
        : op(std::move(op)), expr1(std::move(left)), expr2(std::move(right)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    Op op;
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class LAndExprAST : public BaseAST {
public:
    LAndExprAST(unique_ptr<BaseAST>&& expr) : expr1(std::move(expr)) {}
    LAndExprAST(unique_ptr<BaseAST>&& left, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)) {}
    
    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class LOrExprAST : public BaseAST {
public:
    LOrExprAST(unique_ptr<BaseAST>&& expr)
        : expr1(std::move(expr)) {}
    LOrExprAST(unique_ptr<BaseAST>&& left, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class DeclAST : public BaseAST {
public:
    DeclAST(unique_ptr<BaseAST>&& constDecl)
        : constDecl(std::move(constDecl)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseAST> constDecl;
};

class ConstDeclAST : public BaseAST {
public:
    ConstDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<DefineAST>>&& constDef)
        : btype(std::move(btype)), constDef(std::move(constDef)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseType> btype;
    vector<unique_ptr<DefineAST>> constDef;
};

class VarDeclAST : public BaseAST {
public:
    VarDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<DefineAST>>&& localDef)
        : btype(std::move(btype)), localDef(std::move(localDef)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseType> btype;
    vector<unique_ptr<DefineAST>> localDef;
};

class ConstInitValAST : public BaseAST {
public:
    ConstInitValAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseAST> expr;
};

class InitValAST : public BaseAST {
public:
    InitValAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseAST> expr;
};

class BlockItemAST : public BaseAST {
public:
    BlockItemAST(unique_ptr<BaseAST>&& ast)
        : ast(std::move(ast)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseAST> ast;
};

class LValAST : public BaseAST {
public:
    LValAST(string ident)
        : ident(std::move(ident)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    string ident;
};

class ConstExprAST : public BaseAST {
public:
    ConstExprAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    koopa_raw_value_t ToKoopa(KoopaEnv* env) override;
private:
    unique_ptr<BaseAST> expr;
};