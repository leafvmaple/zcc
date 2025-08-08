#pragma once

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/Function.h"
#include "type.h"

using std::unique_ptr;
using std::vector;
using std::string;

class FuncDefAST;
class FuncFParamAST;
class BlockAST;
class BlockItemAST;
class StmtAST;
class ConstInitValAST;
class InitValAST;
class LValAST;
class NumberAST;
class ExprAST;
class LOrExprAST;
class ConstDeclAST;
class VarDeclAST;
class ConstExprAST;

class DefineAST {
public:
    DefineAST(string ident)
        : ident(std::move(ident)) {}
    DefineAST(string ident, unique_ptr<ConstInitValAST>&& constInitVal)
        : ident(std::move(ident)), constInitVal(std::move(constInitVal)) {}
    DefineAST(string ident, unique_ptr<InitValAST>&& initVal)
        : ident(std::move(ident)), initVal(std::move(initVal)) {}

    string ident;
    unique_ptr<ConstInitValAST> constInitVal;
    unique_ptr<InitValAST> initVal;
};

struct CompUnitAST {
public:
    void AddFuncDef(unique_ptr<FuncDefAST>&& funcDef) {
        funcDefs.emplace_back(std::move(funcDef));
    }

    vector<unique_ptr<FuncDefAST>> funcDefs;
};

class FuncDefAST {
public:
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BlockAST>&& block)
        : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, vector<unique_ptr<FuncFParamAST>>&& params, unique_ptr<BlockAST>&& block)
        : funcType(std::move(funcType)), ident(std::move(ident)), params(std::move(params)), block(std::move(block)) {}

    unique_ptr<BaseType> funcType;
    string ident;
    vector<unique_ptr<FuncFParamAST>> params;
    unique_ptr<BlockAST> block;
};

class BlockAST {
public:
    BlockAST(vector<unique_ptr<BlockItemAST>>&& items) : items(std::move(items)) {}

    vector<unique_ptr<BlockItemAST>> items;
};

class StmtAST {
public:
    enum class Type {
        Assign,
        Expr,
        Block,
        If,
        Ret,
        While,
        Break,
        Continue
    };

    StmtAST(Type type)
        : type(type) {}
    StmtAST(Type type, unique_ptr<ExprAST>&& expr)
        : type(type), expr(std::move(expr)) {}
    StmtAST(Type type, unique_ptr<BlockAST>&& block)
        : type(type), block(std::move(block)) {}
    StmtAST(Type type, unique_ptr<LValAST>&& lval, unique_ptr<ExprAST>&& expr) 
        : type(type), lval(std::move(lval)), expr(std::move(expr)) {}
    StmtAST(Type type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt) 
        : type(type), cond(std::move(cond)), thenStmt(std::move(thenStmt)) {}
    StmtAST(Type type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt, unique_ptr<StmtAST>&& elseStmt) 
        : type(type), cond(std::move(cond)), thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}

    Type type;
    unique_ptr<LValAST> lval;
    unique_ptr<ExprAST> expr;
    unique_ptr<BlockAST> block;
    unique_ptr<ExprAST> cond;
    unique_ptr<StmtAST> thenStmt;
    unique_ptr<StmtAST> elseStmt;
};

class ExprAST {
public:
    ExprAST(unique_ptr<LOrExprAST>&& lorExpr) : lorExpr(std::move(lorExpr)) {}

    unique_ptr<LOrExprAST> lorExpr;
};

class PrimaryExprAST {
public:
    enum class Type {
        Expr,
        LVal,
        Number
    };

    PrimaryExprAST(Type type, unique_ptr<ExprAST>&& expr)
        : type(type), expr(std::move(expr)) {}
    PrimaryExprAST(Type type, unique_ptr<LValAST>&& lval)
        : type(type), lval(std::move(lval)) {}
    PrimaryExprAST(Type type, unique_ptr<NumberAST>&& value)
        : type(type), value(std::move(value)) {}

    Type type;
    unique_ptr<ExprAST> expr;
    unique_ptr<LValAST> lval;
    unique_ptr<NumberAST> value;
};

class NumberAST {
public:
    NumberAST(int value) : value(value) {}

    int value;
};

class UnaryExprAST {
public:
    enum class Type {
        Primary,
        Unary,
        Call
    };

    UnaryExprAST(Type type, unique_ptr<PrimaryExprAST>&& primaryExpr) 
        : type(type), primaryExpr(std::move(primaryExpr)) {}
    UnaryExprAST(Type type, string op, unique_ptr<UnaryExprAST>&& unaryExpr) 
        : type(type), op(std::move(op)), unaryExpr(std::move(unaryExpr)) {}
    UnaryExprAST(Type type, string ident)
        : type(type), ident(std::move(ident)) {}
    UnaryExprAST(Type type, string ident, vector<unique_ptr<ExprAST>>&& callArgs) 
        : type(type), ident(std::move(ident)), callArgs(std::move(callArgs)) {}

    Type type;
    string op;
    string ident;
    unique_ptr<PrimaryExprAST> primaryExpr;
    unique_ptr<UnaryExprAST> unaryExpr;
    vector<unique_ptr<ExprAST>> callArgs;
};

class MulExprAST {
public:
    MulExprAST(unique_ptr<UnaryExprAST>&& unaryExpr)
        : unaryExpr(std::move(unaryExpr)) {}
    MulExprAST(unique_ptr<MulExprAST>&& left, string op, unique_ptr<UnaryExprAST>&& right) 
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    unique_ptr<MulExprAST> left;
    string op;
    unique_ptr<UnaryExprAST> unaryExpr;
    unique_ptr<UnaryExprAST> right;
};

class AddExprAST {
public:
    AddExprAST(unique_ptr<MulExprAST>&& mulExpr)
        : mulExpr(std::move(mulExpr)) {}
    AddExprAST(unique_ptr<AddExprAST>&& left, string op, unique_ptr<MulExprAST>&& right) 
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    unique_ptr<AddExprAST> left;
    string op;
    unique_ptr<MulExprAST> mulExpr;
    unique_ptr<MulExprAST> right;
};

class RelExprAST {
public:
    enum class Op { LT, GT, LE, GE };

    RelExprAST(unique_ptr<AddExprAST>&& addExpr)
        : addExpr(std::move(addExpr)) {}
    RelExprAST(unique_ptr<RelExprAST>&& left, Op op, unique_ptr<AddExprAST>&& right) 
        : left(std::move(left)), op(op), right(std::move(right)) {}
    
    unique_ptr<AddExprAST> addExpr;
    unique_ptr<RelExprAST> left;
    Op op;
    unique_ptr<AddExprAST> right;
};

class EqExprAST {
public:
    enum class Op { EQ, NE };

    EqExprAST(unique_ptr<RelExprAST>&& relExpr)
        : relExpr(std::move(relExpr)) {}
    EqExprAST(unique_ptr<EqExprAST>&& left, Op op, unique_ptr<RelExprAST>&& right) 
        : left(std::move(left)), op(op), right(std::move(right)) {}

    
    unique_ptr<RelExprAST> relExpr;
    unique_ptr<EqExprAST> left;
    Op op;
    unique_ptr<RelExprAST> right;
};

class LAndExprAST {
public:
    LAndExprAST(unique_ptr<EqExprAST>&& eqExpr)
        : eqExpr(std::move(eqExpr)) {}
    LAndExprAST(unique_ptr<LAndExprAST>&& left, unique_ptr<EqExprAST>&& right) 
        : left(std::move(left)), right(std::move(right)) {}

    unique_ptr<EqExprAST> eqExpr;
    unique_ptr<LAndExprAST> left;
    unique_ptr<EqExprAST> right;
};

class LOrExprAST {
public:
    LOrExprAST(unique_ptr<LAndExprAST>&& landExpr)
        : landExpr(std::move(landExpr)) {}
    LOrExprAST(unique_ptr<LOrExprAST>&& left, unique_ptr<LAndExprAST>&& right) 
        : left(std::move(left)), right(std::move(right)) {}

    unique_ptr<LAndExprAST> landExpr;
    unique_ptr<LOrExprAST> left;
    unique_ptr<LAndExprAST> right;
};

class DeclAST {
public:
    DeclAST(unique_ptr<ConstDeclAST>&& constDecl)
        : constDecl(std::move(constDecl)) {}
    DeclAST(unique_ptr<VarDeclAST>&& varDecl)
        : varDecl(std::move(varDecl)) {}

    unique_ptr<ConstDeclAST> constDecl;
    unique_ptr<VarDeclAST> varDecl;
};

class ConstDeclAST {
public:
    ConstDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<DefineAST>>&& constDefs)
        : btype(std::move(btype)), constDefs(std::move(constDefs)) {}

    unique_ptr<BaseType> btype;
    vector<unique_ptr<DefineAST>> constDefs;
};

class VarDeclAST {
public:
    VarDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<DefineAST>>&& varDefs)
        : btype(std::move(btype)), varDefs(std::move(varDefs)) {}

    unique_ptr<BaseType> btype;
    vector<unique_ptr<DefineAST>> varDefs;
};

class ConstInitValAST {
public:
    ConstInitValAST(unique_ptr<ConstExprAST>&& constExpr)
        : constExpr(std::move(constExpr)) {}

    unique_ptr<ConstExprAST> constExpr;
};

class InitValAST {
public:
    InitValAST(unique_ptr<ConstExprAST>&& constExpr)
        : constExpr(std::move(constExpr)) {}

    unique_ptr<ConstExprAST> constExpr;
};

class BlockItemAST {
public:
    BlockItemAST(unique_ptr<DeclAST>&& decl) : decl(std::move(decl)) {}
    BlockItemAST(unique_ptr<StmtAST>&& stmt) : stmt(std::move(stmt)) {}

    unique_ptr<DeclAST> decl;
    unique_ptr<StmtAST> stmt;
};

class LValAST {
public:
    LValAST(string ident) : ident(std::move(ident)) {}

    string ident;
};

class ConstExprAST {
public:
    ConstExprAST(unique_ptr<ExprAST>&& expr) : expr(std::move(expr)) {}

    unique_ptr<ExprAST> expr;
};

class FuncFParamAST {
public:
    FuncFParamAST(unique_ptr<BaseType>&& btype, string ident)
        : btype(std::move(btype)), ident(std::move(ident)) {}

    unique_ptr<BaseType> btype;
    string ident;
};

class FuncRParamAST {
public:
    FuncRParamAST(unique_ptr<ExprAST>&& expr) : expr(std::move(expr)) {}

    unique_ptr<ExprAST> expr;
};