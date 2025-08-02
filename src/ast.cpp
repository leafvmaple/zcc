#include "ast.h"
#include "llvm_ir.h"

void CompUnitAST::AddFuncDef(unique_ptr<BaseAST>&& funcDef) {
    this->funcDef.emplace_back(std::move(funcDef));
}

llvm::Value* CompUnitAST::Codegen(LLVMParams* params) {
    for (auto& func : funcDef) {
        func->Codegen(params);
    }
    return nullptr;
}

void* CompUnitAST::Parse(KoopaEnv* env) {
    return new koopa_raw_program_t {
        .values = slice(KOOPA_RSIK_VALUE),
        .funcs = slice(KOOPA_RSIK_FUNCTION, funcDef, env)
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

void* FuncDefAST::Parse(KoopaEnv* env)  {
    return new koopa_raw_function_data_t {
        .ty = new koopa_raw_type_kind {
            KOOPA_RTT_FUNCTION,
            .data.function = {
                .params = slice(KOOPA_RSIK_TYPE),
                .ret = (koopa_raw_type_t)(funcType->Parse(env))
            }
        },
        .name = c_string("@" + ident),
        .params = slice(KOOPA_RSIK_VALUE),
        .bbs = slice(KOOPA_RSIK_BASIC_BLOCK, block, env)
    };
}

llvm::Value* BlockAST::Codegen(LLVMParams* params) {
    params->symtab.EnterScope();
    for (auto& block : items)
        block->Codegen(params);
    params->symtab.ExitScope();

    return nullptr;  // Block does not return a value
}

void* BlockAST::Parse(KoopaEnv* env) {
    return new koopa_raw_basic_block_data_t {
        .name = "%entry",
        .params = slice(KOOPA_RSIK_VALUE),
        .used_by = slice(KOOPA_RSIK_VALUE),
        .insts = slice(KOOPA_RSIK_VALUE, items, env),
    };
}

llvm::Value* BlockItemAST::Codegen(LLVMParams* params) {
    return ast->Codegen(params);
}

void* BlockItemAST::Parse(KoopaEnv* env) {
    return ast->Parse(env);
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

void* StmtAST::Parse(KoopaEnv* env) {
    if (type == Type::Assign) {
        return nullptr;
    } else if (type == Type::Expr) {
        return expr1->Parse(env);
    } else if (type == Type::Block) {
        return expr1->Parse(env);
    } else if (type == Type::If) {
        return nullptr;
    } else if (type == Type::Ret) {
        return new koopa_raw_value_data_t {
            .ty = type_kind(KOOPA_RTT_UNIT),
            .name = nullptr,
            .used_by = slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_RETURN,
                .data.ret = {
                    .value = (koopa_raw_value_t)expr1->Parse(env)
                }
            }
        };
    }
    assert(false);
    return nullptr;
}

llvm::Value* ExprAST::Codegen(LLVMParams* params) {
    return expr->Codegen(params);
}

void* ExprAST::Parse(KoopaEnv* env) {
    return expr->Parse(env);
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

void* PrimaryExprAST::Parse(KoopaEnv* env) {
    if (type == Type::Expr) {
        return ast->Parse(env);
    } else if (type == Type::LVal) {
        return ast->Parse(env);
    } else if (type == Type::Number) {
        return ast->Parse(env);
    }
    return nullptr;  // Should not reach here
}

llvm::Value* NumberAST::Codegen(LLVMParams* params) {
    return llvm::ConstantInt::get(params->TheContext, llvm::APInt(32, value, true));
}

void* NumberAST::Parse(KoopaEnv* env)  {
    return new koopa_raw_value_data_t {
        .ty = type_kind(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = slice(KOOPA_RSIK_VALUE),
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

void* UnaryExprAST::Parse(KoopaEnv* env) {
    if (type == Type::Primary) {
        return expr->Parse(env);
    } else if (type == Type::Unary) {
        if (op == "+") {
            return expr->Parse(env);
        } else if (op == "-") {
            #if 0
            return new koopa_raw_value_data_t {
                .ty = type_kind(KOOPA_RTT_INT32),
                .name = nullptr,
                .used_by = slice(KOOPA_RSIK_VALUE),
                .kind = {
                    .tag = KOOPA_RVT_BINARY,
                    .data.binary = {
                        .op = KOOPA_RBO_SUB,
                        .lhs = (koopa_raw_value_t)NumberAST(0).Parse(env),
                        .rhs = (koopa_raw_value_t)expr->Parse(env)
                    }
                }
            };
            #endif
            return new koopa_raw_value_data_t {
                .ty = type_kind(KOOPA_RTT_INT32),
                .name = nullptr,
                .used_by = slice(KOOPA_RSIK_VALUE),
                .kind = {
                    .tag = KOOPA_RVT_INTEGER,
                    .data.integer = {
                        .value = -(((const koopa_raw_value_data*)expr->Parse(env))->kind.data.integer.value)
                    }
                }
            };
        } else if (op == "!") {
            #if 0
            return new koopa_raw_value_data_t {
                .ty = type_kind(KOOPA_RTT_INT32),
                .name = nullptr,
                .used_by = slice(KOOPA_RSIK_VALUE),
                .kind = {
                    .tag = KOOPA_RVT_BINARY,
                    .data.binary = {
                        .op = KOOPA_RBO_EQ,
                        .lhs = (koopa_raw_value_t)expr->Parse(env),
                        .rhs = (koopa_raw_value_t)NumberAST(0).Parse(env)
                    }
                }
            };
            #endif
            return new koopa_raw_value_data_t {
                .ty = type_kind(KOOPA_RTT_INT32),
                .name = nullptr,
                .used_by = slice(KOOPA_RSIK_VALUE),
                .kind = {
                    .tag = KOOPA_RVT_INTEGER,
                    .data.integer = {
                        .value = !(((const koopa_raw_value_data*)expr->Parse(env))->kind.data.integer.value)
                    }
                }
            };
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

void* MulExprAST::Parse(KoopaEnv* env) {
    if (expr2) {
        return nullptr;
    } else {
        return expr1->Parse(env);
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

void* AddExprAST::Parse(KoopaEnv* env) {
    if (expr2) {
        return nullptr;
    } else {
        return expr1->Parse(env);
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

void* RelExprAST::Parse(KoopaEnv* env) {
    if (expr2) {
        return nullptr;
    } else {
        return expr1->Parse(env);
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

void* EqExprAST::Parse(KoopaEnv* env) {
    if (expr2) {
        return nullptr;
    } else {
        return expr1->Parse(env);
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

void* LAndExprAST::Parse(KoopaEnv* env) {
    return expr1->Parse(env);
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

void* LOrExprAST::Parse(KoopaEnv* env) {
    return expr1->Parse(env);
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