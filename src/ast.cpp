#include "ast.h"
#include "llvmir.h"

void CompUnitAST::AddFuncDef(unique_ptr<BaseAST>&& funcDef) {
    this->funcDef = std::move(funcDef);
}

llvm::Value* CompUnitAST::Codegen(LLVMParams* params) {
    return funcDef->Codegen(params);
}

llvm::Value* FuncDefAST::Codegen(LLVMParams* params) {
    std::vector<llvm::Type*> ParamTypes {};  // Params is 0

    auto* retType = funcType->Codegen(params);
    auto* FuncType = llvm::FunctionType::get(retType, ParamTypes, false);
    auto* Func = llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage, ident, &params->TheModule);
    auto* BB = llvm::BasicBlock::Create(params->TheContext, "entry", Func);

    params->Builder.SetInsertPoint(BB);
    
    block->Codegen(params);

    return Func;
}

llvm::Value* BlockAST::Codegen(LLVMParams* params) {
    return stmts->Codegen(params);
}

llvm::Value* StmtAST::Codegen(LLVMParams* params) {
    auto* val = expr->Codegen(params);
    return params->Builder.CreateRet(val);
}

llvm::Value* ExprAST::Codegen(LLVMParams* params) {
    return unaryExpr->Codegen(params);
}

llvm::Value* PrimaryExprAST::Codegen(LLVMParams* params) {
    if (expr) {
        return expr->Codegen(params);
    } else if (number) {
        return number->Codegen(params);
    }
    return nullptr;  // Should not reach here
}

llvm::Value* NumberAST::Codegen(LLVMParams* params) {
    return llvm::ConstantInt::get(params->TheContext, llvm::APInt(32, value, true));
}

llvm::Value* UnaryExprAST::Codegen(LLVMParams* params) {
    if (primaryExpr) {
        return primaryExpr->Codegen(params);
    }
    else if (unaryExpr) {
        if (op == "+") {
            return unaryExpr->Codegen(params);
        } else if (op == "-") {
            auto* exprVal = unaryExpr->Codegen(params);
            return params->Builder.CreateNeg(exprVal);
        } else if (op == "!") {
            auto* exprVal = unaryExpr->Codegen(params);
            auto* res = params->Builder.CreateICmpEQ(exprVal, llvm::ConstantInt::get(exprVal->getType(), 0));
            return params->Builder.CreateZExt(res, llvm::Type::getInt32Ty(params->TheContext));
        }
    }
    return nullptr;  // Should not reach here
}