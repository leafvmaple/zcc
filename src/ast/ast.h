#pragma once

#include <memory>
#include <string>
#include <vector>

#include "type.h"
#include "ir/ir.h"

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
class DeclAST;
class PrimaryExprAST;
class UnaryExprAST;
class MulExprAST;
class AddExprAST;
class RelExprAST;
class EqExprAST;
class LAndExprAST;
class FuncRParamAST;

class ConstDefAST {
public:
    ConstDefAST(string ident, unique_ptr<ConstInitValAST>&& initVal);
    ConstDefAST(string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs, unique_ptr<ConstInitValAST>&& initVal);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Codegen(Env<Type, Value, BasicBlock, Function>* env, Type* type);

    string ident;
    vector<unique_ptr<ConstExprAST>> sizeExprs;
    unique_ptr<ConstInitValAST> initVal;
};

class VarDefAST {
public:
    VarDefAST(string ident);
    VarDefAST(string ident, unique_ptr<InitValAST>&& initVal);
    VarDefAST(string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs);
    VarDefAST(string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs, unique_ptr<InitValAST>&& initVal);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Codegen(Env<Type, Value, BasicBlock, Function>* env, Type* type);

    string ident;
    vector<unique_ptr<ConstExprAST>> sizeExprs;
    unique_ptr<InitValAST> initVal;
};

struct CompUnitAST {
public:
    void AddFuncDef(unique_ptr<FuncDefAST>&& funcDef);
    void AddDecl(unique_ptr<DeclAST>&& decl);
    
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Codegen(Env<Type, Value, BasicBlock, Function>* env);

    vector<unique_ptr<FuncDefAST>> funcDefs;
    vector<unique_ptr<DeclAST>> decls;
};

class FuncDefAST {
public:
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BlockAST>&& block);
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, vector<unique_ptr<FuncFParamAST>>&& params, unique_ptr<BlockAST>&& block);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Codegen(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<BaseType> funcType;
    string ident;
    vector<unique_ptr<FuncFParamAST>> params;
    unique_ptr<BlockAST> block;
};

class BlockAST {
public:
    BlockAST(vector<unique_ptr<BlockItemAST>>&& items);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Codegen(Env<Type, Value, BasicBlock, Function>* env);

    vector<unique_ptr<BlockItemAST>> items;
};

class StmtAST {
public:
    enum class TYPE {
        Assign,
        Expr,
        Block,
        If,
        Ret,
        While,
        Break,
        Continue
    };

    StmtAST(TYPE type);
    StmtAST(TYPE type, unique_ptr<ExprAST>&& expr);
    StmtAST(TYPE type, unique_ptr<BlockAST>&& block);
    StmtAST(TYPE type, unique_ptr<LValAST>&& lval, unique_ptr<ExprAST>&& expr);
    StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt);
    StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt, unique_ptr<StmtAST>&& elseStmt);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Codegen(Env<Type, Value, BasicBlock, Function>* env);

    TYPE type;
    unique_ptr<LValAST> lval;
    unique_ptr<ExprAST> expr;
    unique_ptr<BlockAST> block;
    unique_ptr<ExprAST> cond;
    unique_ptr<StmtAST> thenStmt;
    unique_ptr<StmtAST> elseStmt;
};

class ExprAST {
public:
    ExprAST(unique_ptr<LOrExprAST>&& lorExpr);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<LOrExprAST> lorExpr;
};

class PrimaryExprAST {
public:
    enum class TYPE {
        Expr,
        LVal,
        Number
    };

    PrimaryExprAST(TYPE type, unique_ptr<ExprAST>&& expr);
    PrimaryExprAST(TYPE type, unique_ptr<LValAST>&& lval);
    PrimaryExprAST(TYPE type, unique_ptr<NumberAST>&& value);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    TYPE type;
    unique_ptr<ExprAST> expr;
    unique_ptr<LValAST> lval;
    unique_ptr<NumberAST> value;
};

class NumberAST {
public:
    NumberAST(int value);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);

    int value;
};

class UnaryExprAST {
public:
    enum class TYPE {
        Primary,
        Unary,
        Call
    };

    enum class OP {
        PLUS,
        MINUS,
        NOT
    };

    UnaryExprAST(TYPE type, unique_ptr<PrimaryExprAST>&& primaryExpr);
    UnaryExprAST(TYPE type, OP op, unique_ptr<UnaryExprAST>&& unaryExpr);
    UnaryExprAST(TYPE type, string ident);
    UnaryExprAST(TYPE type, string ident, vector<unique_ptr<ExprAST>>&& callArgs);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    TYPE type;
    OP op;
    string ident;
    unique_ptr<PrimaryExprAST> primaryExpr;
    unique_ptr<UnaryExprAST> unaryExpr;
    vector<unique_ptr<ExprAST>> callArgs;
};

class MulExprAST {
public:
    enum class OP { MUL, DIV, MOD };
    MulExprAST(unique_ptr<UnaryExprAST>&& unaryExpr);
    MulExprAST(OP op, unique_ptr<MulExprAST>&& left, unique_ptr<UnaryExprAST>&& right);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<UnaryExprAST> unaryExpr;
    OP op;
    unique_ptr<MulExprAST> left;
    unique_ptr<UnaryExprAST> right;
};

class AddExprAST {
public:
    enum class OP { ADD, SUB };
    AddExprAST(unique_ptr<MulExprAST>&& mulExpr);
    AddExprAST(OP op, unique_ptr<AddExprAST>&& left, unique_ptr<MulExprAST>&& right);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    OP op;
    unique_ptr<MulExprAST> mulExpr;
    unique_ptr<AddExprAST> left;
    unique_ptr<MulExprAST> right;
};

class RelExprAST {
public:
    enum class Op { LT, GT, LE, GE };

    RelExprAST(unique_ptr<AddExprAST>&& addExpr);
    RelExprAST(unique_ptr<RelExprAST>&& left, Op op, unique_ptr<AddExprAST>&& right);
    
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<AddExprAST> addExpr;
    unique_ptr<RelExprAST> left;
    Op op;
    unique_ptr<AddExprAST> right;
};

class EqExprAST {
public:
    enum class Op { EQ, NE };

    EqExprAST(unique_ptr<RelExprAST>&& relExpr);
    EqExprAST(unique_ptr<EqExprAST>&& left, Op op, unique_ptr<RelExprAST>&& right);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<RelExprAST> relExpr;
    unique_ptr<EqExprAST> left;
    Op op;
    unique_ptr<RelExprAST> right;
};

class LAndExprAST {
public:
    LAndExprAST(unique_ptr<EqExprAST>&& eqExpr);
    LAndExprAST(unique_ptr<LAndExprAST>&& left, unique_ptr<EqExprAST>&& right);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<EqExprAST> eqExpr;
    unique_ptr<LAndExprAST> left;
    unique_ptr<EqExprAST> right;
};

class LOrExprAST {
public:
    LOrExprAST(unique_ptr<LAndExprAST>&& landExpr);
    LOrExprAST(unique_ptr<LOrExprAST>&& left, unique_ptr<LAndExprAST>&& right);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<LAndExprAST> landExpr;
    unique_ptr<LOrExprAST> left;
    unique_ptr<LAndExprAST> right;
};

class DeclAST {
public:
    DeclAST(unique_ptr<ConstDeclAST>&& constDecl);
    DeclAST(unique_ptr<VarDeclAST>&& varDecl);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Codegen(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<ConstDeclAST> constDecl;
    unique_ptr<VarDeclAST> varDecl;
};

class ConstDeclAST {
public:
    ConstDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<ConstDefAST>>&& constDefs);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Codegen(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<BaseType> btype;
    vector<unique_ptr<ConstDefAST>> constDefs;
};

class VarDeclAST {
public:
    VarDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<VarDefAST>>&& varDefs);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Codegen(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<BaseType> btype;
    vector<unique_ptr<VarDefAST>> varDefs;
};

class ConstInitValAST {
public:
    ConstInitValAST();
    ConstInitValAST(unique_ptr<ConstExprAST>&& expr);
    ConstInitValAST(vector<unique_ptr<ConstExprAST>>&& constExprs);
    ConstInitValAST(vector<unique_ptr<ConstInitValAST>>&& subVals);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<ConstExprAST> expr;
    vector<unique_ptr<ConstInitValAST>> subVals;
    bool isArray;
};

class InitValAST {
public:
    InitValAST();
    InitValAST(unique_ptr<ExprAST>&& expr);
    InitValAST(vector<unique_ptr<ExprAST>>&& exprs);
    InitValAST(vector<unique_ptr<InitValAST>>&& subVals);


    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env, Value* addr);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env, vector<int> shape, int dim);

    unique_ptr<ExprAST> expr;
    vector<unique_ptr<InitValAST>> subVals;
    bool isArray;
};

class BlockItemAST {
public:
    BlockItemAST(unique_ptr<DeclAST>&& decl);
    BlockItemAST(unique_ptr<StmtAST>&& stmt);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void ToValue(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<DeclAST> decl;
    unique_ptr<StmtAST> stmt;
};

class LValAST {
public:
    LValAST(string ident);
    LValAST(string ident, vector<unique_ptr<ExprAST>>&& indies);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);

    string ident;
    vector<unique_ptr<ExprAST>> indies;
};

class ConstExprAST {
public:
    ConstExprAST(unique_ptr<ExprAST>&& expr);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToNumber(Env<Type, Value, BasicBlock, Function>* env);
    template<typename Type, typename Value, typename BasicBlock, typename Function>
    int ToInteger(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<ExprAST> expr;
};

class FuncFParamAST {
public:
    FuncFParamAST(unique_ptr<BaseType>&& btype, string ident);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<BaseType> btype;
    string ident;
};

class FuncRParamAST {
public:
    FuncRParamAST(unique_ptr<ExprAST>&& expr);

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    Value* ToValue(Env<Type, Value, BasicBlock, Function>* env);

    unique_ptr<ExprAST> expr;
};

#include "ast/ast.tpp"