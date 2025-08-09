#pragma once

#include "ast/ast.h"
#include "ir/ir.h"

template<typename Type, typename Value, typename BasicBlock, typename Function>
class Generator {
public:
    using EnvType = Env<Type, Value, BasicBlock, Function>;
    Generator(EnvType *env) : env(env) {};

    void Generate(CompUnitAST& ast) {
        auto* intType = env->GetInt32Type();
        auto* voidType = env->GetVoidType();
        auto* pointType = env->GetPointerType(intType);

        env->CreateBuiltin("getint", intType, {});
        env->CreateBuiltin("getch", intType, {});
        env->CreateBuiltin("getarray", intType, {pointType});
        env->CreateBuiltin("putint", voidType, {intType});
        env->CreateBuiltin("putch", voidType, {intType});
        env->CreateBuiltin("putarray", voidType, {intType, pointType});
        env->CreateBuiltin("starttiem", voidType, {});
        env->CreateBuiltin("stoptime", voidType, {});

        for (auto&& decl : ast.decls) {
            Generate(decl.get());
        }
        for (auto&& funcDef : ast.funcDefs) {
            Generate(funcDef.get());
        }
    }
    void Generate(FuncDefAST* funcDef) {
        std::vector<Type*> types;
        std::vector<std::string> names;
        
        for (auto& param : funcDef->params) {
            types.push_back(param->btype->Codegen(env));
            names.push_back(param->ident);
        }

        auto* type = env->CreateFuncType(funcDef->funcType->Codegen(env), types);
        auto* func = env->CreateFunction(type, funcDef->ident, names);
        auto* bb = env->CreateBasicBlock("entry", func);        

        env->SetInserPointer(bb);

        env->EnterScope();

        for (size_t i = 0; i < funcDef->params.size(); ++i) {
            env->CreateStore(env->GetFunctionArg(i), Generate(funcDef->params[i].get()));
        }

        Generate(funcDef->block.get());

        env->ExitScope();
    }
    void Generate(BlockAST* block) {
        env->EnterScope();
        for (auto&& item : block->items)
            Generate(item.get());
        env->ExitScope();
    }
    void Generate(BlockItemAST* item) {
        if (item->decl) {
            Generate(item->decl.get());
        } else if (item->stmt) {
            Generate(item->stmt.get());
        }
    }
    void Generate(DeclAST* decl) {
        if (decl->constDecl) {
            Generate(decl->constDecl.get());
        } else if (decl->varDecl) {
            Generate(decl->varDecl.get());
        }
    }
    void Generate(StmtAST* stmt) {
        if (stmt->type == StmtAST::Type::Assign) {
            env->CreateStore(Generate(stmt->expr.get()), Generate(stmt->lval.get()));
        } else if (stmt->type == StmtAST::Type::Expr) {
            if (stmt->expr)
                Generate(stmt->expr.get());
        } else if (stmt->type == StmtAST::Type::Block) {
            Generate(stmt->block.get());
        } else if (stmt->type == StmtAST::Type::If) {
            auto* cond = Generate(stmt->cond.get());
            auto* func = env->GetFunction();
            auto* thenBB = env->CreateBasicBlock("then", func);
            BasicBlock* endBB{};

            if (stmt->elseStmt) {
                auto* elseBB = env->CreateBasicBlock("else", func);
                endBB = env->CreateBasicBlock("end", func);
                env->CreateCondBr(cond, thenBB, elseBB);

                env->SetInserPointer(elseBB);
                Generate(stmt->elseStmt.get());
                env->CreateBr(endBB);
            } else {
                endBB = env->CreateBasicBlock("end", func);
                env->CreateCondBr(cond, thenBB, endBB);
            }
            
            env->SetInserPointer(thenBB);
            Generate(stmt->thenStmt.get());
            env->CreateBr(endBB);

            env->SetInserPointer(endBB);

        } else if (stmt->type == StmtAST::Type::Ret) {
            env->CreateRet(Generate(stmt->expr.get()));
        } else if (stmt->type == StmtAST::Type::While) {
            auto* func = env->GetFunction();
            auto* condBB = env->CreateBasicBlock("while_entry", func);
            auto* bodyBB = env->CreateBasicBlock("while_body", func);
            auto* endBB = env->CreateBasicBlock("end", func);

            env->EnterWhile(condBB, endBB);

            env->CreateBr(condBB);
            env->SetInserPointer(condBB);

            env->CreateCondBr(Generate(stmt->cond.get()), bodyBB, endBB);

            env->SetInserPointer(bodyBB);
            Generate(stmt->thenStmt.get());
            env->CreateBr(condBB);

            env->SetInserPointer(endBB);

            env->ExitWhile();
        } else if (stmt->type == StmtAST::Type::Break) {
            env->CreateBr(env->GetWhileEnd());
        } else if (stmt->type == StmtAST::Type::Continue) {
            env->CreateBr(env->GetWhileEntry());
        }
    }
    Value* Generate(ExprAST* expr) {
        return Generate(expr->lorExpr.get());
    }
    Value* Generate(PrimaryExprAST* primary) {
        if (primary->type == PrimaryExprAST::Type::Expr) {
            return Generate(primary->expr.get());
        } else if (primary->type == PrimaryExprAST::Type::LVal) {
            auto val = Generate(primary->lval.get());
            auto symbol_type = env->GetSymbolType({ .value = val });

            if (symbol_type == VAR_TYPE::VAR) {
                return env->CreateLoad(val);
            } else if (symbol_type == VAR_TYPE::CONST) {
                return val;
            }
        } else if (primary->type == PrimaryExprAST::Type::Number) {
            return Generate(primary->value.get());
        }
        return nullptr;  // Should not reach here
    }
    Value* Generate(NumberAST* number) {
        return env->GetInt32(number->value);
    }
    Value* Generate(LValAST* lval) {
        return env->GetSymbolValue(lval->ident).value;
    }
    Value* Generate(LOrExprAST* lorExpr) {
        if (lorExpr->left) {
            auto lg1 = env->CreateICmpNE(Generate(lorExpr->left.get()), env->GetInt32(0));
            auto lg2 = env->CreateICmpNE(Generate(lorExpr->right.get()), env->GetInt32(0));

            return env->CreateOr(lg1, lg2);
        }
        return Generate(lorExpr->landExpr.get());
    }
    Value* Generate(LAndExprAST* landExpr) {
        if (landExpr->left) {
            auto lg1 = env->CreateICmpNE(Generate(landExpr->left.get()), env->GetInt32(0));
            auto lg2 = env->CreateICmpNE(Generate(landExpr->right.get()), env->GetInt32(0));

            return env->CreateAnd(lg1, lg2);
        }
        return Generate(landExpr->eqExpr.get());
    }
    Value* Generate(EqExprAST* eqExpr) {
        if (eqExpr->left) {
            auto left = Generate(eqExpr->left.get());
            auto right = Generate(eqExpr->right.get());
            if (eqExpr->op == EqExprAST::Op::EQ) {
                return env->CreateICmpEQ(left, right);
            } else if (eqExpr->op == EqExprAST::Op::NE) {
                return env->CreateICmpNE(left, right);
            }
        }
        return Generate(eqExpr->relExpr.get());
    }
    Value* Generate(RelExprAST* relExpr) {
        if (relExpr->left) {
            auto left = Generate(relExpr->left.get());
            auto right = Generate(relExpr->right.get());
            if (relExpr->op == RelExprAST::Op::LT) {
                return env->CreateICmpLT(left, right);
            } else if (relExpr->op == RelExprAST::Op::GT) {
                return env->CreateICmpGT(left, right);
            } else if (relExpr->op == RelExprAST::Op::LE) {
                return env->CreateICmpLE(left, right);
            } else if (relExpr->op == RelExprAST::Op::GE) {
                return env->CreateICmpGE(left, right);
            }
        }
        return Generate(relExpr->addExpr.get());
    }
    Value* Generate(AddExprAST* addExpr) {
        if (addExpr->left) {
            auto left = Generate(addExpr->left.get());
            auto right = Generate(addExpr->right.get());
            if (addExpr->op == "+") {
                return env->CreateAdd(left, right);
            } else if (addExpr->op == "-") {
                return env->CreateSub(left, right);
            }
        }
        return Generate(addExpr->mulExpr.get());
    }
    Value* Generate(MulExprAST* mulExpr) {
        if (mulExpr->left) {
            auto left = Generate(mulExpr->left.get());
            auto right = Generate(mulExpr->right.get());
            if (mulExpr->op == "*") {
                return env->CreateMul(left, right);
            } else if (mulExpr->op == "/") {
                return env->CreateDiv(left, right);
            } else if (mulExpr->op == "%") {
                return env->CreateMod(left, right);
            }
        }
        return Generate(mulExpr->unaryExpr.get());
    }
    Value* Generate(UnaryExprAST* unaryExpr) {
        if (unaryExpr->type == UnaryExprAST::Type::Primary) {
            return Generate(unaryExpr->primaryExpr.get());
        } else if (unaryExpr->type == UnaryExprAST::Type::Unary) {
            auto* expr = Generate(unaryExpr->unaryExpr.get());
            if (unaryExpr->op == "+") {
                return expr;
            } else if (unaryExpr->op == "-") {
                return env->CreateSub(env->GetInt32(0), expr);
            } else if (unaryExpr->op == "!") {
                return env->CreateICmpEQ(expr, env->GetInt32(0));
            }
        } else if (unaryExpr->type == UnaryExprAST::Type::Call) {
            std::vector<Value*> args;
            for (auto& arg : unaryExpr->callArgs) {
                args.push_back(Generate(arg.get()));
            }
            auto symbol = env->GetSymbolValue(unaryExpr->ident);
            return env->CreateCall(symbol.function, args);
        }
        return nullptr;
    }
    Value* Generate(FuncFParamAST* param) {
        auto addr = env->CreateAlloca(param->btype->Codegen(env), param->ident);
        env->AddSymbol(param->ident, VAR_TYPE::VAR, { .value = addr });
        return addr;  // Return the address of the parameter
    }
    void Generate(ConstDeclAST* constDecl) {
        for (auto& def : constDecl->constDefs) {
            env->AddSymbol(def->ident, VAR_TYPE::CONST, { .value = Calculate(def->constInitVal.get()) });
        }
    }
    void Generate(VarDeclAST* varDecl) {
        Value* varAddr{};
        auto* type = varDecl->btype->Codegen(env);
        for (const auto& def : varDecl->varDefs) {
            if (env->IsGlobalScope()) {
                auto* initVal = def->initVal ? Calculate(def->initVal.get()) : env->CreateZero(type);
                varAddr = env->CreateGlobal(type, def->ident, initVal);
            } else {
                varAddr = env->CreateAlloca(type, def->ident);
                if (def->initVal)
                    env->CreateStore(Generate(def->initVal.get()), varAddr);
            }
            env->AddSymbol(def->ident, VAR_TYPE::VAR, { .value = varAddr });
        }
    }
    Value* Generate(ConstInitValAST* initVal) {
        return Generate(initVal->constExpr.get());
    }
    Value* Generate(InitValAST* initVal) {
        return Generate(initVal->constExpr.get());
    }
    Value* Generate(ConstExprAST* constExpr) {
        return Generate(constExpr->expr.get());
    }
    // Get NumberAST
    Value* Calculate(ConstInitValAST* initVal) {
        return Calculate(initVal->constExpr.get());
    }
    // Get NumberAST
    Value* Calculate(InitValAST* initVal) {
        return Calculate(initVal->constExpr.get());
    }
    Value* Calculate(ConstExprAST* constExpr) {
        return Calculate(constExpr->expr.get());
    }
    Value* Calculate(ExprAST* expr) {
        return Calculate(expr->lorExpr.get());
    }
    Value* Calculate(LOrExprAST* lorExpr) {
        if (lorExpr->left) {
            auto left = Calculate(lorExpr->left.get());
            auto right = Calculate(lorExpr->right.get());
            return env->CaculateBinaryOp([](int a, int b) { return a || b; }, left, right);
        }
        return Calculate(lorExpr->landExpr.get());
    }
    Value* Calculate(LAndExprAST* landExpr) {
        if (landExpr->left) {
            auto left = Calculate(landExpr->left.get());
            auto right = Calculate(landExpr->right.get());
            return env->CaculateBinaryOp([](int a, int b) { return a && b; }, left, right);
        }
        return Calculate(landExpr->eqExpr.get());
    }
    Value* Calculate(EqExprAST* eqExpr) {
        if (eqExpr->left) {
            auto left = Calculate(eqExpr->left.get());
            auto right = Calculate(eqExpr->right.get());
            if (eqExpr->op == EqExprAST::Op::EQ) {
                return env->CaculateBinaryOp([](int a, int b) { return a == b; }, left, right);
            } else if (eqExpr->op == EqExprAST::Op::NE) {
                return env->CaculateBinaryOp([](int a, int b) { return a != b; }, left, right);
            }
        }
        return Calculate(eqExpr->relExpr.get());
    }
    Value* Calculate(RelExprAST* relExpr) {
        if (relExpr->left) {
            auto left = Calculate(relExpr->left.get());
            auto right = Calculate(relExpr->right.get());
            if (relExpr->op == RelExprAST::Op::LT) {
                return env->CaculateBinaryOp([](int a, int b) { return a < b; }, left, right);
            } else if (relExpr->op == RelExprAST::Op::GT) {
                return env->CaculateBinaryOp([](int a, int b) { return a > b; }, left, right);
            } else if (relExpr->op == RelExprAST::Op::LE) {
                return env->CaculateBinaryOp([](int a, int b) { return a <= b; }, left, right);
            } else if (relExpr->op == RelExprAST::Op::GE) {
                return env->CaculateBinaryOp([](int a, int b) { return a >= b; }, left, right);
            }
        }
        return Calculate(relExpr->addExpr.get());
    }
    Value* Calculate(AddExprAST* addExpr) {
        if (addExpr->left) {
            auto left = Calculate(addExpr->left.get());
            auto right = Calculate(addExpr->right.get());
            if (addExpr->op == "+") {
                return env->CaculateBinaryOp([](int a, int b) { return a + b; }, left, right);
            } else if (addExpr->op == "-") {
                return env->CaculateBinaryOp([](int a, int b) { return a - b; }, left, right);
            }
        }
        return Calculate(addExpr->mulExpr.get());
    }
    Value* Calculate(MulExprAST* mulExpr) {
        if (mulExpr->left) {
            auto left = Calculate(mulExpr->left.get());
            auto right = Calculate(mulExpr->right.get());
            if (mulExpr->op == "*") {
                return env->CaculateBinaryOp([](int a, int b) { return a * b; }, left, right);
            } else if (mulExpr->op == "/") {
                return env->CaculateBinaryOp([](int a, int b) { return a / b; }, left, right);
            } else if (mulExpr->op == "%") {
                return env->CaculateBinaryOp([](int a, int b) { return a % b; }, left, right);
            }
        }
        return Calculate(mulExpr->unaryExpr.get());
    }
    Value* Calculate(UnaryExprAST* unaryExpr) {
        if (unaryExpr->type == UnaryExprAST::Type::Primary) {
            return Calculate(unaryExpr->primaryExpr.get());
        } else if (unaryExpr->type == UnaryExprAST::Type::Unary) {
            auto* expr = Calculate(unaryExpr->unaryExpr.get());
            if (unaryExpr->op == "+") {
                return expr;
            } else if (unaryExpr->op == "-") {
                return env->CaculateBinaryOp([](int a, int b) { return b - a; }, env->GetInt32(0), expr);
            } else if (unaryExpr->op == "!") {
                return env->CaculateBinaryOp([](int a, int b) { return a == 0; }, expr, env->GetInt32(0));
            }
        } else if (unaryExpr->type == UnaryExprAST::Type::Call) {
            assert(false);
        }
        return nullptr;  // Should not reach here
    }
    Value* Calculate(PrimaryExprAST* primary) {
        if (primary->type == PrimaryExprAST::Type::Expr) {
            return Calculate(primary->expr.get());
        } else if (primary->type == PrimaryExprAST::Type::LVal) {
            auto val = env->GetSymbolValue(primary->lval->ident);
            return val.value;
        } else if (primary->type == PrimaryExprAST::Type::Number) {
            return env->GetInt32(primary->value->value);
        }
        return nullptr;  // Should not reach here
    }

private:
    EnvType* env;
};