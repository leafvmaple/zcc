#pragma once

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/Function.h"

#include "type.h"

using std::unique_ptr;
using std::vector;
using std::string;

class Env;
class FuncDefAST;
class FuncFParamAST;

struct BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual void* Codegen(Env* params) = 0;
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

    void Codegen(Env* params);
private:
    vector<unique_ptr<FuncDefAST>> funcDef;
};

class FuncDefAST {
public:
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BaseAST>&& block)
        : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}

    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, vector<unique_ptr<FuncFParamAST>>&& params, unique_ptr<BaseAST>&& block)
        : funcType(std::move(funcType)), ident(std::move(ident)), params(std::move(params)), block(std::move(block)) {}
        
    void Codegen(Env* params);
private:
    unique_ptr<BaseType> funcType;
    string ident;
    vector<unique_ptr<FuncFParamAST>> params;
    unique_ptr<BaseAST> block;
};

class BlockAST : public BaseAST {
public:
    BlockAST(vector<unique_ptr<BaseAST>>&& blocks)
        : items(std::move(blocks)) {}

    void* Codegen(Env* params) override;
private:
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
        While,
        Break,
        Continue,
    };
    StmtAST(Type type)
        : type(type) {};
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1)
        : type(type), expr1(std::move(expr1)) {};
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1, unique_ptr<BaseAST>&& expr2)
        : type(type), expr1(std::move(expr1)), expr2(std::move(expr2)) {}
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1, unique_ptr<BaseAST>&& expr2, unique_ptr<BaseAST>&& expr3)
        : type(type), expr1(std::move(expr1)), expr2(std::move(expr2)), expr3(std::move(expr3)) {}
 
    void* Codegen(Env* params) override;
private:
    Type type;
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
    unique_ptr<BaseAST> expr3;
};

class ExprAST : public BaseAST {
public:
    ExprAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {};

    void* Codegen(Env* params) override;
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

    void* Codegen(Env* params) override;
private:
    Type type;
    unique_ptr<BaseAST> ast;
};

class NumberAST : public BaseAST {
public:
    NumberAST(int value)
        : value(value) {}

    void* Codegen(Env* params) override;
private:
    int value;
};

class UnaryExprAST : public BaseAST {
public:
    enum class Type {
        Primary,
        Unary,
        Call
    };
    UnaryExprAST(Type type, unique_ptr<BaseAST>&& expr)
        : type(type), expr(std::move(expr)) {}
        UnaryExprAST(Type type, string ident)
        : type(type), ident(std::move(ident)) {}
    UnaryExprAST(Type type, string ident, vector<unique_ptr<BaseAST>>&& args)
        : type(type), ident(std::move(ident)), args(std::move(args)) {}
    UnaryExprAST(Type type, string op, unique_ptr<BaseAST>&& expr)
        : type(type), op(std::move(op)), expr(std::move(expr)) {}

    void* Codegen(Env* params) override;
private:
    Type type;
    string op;
    string ident;
    unique_ptr<BaseAST> expr;
    vector<unique_ptr<BaseAST>> args;
};

class MulExprAST : public BaseAST {
public:
    MulExprAST(unique_ptr<BaseAST>&& expr)
        : expr1(std::move(expr)) {}
    MulExprAST(unique_ptr<BaseAST>&& left, string op, unique_ptr<BaseAST>&& right)
        : op(std::move(op)), expr1(std::move(left)), expr2(std::move(right)) {}

    void* Codegen(Env* params) override;
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

    void* Codegen(Env* params) override;
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

    void* Codegen(Env* params) override;
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

    void* Codegen(Env* params) override;
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
    
    void* Codegen(Env* params) override;
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

    void* Codegen(Env* params) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class DeclAST : public BaseAST {
public:
    DeclAST(unique_ptr<BaseAST>&& constDecl)
        : constDecl(std::move(constDecl)) {}

    void* Codegen(Env* params) override;
private:
    unique_ptr<BaseAST> constDecl;
};

class ConstDeclAST : public BaseAST {
public:
    ConstDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<DefineAST>>&& constDef)
        : btype(std::move(btype)), constDef(std::move(constDef)) {}

    void* Codegen(Env* params) override;
private:
    unique_ptr<BaseType> btype;
    vector<unique_ptr<DefineAST>> constDef;
};

class VarDeclAST : public BaseAST {
public:
    VarDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<DefineAST>>&& localDef)
        : btype(std::move(btype)), localDef(std::move(localDef)) {}

    void* Codegen(Env* params) override;
private:
    unique_ptr<BaseType> btype;
    vector<unique_ptr<DefineAST>> localDef;
};

class ConstInitValAST : public BaseAST {
public:
    ConstInitValAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {}

    void* Codegen(Env* params) override;
private:
    unique_ptr<BaseAST> expr;
};

class InitValAST : public BaseAST {
public:
    InitValAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {}

    void* Codegen(Env* params) override;
private:
    unique_ptr<BaseAST> expr;
};

class BlockItemAST : public BaseAST {
public:
    BlockItemAST(unique_ptr<BaseAST>&& ast)
        : ast(std::move(ast)) {}

    void* Codegen(Env* params) override;
private:
    unique_ptr<BaseAST> ast;
};

class LValAST : public BaseAST {
public:
    LValAST(string ident)
        : ident(std::move(ident)) {}

    void* Codegen(Env* params) override;
private:
    string ident;
};

class ConstExprAST : public BaseAST {
public:
    ConstExprAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {}

    void* Codegen(Env* params) override;
private:
    unique_ptr<BaseAST> expr;
};

class FuncFParamAST : public BaseAST {
public:
    FuncFParamAST(unique_ptr<BaseType>&& btype, string ident)
        : btype(std::move(btype)), ident(std::move(ident)) {}

    void* Codegen(Env* params) override;

    unique_ptr<BaseType> btype;
    string ident;
};

class FuncRParamAST : public BaseAST {
public:
    FuncRParamAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {}

    void* Codegen(Env* params) override;
private:
    unique_ptr<BaseAST> expr;
};