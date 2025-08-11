#include "ast/ast.h"

#include "../libkoopa/include/function.h"
#include "llvm/IR/Function.h"

ConstDefAST::ConstDefAST(string ident, unique_ptr<ConstInitValAST>&& constInitVal)
    : ident(std::move(ident)), constInitVal(std::move(constInitVal)) {}

ConstDefAST::ConstDefAST(string ident, unique_ptr<ConstExprAST>&& sizeExpr, unique_ptr<ConstInitValAST>&& constInitVal)
    : ident(std::move(ident)), sizeExpr(std::move(sizeExpr)), constInitVal(std::move(constInitVal)) {}


VarDefAST::VarDefAST(string ident)
    : ident(std::move(ident)) {}

VarDefAST::VarDefAST(string ident, unique_ptr<ConstExprAST>&& sizeExpr)
    : ident(std::move(ident)), sizeExpr(std::move(sizeExpr)) {}

VarDefAST::VarDefAST(string ident, unique_ptr<InitValAST>&& initVal)
    : ident(std::move(ident)), initVal(std::move(initVal)) {}

VarDefAST::VarDefAST(string ident, unique_ptr<ConstExprAST>&& sizeExpr, unique_ptr<InitValAST>&& initVal)
    : ident(std::move(ident)), sizeExpr(std::move(sizeExpr)), initVal(std::move(initVal)) {}


void CompUnitAST::AddFuncDef(unique_ptr<FuncDefAST>&& funcDef) {
    funcDefs.emplace_back(std::move(funcDef));
}

void CompUnitAST::AddDecl(unique_ptr<DeclAST>&& decl) {
    decls.emplace_back(std::move(decl));
}


FuncDefAST::FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BlockAST>&& block)
    : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}

FuncDefAST::FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, vector<unique_ptr<FuncFParamAST>>&& params, unique_ptr<BlockAST>&& block)
    : funcType(std::move(funcType)), ident(std::move(ident)), params(std::move(params)), block(std::move(block)) {}


BlockAST::BlockAST(vector<unique_ptr<BlockItemAST>>&& items)
    : items(std::move(items)) {}


StmtAST::StmtAST(TYPE type)
    : type(type) {}

StmtAST::StmtAST(TYPE type, unique_ptr<ExprAST>&& expr)
    : type(type), expr(std::move(expr)) {}

StmtAST::StmtAST(TYPE type, unique_ptr<BlockAST>&& block)
    : type(type), block(std::move(block)) {}

StmtAST::StmtAST(TYPE type, unique_ptr<LValAST>&& lval, unique_ptr<ExprAST>&& expr) 
    : type(type), lval(std::move(lval)), expr(std::move(expr)) {}

StmtAST::StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt) 
    : type(type), cond(std::move(cond)), thenStmt(std::move(thenStmt)) {}

StmtAST::StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt, unique_ptr<StmtAST>&& elseStmt) 
    : type(type), cond(std::move(cond)), thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}


ExprAST::ExprAST(unique_ptr<LOrExprAST>&& lorExpr)
    : lorExpr(std::move(lorExpr)) {}


PrimaryExprAST::PrimaryExprAST(TYPE type, unique_ptr<ExprAST>&& expr)
    : type(type), expr(std::move(expr)) {}

PrimaryExprAST::PrimaryExprAST(TYPE type, unique_ptr<LValAST>&& lval)
    : type(type), lval(std::move(lval)) {}

PrimaryExprAST::PrimaryExprAST(TYPE type, unique_ptr<NumberAST>&& value)
    : type(type), value(std::move(value)) {}


NumberAST::NumberAST(int value)
    : value(value) {}


UnaryExprAST::UnaryExprAST(TYPE type, unique_ptr<PrimaryExprAST>&& primaryExpr) 
    : type(type), primaryExpr(std::move(primaryExpr)) {}

UnaryExprAST::UnaryExprAST(TYPE type, OP op, unique_ptr<UnaryExprAST>&& unaryExpr) 
    : type(type), op(op), unaryExpr(std::move(unaryExpr)) {}

UnaryExprAST::UnaryExprAST(TYPE type, string ident)
    : type(type), ident(std::move(ident)) {}

UnaryExprAST::UnaryExprAST(TYPE type, string ident, vector<unique_ptr<ExprAST>>&& callArgs) 
    : type(type), ident(std::move(ident)), callArgs(std::move(callArgs)) {}


MulExprAST::MulExprAST(unique_ptr<UnaryExprAST>&& unaryExpr)
    : unaryExpr(std::move(unaryExpr)) {}

MulExprAST::MulExprAST(OP op, unique_ptr<MulExprAST>&& left, unique_ptr<UnaryExprAST>&& right) 
    : op(op), left(std::move(left)), right(std::move(right)) {}


AddExprAST::AddExprAST(unique_ptr<MulExprAST>&& mulExpr)
    : mulExpr(std::move(mulExpr)) {}

AddExprAST::AddExprAST(OP op, unique_ptr<AddExprAST>&& left, unique_ptr<MulExprAST>&& right) 
    : op(op), left(std::move(left)), right(std::move(right)) {}


RelExprAST::RelExprAST(unique_ptr<AddExprAST>&& addExpr)
    : addExpr(std::move(addExpr)) {}

RelExprAST::RelExprAST(unique_ptr<RelExprAST>&& left, Op op, unique_ptr<AddExprAST>&& right) 
    : left(std::move(left)), op(op), right(std::move(right)) {}


EqExprAST::EqExprAST(unique_ptr<RelExprAST>&& relExpr)
    : relExpr(std::move(relExpr)) {}

EqExprAST::EqExprAST(unique_ptr<EqExprAST>&& left, Op op, unique_ptr<RelExprAST>&& right) 
    : left(std::move(left)), op(op), right(std::move(right)) {}


LAndExprAST::LAndExprAST(unique_ptr<EqExprAST>&& eqExpr)
    : eqExpr(std::move(eqExpr)) {}

LAndExprAST::LAndExprAST(unique_ptr<LAndExprAST>&& left, unique_ptr<EqExprAST>&& right) 
    : left(std::move(left)), right(std::move(right)) {}


LOrExprAST::LOrExprAST(unique_ptr<LAndExprAST>&& landExpr)
    : landExpr(std::move(landExpr)) {}

LOrExprAST::LOrExprAST(unique_ptr<LOrExprAST>&& left, unique_ptr<LAndExprAST>&& right) 
    : left(std::move(left)), right(std::move(right)) {}


DeclAST::DeclAST(unique_ptr<ConstDeclAST>&& constDecl)
    : constDecl(std::move(constDecl)) {}

DeclAST::DeclAST(unique_ptr<VarDeclAST>&& varDecl)
    : varDecl(std::move(varDecl)) {}


ConstDeclAST::ConstDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<ConstDefAST>>&& constDefs)
    : btype(std::move(btype)), constDefs(std::move(constDefs)) {}


VarDeclAST::VarDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<VarDefAST>>&& varDefs)
    : btype(std::move(btype)), varDefs(std::move(varDefs)) {}


ConstInitValAST::ConstInitValAST(unique_ptr<ConstExprAST>&& constExpr)
    : constExpr(std::move(constExpr)), isArray(false) {}

ConstInitValAST::ConstInitValAST()
    : isArray(true) {}

ConstInitValAST::ConstInitValAST(vector<unique_ptr<ConstExprAST>>&& constExprs)
    : constExprs(std::move(constExprs)), isArray(true) {}


InitValAST::InitValAST(unique_ptr<ExprAST>&& expr)
    : expr(std::move(expr)), isArray(false) {}

InitValAST::InitValAST()
    : isArray(true) {}

InitValAST::InitValAST(vector<unique_ptr<ExprAST>>&& exprs)
    : exprs(std::move(exprs)), isArray(true) {}


BlockItemAST::BlockItemAST(unique_ptr<DeclAST>&& decl)
    : decl(std::move(decl)) {}

BlockItemAST::BlockItemAST(unique_ptr<StmtAST>&& stmt)
    : stmt(std::move(stmt)) {}


LValAST::LValAST(string ident)
    : ident(std::move(ident)) {}

LValAST::LValAST(string ident, unique_ptr<ExprAST>&& index)
    : ident(std::move(ident)), index(std::move(index)) {}


ConstExprAST::ConstExprAST(unique_ptr<ExprAST>&& expr)
    : expr(std::move(expr)) {}


FuncFParamAST::FuncFParamAST(unique_ptr<BaseType>&& btype, string ident)
    : btype(std::move(btype)), ident(std::move(ident)) {}


FuncRParamAST::FuncRParamAST(unique_ptr<ExprAST>&& expr)
    : expr(std::move(expr)) {}
