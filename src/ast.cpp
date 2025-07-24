#include "ast.h"

void CompUnitAST::AddFuncDef(unique_ptr<BaseAST>&& funcDef) {
    this->funcDef = std::move(funcDef);
}

std::string CompUnitAST::ToString() const {
    return "CompUnitAST { " + funcDef->ToString() + " }";
}

std::string FuncDefAST::ToString() const {
    return "FuncDefAST { " + funcType->ToString() + ", " + ident + ", " + block->ToString() + " }";
}

std::string FuncTypeAST::ToString() const {
    return "FuncTypeAST { " + type + " }";
}

std::string BlockAST::ToString() const {
    return "BlockAST { " + stmts->ToString() + " }";
}

std::string StmtAST::ToString() const {
    return "StmtAST { " + num->ToString() + " }";
}

std::string NumAST::ToString() const {
    return std::to_string(value);
}