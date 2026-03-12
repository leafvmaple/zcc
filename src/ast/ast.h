#pragma once

#include <memory>
#include <string>
#include <vector>

#include "type.h"

using std::unique_ptr;
using std::vector;
using std::string;

namespace llvm {
    class Value;
    class Type;
    class BasicBlock;
    class Function;
    class FunctionType;
}
class CodeGen;

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
class BinaryExprAST;
class LAndExprAST;
class FuncRParamAST;

class ConstDefAST {
public:
    ConstDefAST(string ident, unique_ptr<ConstInitValAST>&& initVal);
    ConstDefAST(string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs, unique_ptr<ConstInitValAST>&& initVal);

    void Codegen(CodeGen* cg, llvm::Type* type);

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

    void Codegen(CodeGen* cg, llvm::Type* type);

    string ident;
    vector<unique_ptr<ConstExprAST>> sizeExprs;
    unique_ptr<InitValAST> initVal;
};

struct CompUnitAST {
public:
    void AddFuncDef(unique_ptr<FuncDefAST>&& funcDef);
    void AddDecl(unique_ptr<DeclAST>&& decl);
    void Codegen(CodeGen* cg);

    vector<unique_ptr<FuncDefAST>> funcDefs;
    vector<unique_ptr<DeclAST>> decls;
};

class FuncDefAST {
public:
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BlockAST>&& block);
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, vector<unique_ptr<FuncFParamAST>>&& params, unique_ptr<BlockAST>&& block);

    void Codegen(CodeGen* cg);

    unique_ptr<BaseType> funcType;
    string ident;
    vector<unique_ptr<FuncFParamAST>> params;
    unique_ptr<BlockAST> block;
};

class BlockAST {
public:
    BlockAST(vector<unique_ptr<BlockItemAST>>&& items);
    void Codegen(CodeGen* cg);

    vector<unique_ptr<BlockItemAST>> items;
};

class StmtAST {
public:
    enum class TYPE {
        Assign, Expr, Block, If, Ret, While, For, Break, Continue
    };

    StmtAST(TYPE type);
    StmtAST(TYPE type, unique_ptr<ExprAST>&& expr);
    StmtAST(TYPE type, unique_ptr<BlockAST>&& block);
    StmtAST(TYPE type, unique_ptr<LValAST>&& lval, unique_ptr<ExprAST>&& expr);
    StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt);
    StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt, unique_ptr<StmtAST>&& elseStmt);

    void Codegen(CodeGen* cg);

    TYPE type;
    unique_ptr<LValAST> lval;
    unique_ptr<ExprAST> expr;
    unique_ptr<BlockAST> block;
    unique_ptr<ExprAST> cond;
    unique_ptr<StmtAST> thenStmt;
    unique_ptr<StmtAST> elseStmt;
    unique_ptr<DeclAST> forDecl;
    unique_ptr<StmtAST> forInitStmt;
    unique_ptr<StmtAST> forStepStmt;
};

class ExprAST {
public:
    ExprAST(unique_ptr<LOrExprAST>&& lorExpr);

    llvm::Value* ToValue(CodeGen* cg);
    llvm::Value* ToNumber(CodeGen* cg);

    unique_ptr<LOrExprAST> lorExpr;
};

class PrimaryExprAST {
public:
    enum class TYPE { Expr, LVal, Number, String };

    PrimaryExprAST(TYPE type, unique_ptr<ExprAST>&& expr);
    PrimaryExprAST(TYPE type, unique_ptr<LValAST>&& lval);
    PrimaryExprAST(TYPE type, unique_ptr<NumberAST>&& value);
    PrimaryExprAST(string strVal);

    llvm::Value* ToValue(CodeGen* cg);
    llvm::Value* ToNumber(CodeGen* cg);

    TYPE type;
    unique_ptr<ExprAST> expr;
    unique_ptr<LValAST> lval;
    unique_ptr<NumberAST> value;
    string strVal;
};

class NumberAST {
public:
    NumberAST(int value);
    llvm::Value* ToValue(CodeGen* cg);

    int value;
};

class UnaryExprAST {
public:
    enum class TYPE { Primary, Unary, Call };
    enum class OP { PLUS, MINUS, NOT };

    UnaryExprAST(TYPE type, unique_ptr<PrimaryExprAST>&& primaryExpr);
    UnaryExprAST(TYPE type, OP op, unique_ptr<UnaryExprAST>&& unaryExpr);
    UnaryExprAST(TYPE type, string ident);
    UnaryExprAST(TYPE type, string ident, vector<unique_ptr<ExprAST>>&& callArgs);

    llvm::Value* ToValue(CodeGen* cg);
    llvm::Value* ToNumber(CodeGen* cg);

    TYPE type;
    OP op;
    string ident;
    unique_ptr<PrimaryExprAST> primaryExpr;
    unique_ptr<UnaryExprAST> unaryExpr;
    vector<unique_ptr<ExprAST>> callArgs;
};

class BinaryExprAST {
public:
    enum class Op {
        ADD, SUB, MUL, DIV, MOD,
        LT, GT, LE, GE, EQ, NE,
    };

    BinaryExprAST(unique_ptr<UnaryExprAST>&& operand);
    BinaryExprAST(Op op, unique_ptr<BinaryExprAST>&& lhs, unique_ptr<BinaryExprAST>&& rhs);

    llvm::Value* ToValue(CodeGen* cg);
    llvm::Value* ToNumber(CodeGen* cg);

    Op op;
    unique_ptr<UnaryExprAST> operand;
    unique_ptr<BinaryExprAST> lhs;
    unique_ptr<BinaryExprAST> rhs;
};

class LAndExprAST {
public:
    LAndExprAST(unique_ptr<BinaryExprAST>&& operand);
    LAndExprAST(unique_ptr<LAndExprAST>&& left, unique_ptr<BinaryExprAST>&& right);

    llvm::Value* ToValue(CodeGen* cg);
    llvm::Value* ToNumber(CodeGen* cg);

    unique_ptr<BinaryExprAST> operand;
    unique_ptr<LAndExprAST> left;
    unique_ptr<BinaryExprAST> right;
};

class LOrExprAST {
public:
    LOrExprAST(unique_ptr<LAndExprAST>&& operand);
    LOrExprAST(unique_ptr<LOrExprAST>&& left, unique_ptr<LAndExprAST>&& right);

    llvm::Value* ToValue(CodeGen* cg);
    llvm::Value* ToNumber(CodeGen* cg);

    unique_ptr<LAndExprAST> operand;
    unique_ptr<LOrExprAST> left;
    unique_ptr<LAndExprAST> right;
};

class DeclAST {
public:
    DeclAST(unique_ptr<ConstDeclAST>&& constDecl);
    DeclAST(unique_ptr<VarDeclAST>&& varDecl);

    void Codegen(CodeGen* cg);

    unique_ptr<ConstDeclAST> constDecl;
    unique_ptr<VarDeclAST> varDecl;
};

class ConstDeclAST {
public:
    ConstDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<ConstDefAST>>&& constDefs);
    void Codegen(CodeGen* cg);

    unique_ptr<BaseType> btype;
    vector<unique_ptr<ConstDefAST>> constDefs;
};

class VarDeclAST {
public:
    VarDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<VarDefAST>>&& varDefs);
    void Codegen(CodeGen* cg);

    unique_ptr<BaseType> btype;
    vector<unique_ptr<VarDefAST>> varDefs;
};

class ConstInitValAST {
public:
    ConstInitValAST();
    ConstInitValAST(unique_ptr<ConstExprAST>&& expr);
    ConstInitValAST(vector<unique_ptr<ConstExprAST>>&& constExprs);
    ConstInitValAST(vector<unique_ptr<ConstInitValAST>>&& subVals);

    llvm::Value* ToNumber(CodeGen* cg);

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

    llvm::Value* ToValue(CodeGen* cg, llvm::Value* addr);
    llvm::Value* ToNumber(CodeGen* cg, vector<int> shape, int dim);

    unique_ptr<ExprAST> expr;
    vector<unique_ptr<InitValAST>> subVals;
    bool isArray;
};

class BlockItemAST {
public:
    BlockItemAST(unique_ptr<DeclAST>&& decl);
    BlockItemAST(unique_ptr<StmtAST>&& stmt);

    void ToValue(CodeGen* cg);

    unique_ptr<DeclAST> decl;
    unique_ptr<StmtAST> stmt;
};

class LValAST {
public:
    LValAST(string ident);
    LValAST(string ident, vector<unique_ptr<ExprAST>>&& indies);

    llvm::Value* ToValue(CodeGen* cg);
    llvm::Value* ToNumber(CodeGen* cg);
    llvm::Value* ToPointer(CodeGen* cg);

    string ident;
    vector<unique_ptr<ExprAST>> indies;
};

class ConstExprAST {
public:
    ConstExprAST(unique_ptr<ExprAST>&& expr);

    llvm::Value* ToValue(CodeGen* cg);
    llvm::Value* ToNumber(CodeGen* cg);
    int ToInteger(CodeGen* cg);

    unique_ptr<ExprAST> expr;
};

class FuncFParamAST {
public:
    FuncFParamAST(unique_ptr<BaseType>&& btype, string ident, bool isArray = false);
    FuncFParamAST(unique_ptr<BaseType>&& btype, string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs);

    llvm::Type* ToType(CodeGen* cg);
    llvm::Value* Alloca(CodeGen* cg, llvm::Type* type);

    unique_ptr<BaseType> btype;
    string ident;
    vector<unique_ptr<ConstExprAST>> sizeExprs;
    bool isArray;
};

class FuncRParamAST {
public:
    FuncRParamAST(unique_ptr<ExprAST>&& expr);
    llvm::Value* ToValue(CodeGen* cg);

    unique_ptr<ExprAST> expr;
};
