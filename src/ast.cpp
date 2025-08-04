#include "ast.h"
#include "llvm_ir.h"

void CompUnitAST::AddFuncDef(unique_ptr<FuncDefAST>&& funcDef) {
    this->funcDef.emplace_back(std::move(funcDef));
}

llvm::Value* CompUnitAST::Codegen(LLVMEnv* params) {
    for (auto& func : funcDef) {
        func->Codegen(params);
    }
    return nullptr;
}

void CompUnitAST::ToKoopa(KoopaEnv* env) {
    for (auto& func : funcDef) {
        func->ToKoopa(env);
    }
}

llvm::Value* FuncDefAST::Codegen(LLVMEnv* params) {
    auto* FuncType = llvm::FunctionType::get(funcType->Codegen(params), {}, false);
    auto* Func = llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage, ident, &params->TheModule);
    auto* BB = llvm::BasicBlock::Create(params->TheContext, "entry", Func);

    params->Builder.SetInsertPoint(BB);
    
    block->Codegen(params);

    return nullptr;
}

koopa_raw_function_t FuncDefAST::ToKoopa(KoopaEnv* env)  {
    auto* type = env->CreateFuncType(funcType->ToKoopa(env));
    env->CreateFunction(type, "@" + ident);
    env->CreateBasicBlock("%entry");

    block->ToKoopa(env);

    return nullptr;
}

llvm::Value* BlockAST::Codegen(LLVMEnv* params) {
    params->symtab.EnterScope();
    for (auto& block : items)
        block->Codegen(params);
    params->symtab.ExitScope();

    return nullptr;  // Block does not return a value
}

void* BlockAST::ToKoopa(KoopaEnv* env) {
    env->EnterScope();
    for (auto& item : items)
        item->ToKoopa(env);
    env->ExitScope();
    
    return nullptr;  // Block does not return a value
}

llvm::Value* BlockItemAST::Codegen(LLVMEnv* params) {
    return ast->Codegen(params);
}

void* BlockItemAST::ToKoopa(KoopaEnv* env) {
    return ast->ToKoopa(env);
}

llvm::Value* StmtAST::Codegen(LLVMEnv* params) {
    if (type == Type::Assign) {
        params->CreateStore(expr2->Codegen(params), expr1->Codegen(params));
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

void* StmtAST::ToKoopa(KoopaEnv* env) {
    if (type == Type::Assign) {
        env->CreateStore(expr2->ToKoopa(env), expr1->ToKoopa(env));
    } else if (type == Type::Expr) {
        return expr1->ToKoopa(env);
    } else if (type == Type::Block) {
        return expr1->ToKoopa(env);
    } else if (type == Type::If) {
        return nullptr;
    } else if (type == Type::Ret) {
        env->CreateRet(expr1->ToKoopa(env));
    }
    return nullptr;
}

llvm::Value* ExprAST::Codegen(LLVMEnv* params) {
    return expr->Codegen(params);
}

void* ExprAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}

llvm::Value* PrimaryExprAST::Codegen(LLVMEnv* params) {
    if (type == Type::Expr) {
        return ast->Codegen(params);
    } else if (type == Type::LVal) {
        auto* addr = ast->Codegen(params);
        auto* type = params->symtab.GetSymbolType(addr);
        return (llvm::Value*)params->CreateLoad(addr);
    } else if (type == Type::Number) {
        return ast->Codegen(params);
    }
    return nullptr;  // Should not reach here
}

void* PrimaryExprAST::ToKoopa(KoopaEnv* env) {
    if (type == Type::Expr) {
        return ast->ToKoopa(env);
    } else if (type == Type::LVal) {
        auto* val = ast->ToKoopa(env);
        auto symbol_type = env->GetSymbolType(val);
        if (symbol_type == VAR_TYPE::VAR) {
            return env->CreateLoad(val);
        } else if (symbol_type == VAR_TYPE::CONST) {
            return val;
        }
    } else if (type == Type::Number) {
        return ast->ToKoopa(env);
    }
    return nullptr;  // Should not reach here
}

llvm::Value* NumberAST::Codegen(LLVMEnv* params) {
    return llvm::ConstantInt::get(params->TheContext, llvm::APInt(32, value, true));
}

void* NumberAST::ToKoopa(KoopaEnv* env)  {
    return new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_INTEGER,
            .data.integer = {
                .value = value
            }
        }
    };
}

llvm::Value* UnaryExprAST::Codegen(LLVMEnv* params) {
    if (type == Type::Primary) {
        return expr->Codegen(params);
    } else if (type == Type::Unary) {
        if (op == "+") {
            return expr->Codegen(params);
        } else if (op == "-") {
            auto* exprVal = expr->Codegen(params);
            return params->Builder.CreateNeg(exprVal);
        } else if (op == "!") {
            auto* exprVal = expr->Codegen(params);
            auto* res = params->Builder.CreateICmpEQ(exprVal, llvm::ConstantInt::get(exprVal->getType(), 0));
            return params->Builder.CreateZExt(res, llvm::Type::getInt32Ty(params->TheContext));
        }
    }
    return nullptr;  // Should not reach here
}

void* UnaryExprAST::ToKoopa(KoopaEnv* env) {
    if (type == Type::Primary) {
        return expr->ToKoopa(env);
    } else if (type == Type::Unary) {
        if (op == "+") {
            return expr->ToKoopa(env);
        } else if (op == "-") {
            return env->CreateSub(NumberAST(0).ToKoopa(env), expr->ToKoopa(env));
        } else if (op == "!") {
            return env->CreateICmpEQ(expr->ToKoopa(env), NumberAST(0).ToKoopa(env));
        }
    }
    return nullptr;  // Should not reach here
}

llvm::Value* MulExprAST::Codegen(LLVMEnv* params) {
    if (expr2) {
        auto* leftVal = expr1->Codegen(params);
        auto* rightVal = expr2->Codegen(params);
        
        if (op == "*") {
            return params->Builder.CreateMul(leftVal, rightVal);
        } else if (op == "/") {
            return params->Builder.CreateSDiv(leftVal, rightVal);
        } else if (op == "%") {
            return params->Builder.CreateSRem(leftVal, rightVal);
        }
    } else {
        return expr1->Codegen(params);
    }
    return nullptr;  // Should not reach here
}

void* MulExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        if (op == "*") {
            return env->CreateMul(expr1->ToKoopa(env), expr2->ToKoopa(env));
        } else if (op == "/") {
            return env->CreateDiv(expr1->ToKoopa(env), expr2->ToKoopa(env));
        } else if (op == "%") {
            return env->CreateMod(expr1->ToKoopa(env), expr2->ToKoopa(env));
        }
    } else {
        return expr1->ToKoopa(env);
    }
    return nullptr;
}

llvm::Value* AddExprAST::Codegen(LLVMEnv* params) {
    if (expr2) {
        auto* leftVal = expr1->Codegen(params);
        auto* rightVal = expr2->Codegen(params);
        
        if (op == "+") {
            return params->Builder.CreateAdd(leftVal, rightVal);
        } else if (op == "-") {
            return params->Builder.CreateSub(leftVal, rightVal);
        }
    } else {
        return expr1->Codegen(params);
    }
    return nullptr;  // Should not reach here
}

void* AddExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        if (op == "+") {
            return env->CreateAdd(expr1->ToKoopa(env), expr2->ToKoopa(env));
        } else if (op == "-") {
            return env->CreateSub(expr1->ToKoopa(env), expr2->ToKoopa(env));
        }
    } else {
        return expr1->ToKoopa(env);
    }
    return nullptr;
}

llvm::Value* RelExprAST::Codegen(LLVMEnv* params) {
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

void* RelExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        if (op == Op::LT) {
            return env->CreateICmpLT(expr1->ToKoopa(env), expr2->ToKoopa(env));
        } else if (op == Op::GT) {
            return env->CreateICmpGT(expr1->ToKoopa(env), expr2->ToKoopa(env));
        } else if (op == Op::LE) {
            return env->CreateICmpLE(expr1->ToKoopa(env), expr2->ToKoopa(env));
        } else if (op == Op::GE) {
            return env->CreateICmpGE(expr1->ToKoopa(env), expr2->ToKoopa(env));
        }
    } else {
        return expr1->ToKoopa(env);
    }
    return nullptr;  // Should not reach here
}

llvm::Value* EqExprAST::Codegen(LLVMEnv* params) {
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

void* EqExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        if (op == Op::EQ) {
            return env->CreateICmpEQ(expr1->ToKoopa(env), expr2->ToKoopa(env));
        } else if (op == Op::NE) {
            return env->CreateICmpNE(expr1->ToKoopa(env), expr2->ToKoopa(env));
        }
    }
    return expr1->ToKoopa(env);
}

llvm::Value* LAndExprAST::Codegen(LLVMEnv* params) {
    if (expr2) {
        auto* leftVal = expr1->Codegen(params);
        auto* rightVal = expr2->Codegen(params);
        auto* res = params->Builder.CreateAnd(leftVal, rightVal);
        return params->Builder.CreateZExt(res, llvm::Type::getInt32Ty(params->TheContext));
    }
    return expr1->Codegen(params);
}

void* LAndExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        auto lg1 = env->CreateICmpNE(expr1->ToKoopa(env), NumberAST(0).ToKoopa(env));
        auto lg2 = env->CreateICmpNE(expr2->ToKoopa(env), NumberAST(0).ToKoopa(env));
        return env->CreateAnd(lg1, lg2);
    }
    return expr1->ToKoopa(env);
}

llvm::Value* LOrExprAST::Codegen(LLVMEnv* params) {
    if (expr2) {
        auto* leftVal = expr1->Codegen(params);
        auto* rightVal = expr2->Codegen(params);
        auto* res = params->Builder.CreateOr(leftVal, rightVal);
        return params->Builder.CreateZExt(res, llvm::Type::getInt32Ty(params->TheContext));
    }
    return expr1->Codegen(params);
}

void* LOrExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        auto lg1 = env->CreateICmpNE(expr1->ToKoopa(env), NumberAST(0).ToKoopa(env));
        auto lg2 = env->CreateICmpNE(expr2->ToKoopa(env), NumberAST(0).ToKoopa(env));
        return env->CreateOr(lg1, lg2);
    }
    return expr1->ToKoopa(env);
}

llvm::Value* DeclAST::Codegen(LLVMEnv* params) {
    return constDecl->Codegen(params);
}

void* DeclAST::ToKoopa(KoopaEnv* env) {
    return constDecl->ToKoopa(env);
}

llvm::Value* ConstDeclAST::Codegen(LLVMEnv* params) {
    for (const auto& def : constDef) {
        params->symtab.AddSymbol(def->ident, btype->Codegen(params), def->initVal->Codegen(params));
    }
    return nullptr;  // Const declarations do not return a value
}

void* ConstDeclAST::ToKoopa(KoopaEnv* env) {
    for (const auto& def : constDef) {
        env->AddSymbol(def->ident, VAR_TYPE::CONST, def->initVal->ToKoopa(env));
    }
    return nullptr;
}

llvm::Value* VarDeclAST::Codegen(LLVMEnv* params) {
    auto* type = btype->Codegen(params);
    for (const auto& def : localDef) {
        auto* varAddr = params->Builder.CreateAlloca(type, nullptr, def->ident);
        if (def->initVal)
            params->Builder.CreateStore(def->initVal->Codegen(params), varAddr);

        params->symtab.AddSymbol(def->ident, type, varAddr);
    }
    return nullptr;  // Const declarations do not return a value
}

void* VarDeclAST::ToKoopa(KoopaEnv* env) {
    for (const auto& def : localDef) {
        auto* varAddr = env->CreateAlloca(btype->ToKoopa(env), "@" + def->ident);

        if (def->initVal) {
            env->CreateStore(def->initVal->ToKoopa(env), varAddr);
        }

        env->AddSymbol(def->ident, VAR_TYPE::VAR, varAddr);
    }

    return nullptr;
}

// TODO ToValue
llvm::Value* ConstInitValAST::Codegen(LLVMEnv* params) {
    return expr->Codegen(params);
}

void* ConstInitValAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}

llvm::Value* InitValAST::Codegen(LLVMEnv* params) {
    return expr->Codegen(params);
}

void* InitValAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}

llvm::Value* LValAST::Codegen(LLVMEnv* params) {
    return params->symtab.GetSymbolValue(ident);
}

void* LValAST::ToKoopa(KoopaEnv* env) {
    return env->GetSymbolValue(ident);
}

llvm::Value* ConstExprAST::Codegen(LLVMEnv* params) {
    return expr->Codegen(params);
}

void* ConstExprAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}