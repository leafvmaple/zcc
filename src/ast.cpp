#include "ast.h"
#include "llvm_ir.h"

void CompUnitAST::AddFuncDef(unique_ptr<FuncDefAST>&& funcDef) {
    this->funcDef.emplace_back(std::move(funcDef));
}

void* CompUnitAST::Codegen(LLVMEnv* env) {
    for (auto& func : funcDef) {
        func->Codegen(env);
    }
    return nullptr;
}

void CompUnitAST::ToKoopa(KoopaEnv* env) {
    for (auto& func : funcDef) {
        func->ToKoopa(env);
    }
}

void* FuncDefAST::Codegen(LLVMEnv* env) {
    auto* type = env->CreateFuncType(funcType->Codegen(env));
    auto* func = env->CreateFunction(type, ident);
    auto* bb = env->CreateBasicBlock("entry", func);

    env->SetInserPointer(bb);

    block->Codegen(env);

    return nullptr;
}

koopa_raw_function_t FuncDefAST::ToKoopa(KoopaEnv* env)  {
    auto* type = env->CreateFuncType(funcType->ToKoopa(env));
    auto* func = env->CreateFunction(type, ident);
    auto* bb = env->CreateBasicBlock("entry", func);

    env->SetInserPointer(bb);

    block->ToKoopa(env);

    return nullptr;
}

void* BlockAST::Codegen(LLVMEnv* env) {
    env->EnterScope();
    for (auto& block : items)
        block->Codegen(env);
    env->ExitScope();

    return nullptr;  // Block does not return a value
}

void* BlockAST::ToKoopa(KoopaEnv* env) {
    env->EnterScope();
    for (auto& item : items)
        item->ToKoopa(env);
    env->ExitScope();
    
    return nullptr;  // Block does not return a value
}

void* BlockItemAST::Codegen(LLVMEnv* env) {
    return ast->Codegen(env);
}

void* BlockItemAST::ToKoopa(KoopaEnv* env) {
    return ast->ToKoopa(env);
}

void* StmtAST::Codegen(LLVMEnv* env) {
    if (type == Type::Assign) {
        env->CreateStore(expr2->Codegen(env), expr1->Codegen(env));
    } else if (type == Type::Expr) {
        if (expr1)
            expr1->Codegen(env);
    } else if (type == Type::Block) {
        expr1->Codegen(env);
    } else if (type == Type::If) {
        // auto* cond = env->CreateICmpNE(expr1->Codegen(env), env->Builder.getInt32(0));
        // auto* func = env->Builder.GetInsertBlock()->getParent();
        // auto* thenBB = env->CreateBasicBlock("then", func);
        // llvm::BasicBlock* endBB{};

        // if (expr3) {
        //     auto* elseBB = llvm::BasicBlock::Create(env->TheContext, "else", func);
        //     endBB = llvm::BasicBlock::Create(env->TheContext, "end", func);
        //     env->Builder.CreateCondBr(cond, thenBB, elseBB);

        //     env->Builder.SetInsertPoint(elseBB);
        //     expr3->Codegen(env);
        //     env->Builder.CreateBr(endBB);
        // }
        // else {
        //     endBB = llvm::BasicBlock::Create(env->TheContext, "end", func);
        //     env->Builder.CreateCondBr(cond, thenBB, endBB);
        // }
        
        // env->Builder.SetInsertPoint(thenBB);
        // expr2->Codegen(env);
        // env->Builder.CreateBr(endBB);

        // env->Builder.SetInsertPoint(endBB);
    } else if (type == Type::Ret) {
        env->CreateRet(expr1->Codegen(env));
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

void* ExprAST::Codegen(LLVMEnv* env) {
    return expr->Codegen(env);
}

void* ExprAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}

void* PrimaryExprAST::Codegen(LLVMEnv* env) {
    if (type == Type::Expr) {
        return ast->Codegen(env);
    } else if (type == Type::LVal) {
        auto* val = ast->Codegen(env);
        auto symbol_type = env->GetSymbolType(val);
        if (symbol_type == VAR_TYPE::VAR) {
            return env->CreateLoad(val);
        } else if (symbol_type == VAR_TYPE::CONST) {
            return val;
        }
    } else if (type == Type::Number) {
        return ast->Codegen(env);
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

void* NumberAST::Codegen(LLVMEnv* env) {
    return llvm::ConstantInt::get(env->TheContext, llvm::APInt(32, value, true));
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

void* UnaryExprAST::Codegen(LLVMEnv* env) {
    if (type == Type::Primary) {
        return expr->Codegen(env);
    } else if (type == Type::Unary) {
        if (op == "+") {
            return expr->Codegen(env);
        } else if (op == "-") {
            return env->CreateSub(NumberAST(0).Codegen(env), expr->Codegen(env));
        } else if (op == "!") {
            return env->CreateICmpEQ(expr->Codegen(env), NumberAST(0).Codegen(env));
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

void* MulExprAST::Codegen(LLVMEnv* env) {
    if (expr2) {
        if (op == "*") {
            return env->CreateMul(expr1->Codegen(env), expr2->Codegen(env));
        } else if (op == "/") {
            return env->CreateDiv(expr1->Codegen(env), expr2->Codegen(env));
        } else if (op == "%") {
            return env->CreateMod(expr1->Codegen(env), expr2->Codegen(env));
        }
    } else {
        return expr1->Codegen(env);
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

void* AddExprAST::Codegen(LLVMEnv* env) {
    if (expr2) {
        if (op == "+") {
            return env->CreateAdd(expr1->Codegen(env), expr2->Codegen(env));
        } else if (op == "-") {
            return env->CreateSub(expr1->Codegen(env), expr2->Codegen(env));
        }
    } else {
        return expr1->Codegen(env);
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

void* RelExprAST::Codegen(LLVMEnv* env) {
    if (expr2) {
        if (op == Op::LT) {
            return env->CreateICmpLT(expr1->Codegen(env), expr2->Codegen(env));
        } else if (op == Op::GT) {
            return env->CreateICmpGT(expr1->Codegen(env), expr2->Codegen(env));
        } else if (op == Op::LE) {
            return env->CreateICmpLE(expr1->Codegen(env), expr2->Codegen(env));
        } else if (op == Op::GE) {
            return env->CreateICmpGE(expr1->Codegen(env), expr2->Codegen(env));
        }

        // return env->Builder.CreateZExt(res, llvm::Type::getInt32Ty(env->TheContext));
    }
    return expr1->Codegen(env);
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

void* EqExprAST::Codegen(LLVMEnv* env) {
    if (expr2) {
        if (op == Op::EQ) {
            return env->CreateICmpEQ(expr1->Codegen(env), expr2->Codegen(env));
        } else if (op == Op::NE) {
            return env->CreateICmpNE(expr1->Codegen(env), expr2->Codegen(env));
        }

        // return env->Builder.CreateZExt(res, llvm::Type::getInt32Ty(env->TheContext));
    }
    return expr1->Codegen(env);
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

void* LAndExprAST::Codegen(LLVMEnv* env) {
    if (expr2) {
        auto lg1 = env->CreateICmpNE(expr1->Codegen(env), NumberAST(0).Codegen(env));
        auto lg2 = env->CreateICmpNE(expr2->Codegen(env), NumberAST(0).Codegen(env));
        return env->CreateAnd(lg1, lg2);
    }
    return expr1->Codegen(env);
}

void* LAndExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        auto lg1 = env->CreateICmpNE(expr1->ToKoopa(env), NumberAST(0).ToKoopa(env));
        auto lg2 = env->CreateICmpNE(expr2->ToKoopa(env), NumberAST(0).ToKoopa(env));
        return env->CreateAnd(lg1, lg2);
    }
    return expr1->ToKoopa(env);
}

void* LOrExprAST::Codegen(LLVMEnv* env) {
    if (expr2) {
        auto lg1 = env->CreateICmpNE(expr1->Codegen(env), NumberAST(0).Codegen(env));
        auto lg2 = env->CreateICmpNE(expr2->Codegen(env), NumberAST(0).Codegen(env));
        return env->CreateOr(lg1, lg2);
    }
    return expr1->Codegen(env);
}

void* LOrExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        auto lg1 = env->CreateICmpNE(expr1->ToKoopa(env), NumberAST(0).ToKoopa(env));
        auto lg2 = env->CreateICmpNE(expr2->ToKoopa(env), NumberAST(0).ToKoopa(env));
        return env->CreateOr(lg1, lg2);
    }
    return expr1->ToKoopa(env);
}

void* DeclAST::Codegen(LLVMEnv* env) {
    return constDecl->Codegen(env);
}

void* DeclAST::ToKoopa(KoopaEnv* env) {
    return constDecl->ToKoopa(env);
}

void* ConstDeclAST::Codegen(LLVMEnv* env) {
    for (const auto& def : constDef) {
        env->AddSymbol(def->ident, VAR_TYPE::CONST, def->initVal->Codegen(env));
    }
    return nullptr;  // Const declarations do not return a value
}

void* ConstDeclAST::ToKoopa(KoopaEnv* env) {
    for (const auto& def : constDef) {
        env->AddSymbol(def->ident, VAR_TYPE::CONST, def->initVal->ToKoopa(env));
    }
    return nullptr;
}

void* VarDeclAST::Codegen(LLVMEnv* env) {
    for (const auto& def : localDef) {
        auto* varAddr = env->CreateAlloca(btype->Codegen(env), def->ident);
        if (def->initVal)
            env->CreateStore(def->initVal->Codegen(env), varAddr);

        env->AddSymbol(def->ident, VAR_TYPE::VAR, varAddr);
    }
    return nullptr;  // Const declarations do not return a value
}

void* VarDeclAST::ToKoopa(KoopaEnv* env) {
    for (const auto& def : localDef) {
        auto* varAddr = env->CreateAlloca(btype->ToKoopa(env), def->ident);

        if (def->initVal) {
            env->CreateStore(def->initVal->ToKoopa(env), varAddr);
        }

        env->AddSymbol(def->ident, VAR_TYPE::VAR, varAddr);
    }

    return nullptr;
}

// TODO ToValue
void* ConstInitValAST::Codegen(LLVMEnv* env) {
    return expr->Codegen(env);
}

void* ConstInitValAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}

void* InitValAST::Codegen(LLVMEnv* env) {
    return expr->Codegen(env);
}

void* InitValAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}

void* LValAST::Codegen(LLVMEnv* env) {
    return env->GetSymbolValue(ident);
}

void* LValAST::ToKoopa(KoopaEnv* env) {
    return env->GetSymbolValue(ident);
}

void* ConstExprAST::Codegen(LLVMEnv* env) {
    return expr->Codegen(env);
}

void* ConstExprAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}