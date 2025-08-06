#include "ir/ir.h"

#include "ast.h"

void CompUnitAST::AddFuncDef(unique_ptr<FuncDefAST>&& funcDef) {
    this->funcDef.emplace_back(std::move(funcDef));
}

void CompUnitAST::Codegen(Env* env) {
    for (auto& func : funcDef)
        func->Codegen(env);
}

void FuncDefAST::Codegen(Env* env) {
    auto* type = env->CreateFuncType(funcType->Codegen(env));
    auto* func = env->CreateFunction(type, ident);
    auto* bb = env->CreateBasicBlock("entry", func);

    env->SetInserPointer(bb);

    block->Codegen(env);
}

void* BlockAST::Codegen(Env* env) {
    env->EnterScope();
    for (auto& block : items)
        block->Codegen(env);
    env->ExitScope();

    return nullptr;  // Block does not return a value
}

void* BlockItemAST::Codegen(Env* env) {
    return ast->Codegen(env);
}

void* StmtAST::Codegen(Env* env) {
    if (type == Type::Assign) {
        env->CreateStore(expr2->Codegen(env), expr1->Codegen(env));
    } else if (type == Type::Expr) {
        if (expr1)
            expr1->Codegen(env);
    } else if (type == Type::Block) {
        expr1->Codegen(env);
    } else if (type == Type::If) {
        auto* cond = expr1->Codegen(env);
        auto* func = env->GetFunction();
        auto* thenBB = env->CreateBasicBlock("then", func);
        void* endBB{};

        if (expr3) {
            auto* elseBB = env->CreateBasicBlock("else", func);
            endBB = env->CreateBasicBlock("end", func);
            env->CreateCondBr(cond, thenBB, elseBB);

            env->SetInserPointer(elseBB);
            expr3->Codegen(env);
            env->CreateBr(endBB);
        } else {
            endBB = env->CreateBasicBlock("end", func);
            env->CreateCondBr(cond, thenBB, endBB);
        }
        
        env->SetInserPointer(thenBB);
        expr2->Codegen(env);
        env->CreateBr(endBB);

        env->SetInserPointer(endBB);

    } else if (type == Type::Ret) {
        env->CreateRet(expr1->Codegen(env));
    } else if (type == Type::While) {
        void* func = env->GetFunction();
        void* condBB = env->CreateBasicBlock("while_entry", func);
        void* bodyBB = env->CreateBasicBlock("while_body", func);
        void* endBB = env->CreateBasicBlock("end", func);

        env->EnterWhile(condBB, endBB);

        env->CreateBr(condBB);
        env->SetInserPointer(condBB);

        env->CreateCondBr(expr1->Codegen(env), bodyBB, endBB);

        env->SetInserPointer(bodyBB);
        expr2->Codegen(env);
        env->CreateBr(condBB);

        env->SetInserPointer(endBB);

        env->ExitWhile();
    } else if (type == Type::Break) {
        env->CreateBr(env->GetWhileEnd());
    } else if (type == Type::Continue) {
        env->CreateBr(env->GetWhileEntry());
    }

    return nullptr;  // Assignment does not return a value
}

void* ExprAST::Codegen(Env* env) {
    return expr->Codegen(env);
}

void* PrimaryExprAST::Codegen(Env* env) {
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

void* NumberAST::Codegen(Env* env) {
    return env->GetInt32(value);
}

void* UnaryExprAST::Codegen(Env* env) {
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

void* MulExprAST::Codegen(Env* env) {
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

void* AddExprAST::Codegen(Env* env) {
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

void* RelExprAST::Codegen(Env* env) {
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
    }
    return expr1->Codegen(env);
}

void* EqExprAST::Codegen(Env* env) {
    if (expr2) {
        if (op == Op::EQ) {
            return env->CreateICmpEQ(expr1->Codegen(env), expr2->Codegen(env));
        } else if (op == Op::NE) {
            return env->CreateICmpNE(expr1->Codegen(env), expr2->Codegen(env));
        }
    }
    return expr1->Codegen(env);
}

void* LAndExprAST::Codegen(Env* env) {
    if (expr2) {
        auto lg1 = env->CreateICmpNE(expr1->Codegen(env), NumberAST(0).Codegen(env));
        auto lg2 = env->CreateICmpNE(expr2->Codegen(env), NumberAST(0).Codegen(env));
        return env->CreateAnd(lg1, lg2);
    }
    return expr1->Codegen(env);
}

void* LOrExprAST::Codegen(Env* env) {
    if (expr2) {
        auto lg1 = env->CreateICmpNE(expr1->Codegen(env), NumberAST(0).Codegen(env));
        auto lg2 = env->CreateICmpNE(expr2->Codegen(env), NumberAST(0).Codegen(env));
        return env->CreateOr(lg1, lg2);
    }
    return expr1->Codegen(env);
}

void* DeclAST::Codegen(Env* env) {
    return constDecl->Codegen(env);
}

void* ConstDeclAST::Codegen(Env* env) {
    for (const auto& def : constDef) {
        env->AddSymbol(def->ident, VAR_TYPE::CONST, def->initVal->Codegen(env));
    }
    return nullptr;  // Const declarations do not return a value
}

void* VarDeclAST::Codegen(Env* env) {
    for (const auto& def : localDef) {
        auto* varAddr = env->CreateAlloca(btype->Codegen(env), def->ident);

        if (def->initVal)
            env->CreateStore(def->initVal->Codegen(env), varAddr);

        env->AddSymbol(def->ident, VAR_TYPE::VAR, varAddr);
    }
    return nullptr;  // Const declarations do not return a value
}

// TODO ToValue
void* ConstInitValAST::Codegen(Env* env) {
    return expr->Codegen(env);
}

void* InitValAST::Codegen(Env* env) {
    return expr->Codegen(env);
}

void* LValAST::Codegen(Env* env) {
    return env->GetSymbolValue(ident);
}

void* ConstExprAST::Codegen(Env* env) {
    return expr->Codegen(env);
}
