#include "ast.h"
#include "llvm_ir.h"

void CompUnitAST::AddFuncDef(unique_ptr<FuncDefAST>&& funcDef) {
    this->funcDef.emplace_back(std::move(funcDef));
}

llvm::Value* CompUnitAST::Codegen(LLVMParams* params) {
    for (auto& func : funcDef) {
        func->Codegen(params);
    }
    return nullptr;
}

koopa_raw_program_t CompUnitAST::ToKoopa(KoopaEnv* env) {
    return {
        .values = koopa_slice(KOOPA_RSIK_VALUE),
        .funcs = koopa_slice(KOOPA_RSIK_FUNCTION, funcDef, env)
    };
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

koopa_raw_function_t FuncDefAST::ToKoopa(KoopaEnv* env)  {
    env->create_function(funcType->ToKoopa(env), "@" + ident);
    env->create_basic_block("%entry");

    block->ToKoopa(env);

    return env->get_function();
}

llvm::Value* BlockAST::Codegen(LLVMParams* params) {
    params->symtab.EnterScope();
    for (auto& block : items)
        block->Codegen(params);
    params->symtab.ExitScope();

    return nullptr;  // Block does not return a value
}

koopa_raw_value_t BlockAST::ToKoopa(KoopaEnv* env) {
    env->enter_scope();
    for (auto& item : items)
        item->ToKoopa(env);
    env->exit_scope();
    
    return nullptr;  // Block does not return a value
}

llvm::Value* BlockItemAST::Codegen(LLVMParams* params) {
    return ast->Codegen(params);
}

koopa_raw_value_t BlockItemAST::ToKoopa(KoopaEnv* env) {
    return ast->ToKoopa(env);
}

llvm::Value* StmtAST::Codegen(LLVMParams* params) {
    if (type == Type::Assign) {
        params->Builder.CreateStore(expr2->Codegen(params), expr1->Codegen(params));
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

koopa_raw_value_t StmtAST::ToKoopa(KoopaEnv* env) {
    if (type == Type::Assign) {
        env->create_store(expr2->ToKoopa(env), expr1->ToKoopa(env));
    } else if (type == Type::Expr) {
        return expr1->ToKoopa(env);
    } else if (type == Type::Block) {
        return expr1->ToKoopa(env);
    } else if (type == Type::If) {
        return nullptr;
    } else if (type == Type::Ret) {
        env->create_ret(expr1->ToKoopa(env));
    }
    return nullptr;
}

llvm::Value* ExprAST::Codegen(LLVMParams* params) {
    return expr->Codegen(params);
}

koopa_raw_value_t ExprAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
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

koopa_raw_value_t PrimaryExprAST::ToKoopa(KoopaEnv* env) {
    if (type == Type::Expr) {
        return ast->ToKoopa(env);
    } else if (type == Type::LVal) {
        auto* val = ast->ToKoopa(env);
        auto symbol_type = env->get_symbol_type(val);
        if (symbol_type == SYMBOL::VAR) {
            return env->create_load(val);
        } else if (symbol_type == SYMBOL::CONST) {
            return val;
        }
    } else if (type == Type::Number) {
        return ast->ToKoopa(env);
    }
    return nullptr;  // Should not reach here
}

llvm::Value* NumberAST::Codegen(LLVMParams* params) {
    return llvm::ConstantInt::get(params->TheContext, llvm::APInt(32, value, true));
}

koopa_raw_value_t NumberAST::ToKoopa(KoopaEnv* env)  {
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

llvm::Value* UnaryExprAST::Codegen(LLVMParams* params) {
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

koopa_raw_value_t UnaryExprAST::ToKoopa(KoopaEnv* env) {
    if (type == Type::Primary) {
        return expr->ToKoopa(env);
    } else if (type == Type::Unary) {
        if (op == "+") {
            return expr->ToKoopa(env);
        } else if (op == "-") {
            return env->create_inst(new koopa_raw_value_data_t {
                .ty = koopa_type(KOOPA_RTT_INT32),
                .name = nullptr,
                .used_by = koopa_slice(KOOPA_RSIK_VALUE),
                .kind = {
                    .tag = KOOPA_RVT_BINARY,
                    .data.binary = {
                        .op = KOOPA_RBO_SUB,
                        .lhs = NumberAST(0).ToKoopa(env),
                        .rhs = expr->ToKoopa(env)
                    }
                }
            });
        } else if (op == "!") {
            return env->create_inst(new koopa_raw_value_data_t {
                .ty = koopa_type(KOOPA_RTT_INT32),
                .name = nullptr,
                .used_by = koopa_slice(KOOPA_RSIK_VALUE),
                .kind = {
                    .tag = KOOPA_RVT_BINARY,
                    .data.binary = {
                        .op = KOOPA_RBO_EQ,
                        .lhs = expr->ToKoopa(env),
                        .rhs = NumberAST(0).ToKoopa(env)
                    }
                }
            });
        }
    }
    return nullptr;  // Should not reach here
}

llvm::Value* MulExprAST::Codegen(LLVMParams* params) {
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

koopa_raw_value_t MulExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        return env->create_inst(new koopa_raw_value_data_t {
            .ty = koopa_type(KOOPA_RTT_INT32),
            .name = nullptr,
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_BINARY,
                .data.binary = {
                    .op = (op == "*") ? KOOPA_RBO_MUL :
                          (op == "/") ? KOOPA_RBO_DIV : KOOPA_RBO_MOD,
                    .lhs = expr1->ToKoopa(env),
                    .rhs = expr2->ToKoopa(env)
                }
            }
        });
    } else {
        return expr1->ToKoopa(env);
    }
}

llvm::Value* AddExprAST::Codegen(LLVMParams* params) {
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

koopa_raw_value_t AddExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        return env->create_inst(new koopa_raw_value_data_t {
            .ty = koopa_type(KOOPA_RTT_INT32),
            .name = nullptr,
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_BINARY,
                .data.binary = {
                    .op = (op == "+") ? KOOPA_RBO_ADD : KOOPA_RBO_SUB,
                    .lhs = expr1->ToKoopa(env),
                    .rhs = expr2->ToKoopa(env)
                }
            }
        });
    } else {
        return expr1->ToKoopa(env);
    }
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

koopa_raw_value_t RelExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        return env->create_inst(new koopa_raw_value_data_t {
            .ty = koopa_type(KOOPA_RTT_INT32),
            .name = nullptr,
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_BINARY,
                .data.binary = {
                    .op = (op == Op::LT) ? KOOPA_RBO_LT :
                          (op == Op::GT) ? KOOPA_RBO_GT :
                          (op == Op::LE) ? KOOPA_RBO_LE : KOOPA_RBO_GE,
                    .lhs = expr1->ToKoopa(env),
                    .rhs = expr2->ToKoopa(env)
                }
            }
        });
    } else {
        return expr1->ToKoopa(env);
    }
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

koopa_raw_value_t EqExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        return env->create_inst(new koopa_raw_value_data_t {
            .ty = koopa_type(KOOPA_RTT_INT32),
            .name = nullptr,
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_BINARY,
                .data.binary = {
                    .op = (op == Op::EQ) ? KOOPA_RBO_EQ : KOOPA_RBO_NOT_EQ,
                    .lhs = expr1->ToKoopa(env),
                    .rhs = expr2->ToKoopa(env)
                }
            }
        });
    } else {
        return expr1->ToKoopa(env);
    }
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

koopa_raw_value_t LAndExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        auto lg1 = env->create_inst(int2logic(expr1->ToKoopa(env)));
        auto lg2 = env->create_inst(int2logic(expr2->ToKoopa(env)));
        return env->create_inst(new koopa_raw_value_data_t {
            .ty = koopa_type(KOOPA_RTT_INT32),
            .name = nullptr,
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_BINARY,
                .data.binary = {
                    .op = KOOPA_RBO_AND,
                    .lhs = lg1,
                    .rhs = lg2
                }
            }
        });
    }
    return expr1->ToKoopa(env);
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

koopa_raw_value_t LOrExprAST::ToKoopa(KoopaEnv* env) {
    if (expr2) {
        auto lg1 = env->create_inst(int2logic(expr1->ToKoopa(env)));
        auto lg2 = env->create_inst(int2logic(expr2->ToKoopa(env)));
        return env->create_inst(new koopa_raw_value_data_t {
            .ty = koopa_type(KOOPA_RTT_INT32),
            .name = nullptr,
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_BINARY,
                .data.binary = {
                    .op = KOOPA_RBO_OR,
                    .lhs = lg1,
                    .rhs = lg2
                }
            }
        });
    }
    return expr1->ToKoopa(env);
}

llvm::Value* DeclAST::Codegen(LLVMParams* params) {
    return constDecl->Codegen(params);
}

koopa_raw_value_t DeclAST::ToKoopa(KoopaEnv* env) {
    return constDecl->ToKoopa(env);
}

llvm::Value* ConstDeclAST::Codegen(LLVMParams* params) {
    for (const auto& def : constDef) {
        params->symtab.AddSymbol(def->ident, btype->Codegen(params), def->initVal->Codegen(params));
    }
    return nullptr;  // Const declarations do not return a value
}

koopa_raw_value_t ConstDeclAST::ToKoopa(KoopaEnv* env) {
    for (const auto& def : constDef) {
        env->add_symbol(def->ident, SYMBOL::CONST, def->initVal->ToKoopa(env));
    }
    return nullptr;
}

llvm::Value* VarDeclAST::Codegen(LLVMParams* params) {
    auto* type = btype->Codegen(params);
    for (const auto& def : localDef) {
        auto* varAddr = params->Builder.CreateAlloca(type, nullptr, def->ident);
        if (def->initVal)
            params->Builder.CreateStore(def->initVal->Codegen(params), varAddr);

        params->symtab.AddSymbol(def->ident, type, varAddr);
    }
    return nullptr;  // Const declarations do not return a value
}

koopa_raw_value_t VarDeclAST::ToKoopa(KoopaEnv* env) {
    for (const auto& def : localDef) {
        auto* varAddr = env->create_inst(new koopa_raw_value_data_t {
            .ty = btype->ToKoopa(env),
            .name = to_string("@" + def->ident),
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_ALLOC,
            }
        });

        if (def->initVal) {
            env->create_inst(new koopa_raw_value_data_t {
                .ty = btype->ToKoopa(env),
                .name = nullptr,
                .used_by = koopa_slice(KOOPA_RSIK_VALUE),
                .kind = {
                    .tag = KOOPA_RVT_STORE,
                    .data.store = {
                        .value = def->initVal->ToKoopa(env),
                        .dest = varAddr
                    }
                }
            });
        }

        env->add_symbol(def->ident, SYMBOL::VAR, varAddr);
    }

    return nullptr;
}

// TODO ToValue
llvm::Value* ConstInitValAST::Codegen(LLVMParams* params) {
    return expr->Codegen(params);
}

koopa_raw_value_t ConstInitValAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}

llvm::Value* InitValAST::Codegen(LLVMParams* params) {
    return expr->Codegen(params);
}

koopa_raw_value_t InitValAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}

llvm::Value* LValAST::Codegen(LLVMParams* params) {
    return params->symtab.GetSymbolValue(ident);
}

koopa_raw_value_t LValAST::ToKoopa(KoopaEnv* env) {
    return env->get_symbol_value(ident);
}

llvm::Value* ConstExprAST::Codegen(LLVMParams* params) {
    return expr->Codegen(params);
}

koopa_raw_value_t ConstExprAST::ToKoopa(KoopaEnv* env) {
    return expr->ToKoopa(env);
}