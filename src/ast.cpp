#include "ast.h"
#include "llvmir.h"

void CompUnitAST::AddFuncDef(unique_ptr<BaseAST>&& funcDef) {
    this->funcDef = std::move(funcDef);
}

std::string CompUnitAST::ToString() const {
    return "CompUnitAST { " + funcDef->ToString() + " }";
}

llvm::Value* CompUnitAST::Codegen(LLVMParams* params) {
    return funcDef->Codegen(params);
}

std::string FuncDefAST::ToString() const {
    return "FuncDefAST { " + funcType->ToString() + ", " + ident + ", " + block->ToString() + " }";
}

llvm::Value* FuncDefAST::Codegen(LLVMParams* params) {
    auto intType = llvm::Type::getInt32Ty(params->TheContext);

    std::vector<llvm::Type*> ParamTypes {};  // Params is 0

    auto* FuncType = llvm::FunctionType::get(intType, ParamTypes, false);
    auto* Func = llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage, ident, &params->TheModule);
    auto* BB = llvm::BasicBlock::Create(params->TheContext, "entry", Func);

    params->Builder.SetInsertPoint(BB);
    
    block->Codegen(params);

    return Func;
}

std::string FuncTypeAST::ToString() const {
    return "FuncTypeAST { " + type + " }";
}

std::string BlockAST::ToString() const {
    return "BlockAST { " + stmts->ToString() + " }";
}

llvm::Value* BlockAST::Codegen(LLVMParams* params) {
    return stmts->Codegen(params);
}

std::string StmtAST::ToString() const {
    return "StmtAST { " + num->ToString() + " }";
}

llvm::Value* StmtAST::Codegen(LLVMParams* params) {
    auto* val = num->Codegen(params);
    return params->Builder.CreateRet(val);
}

std::string NumAST::ToString() const {
    return std::to_string(value);
}

llvm::Value* NumAST::Codegen(LLVMParams* params) {
    return llvm::ConstantInt::get(params->TheContext, llvm::APInt(32, value, true));
}