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
    params->symtab.EnterScope();
    for (auto& block : blocks)
        block->Codegen(params);
    params->symtab.ExitScope();

    return nullptr;  // Block does not return a value
}

llvm::Value* BlockItemAST::Codegen(LLVMParams* params) {
    return ast->Codegen(params);
}

llvm::Value* StmtAST::Codegen(LLVMParams* params) {
    if (type == Type::Assign) {
        auto* lvalVal = expr1->Codegen(params);
        auto* initVal = expr2->Codegen(params);
        params->Builder.CreateStore(initVal, lvalVal);
    } else if (type == Type::Expr) {
        if (expr1)
            expr1->Codegen(params);
    } else if (type == Type::Block) {
        expr1->Codegen(params);
    } else if (type == Type::If) {
        auto* cond = params->Builder.CreateICmpNE(expr1->Codegen(params), params->Builder.getInt32(0));

        auto* func = params->Builder.GetInsertBlock()->getParent();
        auto* thenBB = llvm::BasicBlock::Create(params->TheContext, "then", func);
        llvm::BasicBlock* endBB{};

        if (expr3) {
            auto* elseBB = llvm::BasicBlock::Create(params->TheContext, "else", func);
            endBB = llvm::BasicBlock::Create(params->TheContext, "end", func);
            params->Builder.CreateCondBr(cond, thenBB, elseBB);

            params->Builder.SetInsertPoint(elseBB);
            expr3->Codegen(params);
            params->Builder.CreateBr(endBB);
        }
        else {
            endBB = llvm::BasicBlock::Create(params->TheContext, "end", func);
            params->Builder.CreateCondBr(cond, thenBB, endBB);
        }
        
        params->Builder.SetInsertPoint(thenBB);
        expr2->Codegen(params);
        params->Builder.CreateBr(endBB);

        params->Builder.SetInsertPoint(endBB);
    } else if (type == Type::Ret) {
        params->Builder.CreateRet(expr1->Codegen(params));
    }

    return nullptr;  // Assignment does not return a value
}

llvm::Value* ExprAST::Codegen(LLVMParams* params) {
    return unaryExpr->Codegen(params);
}

llvm::Value* PrimaryExprAST::Codegen(LLVMParams* params) {
    if (type == Type::Expr) {
        return ast->Codegen(params);
    } else if (type == Type::LVal) {
        auto* addr = ast->Codegen(params);
        auto* type = params->symtab.GetSymbolType(addr);
        return params->Builder.CreateLoad(type, addr);
    } else if (type == Type::Number) {
        return ast->Codegen(params);
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

llvm::Value* MulExprAST::Codegen(LLVMParams* params) {
    if (mulExpr) {
        auto* leftVal = mulExpr->Codegen(params);
        auto* rightVal = unaryExpr->Codegen(params);
        
        if (op == "*") {
            return params->Builder.CreateMul(leftVal, rightVal);
        } else if (op == "/") {
            return params->Builder.CreateSDiv(leftVal, rightVal);
        } else if (op == "%") {
            return params->Builder.CreateSRem(leftVal, rightVal);
        }
    } else {
        return unaryExpr->Codegen(params);
    }
    return nullptr;  // Should not reach here
}

llvm::Value* AddExprAST::Codegen(LLVMParams* params) {
    if (addExpr) {
        auto* leftVal = addExpr->Codegen(params);
        auto* rightVal = mulExpr->Codegen(params);
        
        if (op == "+") {
            return params->Builder.CreateAdd(leftVal, rightVal);
        } else if (op == "-") {
            return params->Builder.CreateSub(leftVal, rightVal);
        }
    } else {
        return mulExpr->Codegen(params);
    }
    return nullptr;  // Should not reach here
}

llvm::Value* RelExprAST::Codegen(LLVMParams* params) {
    if (expr2) {
        auto* leftVal = expr1->Codegen(params);
        auto* rightVal = expr2->Codegen(params);
        llvm::Value* res{};
        
        switch (op) {
            case Op::LT:
                res = params->Builder.CreateICmpSLT(leftVal, rightVal);
                break;
            case Op::GT:
                res = params->Builder.CreateICmpSGT(leftVal, rightVal);
                break;
            case Op::LE:
                res = params->Builder.CreateICmpSLE(leftVal, rightVal);
                break;
            case Op::GE:
                res = params->Builder.CreateICmpSGE(leftVal, rightVal);
                break;
        }

        return params->Builder.CreateZExt(res, llvm::Type::getInt32Ty(params->TheContext));
    }
    return expr1->Codegen(params);
}

llvm::Value* EqExprAST::Codegen(LLVMParams* params) {
    if (expr2) {
        auto* leftVal = expr1->Codegen(params);
        auto* rightVal = expr2->Codegen(params);
        llvm::Value* res{};
        
        switch (op) {
            case Op::EQ:
                res = params->Builder.CreateICmpEQ(leftVal, rightVal);
                break;
            case Op::NE:
                res = params->Builder.CreateICmpNE(leftVal, rightVal);
                break;
        }

        return params->Builder.CreateZExt(res, llvm::Type::getInt32Ty(params->TheContext));
    }
    return expr1->Codegen(params);
}

llvm::Value* LAndExprAST::Codegen(LLVMParams* params) {
    if (expr2) {
        auto* leftVal = expr1->Codegen(params);
        auto* rightVal = expr2->Codegen(params);
        auto* res = params->Builder.CreateAnd(leftVal, rightVal);
        return params->Builder.CreateZExt(res, llvm::Type::getInt32Ty(params->TheContext));
    }
    return expr1->Codegen(params);
}

llvm::Value* LOrExprAST::Codegen(LLVMParams* params) {
    if (expr2) {
        auto* leftVal = expr1->Codegen(params);
        auto* rightVal = expr2->Codegen(params);
        auto* res = params->Builder.CreateOr(leftVal, rightVal);
        return params->Builder.CreateZExt(res, llvm::Type::getInt32Ty(params->TheContext));
    }
    return expr1->Codegen(params);
}

llvm::Value* DeclAST::Codegen(LLVMParams* params) {
    return constDecl->Codegen(params);
}

llvm::Value* ConstDeclAST::Codegen(LLVMParams* params) {
    auto* type = btype->Codegen(params);
    auto* constVal = constDef->initVal->Codegen(params);
    params->symtab.AddSymbol(constDef->ident, type, constVal);

    return nullptr;  // Const declarations do not return a value
}

llvm::Value* VarDeclAST::Codegen(LLVMParams* params) {
    auto* type = btype->Codegen(params);
    auto* varAddr = params->Builder.CreateAlloca(type, nullptr, localDef->ident);
    if (localDef->initVal)
        params->Builder.CreateStore(localDef->initVal->Codegen(params), varAddr);

    params->symtab.AddSymbol(localDef->ident,  type, varAddr);
    return nullptr;  // Const declarations do not return a value
}

llvm::Value* ConstInitValAST::Codegen(LLVMParams* params) {
    return expr->Codegen(params);
}

llvm::Value* InitValAST::Codegen(LLVMParams* params) {
    return expr->Codegen(params);
}

llvm::Value* LValAST::Codegen(LLVMParams* params) {
    return params->symtab.GetSymbolValue(ident);
}

llvm::Value* ConstExprAST::Codegen(LLVMParams* params) {
    return expr->Codegen(params);
}