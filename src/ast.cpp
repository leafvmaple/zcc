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
    std::vector<llvm::Type*> ParamTypes {};  // Params is 0

    llvm::FunctionType* FuncType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*params->TheContext),
        ParamTypes,
        false
    );

    llvm::Function* Func = llvm::Function::Create(
        FuncType,
        llvm::Function::ExternalLinkage,
        ident,
        params->TheModule.get()
    );

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(*params->TheContext, "entry", Func);
    params->Builder->SetInsertPoint(BB);

    return Func;
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