#include "ast/ast.h"

#include "../libkoopa/include/function.h"
#include "llvm/IR/Function.h"

ConstDefAST::ConstDefAST(string ident, unique_ptr<ConstInitValAST>&& constInitVal)
    : ident(std::move(ident)), constInitVal(std::move(constInitVal)) {}

ConstDefAST::ConstDefAST(string ident, unique_ptr<ConstExprAST>&& size, unique_ptr<ConstInitValAST>&& constInitVal)
    : ident(std::move(ident)), size(std::move(size)), constInitVal(std::move(constInitVal)) {}

VarDefAST::VarDefAST(string ident)
    : ident(std::move(ident)) {}

VarDefAST::VarDefAST(string ident, unique_ptr<ConstExprAST>&& size)
    : ident(std::move(ident)), size(std::move(size)) {}

VarDefAST::VarDefAST(string ident, unique_ptr<InitValAST>&& initVal)
    : ident(std::move(ident)), initVal(std::move(initVal)) {}

VarDefAST::VarDefAST(string ident, unique_ptr<ConstExprAST>&& size, unique_ptr<InitValAST>&& initVal)
    : ident(std::move(ident)), size(std::move(size)), initVal(std::move(initVal)) {}

void CompUnitAST::AddFuncDef(unique_ptr<FuncDefAST>&& funcDef) {
    funcDefs.emplace_back(std::move(funcDef));
}

void CompUnitAST::AddDecl(unique_ptr<DeclAST>&& decl) {
    decls.emplace_back(std::move(decl));
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void CompUnitAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    auto* intType = env->GetInt32Type();
    auto* voidType = env->GetVoidType();
    auto* pointType = env->GetPointerType(intType);

    env->CreateBuiltin("getint", intType, {});
    env->CreateBuiltin("getch", intType, {});
    env->CreateBuiltin("getarray", intType, {pointType});
    env->CreateBuiltin("putint", voidType, {intType});
    env->CreateBuiltin("putch", voidType, {intType});
    env->CreateBuiltin("putarray", voidType, {intType, pointType});
    env->CreateBuiltin("starttime", voidType, {});
    env->CreateBuiltin("stoptime", voidType, {});

    for (auto&& decl : decls) {
        decl->Codegen(env);
    }

    for (auto&& funcDef : funcDefs) {
        funcDef->Codegen(env);
    }
}

FuncDefAST::FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BlockAST>&& block)
    : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}

FuncDefAST::FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, vector<unique_ptr<FuncFParamAST>>&& params, unique_ptr<BlockAST>&& block)
    : funcType(std::move(funcType)), ident(std::move(ident)), params(std::move(params)), block(std::move(block)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void FuncDefAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    std::vector<Type*> paramTypes;
    std::vector<std::string> paramNames;
    
    for (auto& param : params) {
        paramTypes.push_back(param->btype->Codegen(env));
        paramNames.push_back(param->ident);
    }

    auto* funcType = env->CreateFuncType(this->funcType->Codegen(env), paramTypes);
    auto* func = env->CreateFunction(funcType, ident, paramNames);
    auto* entryBB = env->CreateBasicBlock("entry", func);
    env->SetInserPointer(entryBB);

    env->EnterScope();

    for (size_t i = 0; i < params.size(); ++i) {
        env->CreateStore(env->GetFunctionArg(i), params[i]->Codegen(env));
    }

    block->Codegen(env);

    env->ExitScope();
}

BlockAST::BlockAST(vector<unique_ptr<BlockItemAST>>&& items) : items(std::move(items)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void BlockAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    env->EnterScope();
    for (auto&& item : items) {
        item->Codegen(env);
    }
    env->ExitScope();
}

StmtAST::StmtAST(TYPE type) : type(type) {}

StmtAST::StmtAST(TYPE type, unique_ptr<ExprAST>&& expr)
    : type(type), expr(std::move(expr)) {}

StmtAST::StmtAST(TYPE type, unique_ptr<BlockAST>&& block)
    : type(type), block(std::move(block)) {}

StmtAST::StmtAST(TYPE type, unique_ptr<LValAST>&& lval, unique_ptr<ExprAST>&& expr) 
    : type(type), lval(std::move(lval)), expr(std::move(expr)) {}

StmtAST::StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt) 
    : type(type), cond(std::move(cond)), thenStmt(std::move(thenStmt)) {}

StmtAST::StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt, unique_ptr<StmtAST>&& elseStmt) 
    : type(type), cond(std::move(cond)), thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void StmtAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Assign: {
            auto* val = expr->Codegen(env);
            auto* addr = lval->Codegen(env);
            env->CreateStore(val, addr);
            break;
        }
        case TYPE::Expr: {
            if (expr) expr->Codegen(env);
            break;
        }
        case TYPE::Block: {
            block->Codegen(env);
            break;
        }
        case TYPE::If: {
            auto* condVal = cond->Codegen(env);
            auto* func = env->GetFunction();
            auto* thenBB = env->CreateBasicBlock("then", func);
            BasicBlock* endBB{};

            if (elseStmt) {
                auto* elseBB = env->CreateBasicBlock("else", func);
                endBB = env->CreateBasicBlock("if_end", func);
                env->CreateCondBr(condVal, thenBB, elseBB);

                env->SetInserPointer(elseBB);
                elseStmt->Codegen(env);
                env->CreateBr(endBB);
            } else {
                endBB = env->CreateBasicBlock("if_end", func);
                env->CreateCondBr(condVal, thenBB, endBB);
            }

            env->SetInserPointer(thenBB);
            thenStmt->Codegen(env);
            env->CreateBr(endBB);

            env->SetInserPointer(endBB);
            break;
        }
        case TYPE::Ret: {
            auto* retVal = expr->Codegen(env);
            env->CreateRet(retVal);
            break;
        }
        case TYPE::While: {
            auto* func = env->GetFunction();
            auto* condBB = env->CreateBasicBlock("while_cond", func);
            auto* bodyBB = env->CreateBasicBlock("while_body", func);
            auto* endBB = env->CreateBasicBlock("while_end", func);

            env->EnterWhile(condBB, endBB);
            env->CreateBr(condBB);

            env->SetInserPointer(condBB);
            auto* condVal = cond->Codegen(env);
            env->CreateCondBr(condVal, bodyBB, endBB);

            env->SetInserPointer(bodyBB);
            thenStmt->Codegen(env);
            env->CreateBr(condBB);

            env->SetInserPointer(endBB);
            env->ExitWhile();
            break;
        }
        case TYPE::Break: {
            env->CreateBr(env->GetWhileEnd());
            break;
        }
        case TYPE::Continue: {
            env->CreateBr(env->GetWhileEntry());
            break;
        }
    }
}

ExprAST::ExprAST(unique_ptr<LOrExprAST>&& lorExpr) : lorExpr(std::move(lorExpr)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    return lorExpr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    return lorExpr->Calculate(env);
}

PrimaryExprAST::PrimaryExprAST(TYPE type, unique_ptr<ExprAST>&& expr)
    : type(type), expr(std::move(expr)) {}

PrimaryExprAST::PrimaryExprAST(TYPE type, unique_ptr<LValAST>&& lval)
    : type(type), lval(std::move(lval)) {}

PrimaryExprAST::PrimaryExprAST(TYPE type, unique_ptr<NumberAST>&& value)
    : type(type), value(std::move(value)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* PrimaryExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Expr: return expr->Codegen(env);
        case TYPE::LVal: {
            auto* val = lval->Codegen(env);
            auto symbolType = env->GetSymbolType({.value = val});
            if (symbolType == VAR_TYPE::VAR) return env->CreateLoad(val);
            else return val;
        }
        case TYPE::Number: return value->Codegen(env);
    }
    return nullptr;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* PrimaryExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Expr: return expr->Calculate(env);
        case TYPE::LVal: {
            auto* val = lval->Codegen(env);
            auto symbolType = env->GetSymbolType({.value = val});
            if (symbolType == VAR_TYPE::VAR) return env->CreateLoad(val);
            else return val;
        }
        case TYPE::Number: return value->Codegen(env);
    }
    return nullptr;
}

NumberAST::NumberAST(int value) : value(value) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* NumberAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    return env->GetInt32(value);
}

UnaryExprAST::UnaryExprAST(TYPE type, unique_ptr<PrimaryExprAST>&& primaryExpr) 
    : type(type), primaryExpr(std::move(primaryExpr)) {}

UnaryExprAST::UnaryExprAST(TYPE type, OP op, unique_ptr<UnaryExprAST>&& unaryExpr) 
    : type(type), op(op), unaryExpr(std::move(unaryExpr)) {}

UnaryExprAST::UnaryExprAST(TYPE type, string ident)
    : type(type), ident(std::move(ident)) {}

UnaryExprAST::UnaryExprAST(TYPE type, string ident, vector<unique_ptr<ExprAST>>&& callArgs) 
    : type(type), ident(std::move(ident)), callArgs(std::move(callArgs)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* UnaryExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Primary:
            return primaryExpr->Codegen(env);
        case TYPE::Unary: {
            auto* exprVal = unaryExpr->Codegen(env);
            switch (op) {
                case OP::PLUS: return exprVal;
                case OP::MINUS: return env->CreateSub(env->GetInt32(0), exprVal);
                case OP::NOT: return env->CreateICmpEQ(exprVal, env->GetInt32(0));
            }
        }
        case TYPE::Call: {
            std::vector<Value*> args;
            for (auto& arg : callArgs) {
                args.push_back(arg->Codegen(env));
            }
            auto symbol = env->GetSymbolValue(ident);
            return env->CreateCall(symbol.function, args);
        }
    }
    return nullptr;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* UnaryExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Primary:
            return primaryExpr->Calculate(env);
        case TYPE::Unary: {
            auto* exprVal = unaryExpr->Calculate(env);
            switch (op) {
                case OP::PLUS: return exprVal;
                case OP::MINUS: return env->CaculateBinaryOp([](int a, int b) { return a - b; }, env->GetInt32(0), exprVal);
                case OP::NOT: return env->CaculateBinaryOp([](int a, int b) { return a == 0; }, exprVal, env->GetInt32(0));
            }
        }
        case TYPE::Call: {
            assert(false && "Call expressions should not be calculated directly");
        }
    }
    return nullptr;
}

MulExprAST::MulExprAST(unique_ptr<UnaryExprAST>&& unaryExpr)
    : unaryExpr(std::move(unaryExpr)) {}

MulExprAST::MulExprAST(OP op, unique_ptr<MulExprAST>&& left, unique_ptr<UnaryExprAST>&& right) 
    : op(op), left(std::move(left)), right(std::move(right)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* MulExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Codegen(env);
        auto* rightVal = right->Codegen(env);
        switch (op) {
            case OP::MUL: return env->CreateMul(leftVal, rightVal);
            case OP::DIV: return env->CreateDiv(leftVal, rightVal);
            case OP::MOD: return env->CreateMod(leftVal, rightVal);
        }
    }
    return unaryExpr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* MulExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Calculate(env);
        auto* rightVal = right->Calculate(env);
        switch (op) {
            case OP::MUL: return env->CaculateBinaryOp([](int a, int b) { return a * b; }, leftVal, rightVal);
            case OP::DIV: return env->CaculateBinaryOp([](int a, int b) { return a / b; }, leftVal, rightVal);
            case OP::MOD: return env->CaculateBinaryOp([](int a, int b) { return a % b; }, leftVal, rightVal);
        }
    }
    return unaryExpr->Calculate(env);
}

AddExprAST::AddExprAST(unique_ptr<MulExprAST>&& mulExpr)
    : mulExpr(std::move(mulExpr)) {}

AddExprAST::AddExprAST(OP op, unique_ptr<AddExprAST>&& left, unique_ptr<MulExprAST>&& right) 
    : op(op), left(std::move(left)), right(std::move(right)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* AddExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Codegen(env);
        auto* rightVal = right->Codegen(env);
        if (op == OP::ADD) return env->CreateAdd(leftVal, rightVal);
        else return env->CreateSub(leftVal, rightVal);
    }
    return mulExpr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* AddExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Calculate(env);
        auto* rightVal = right->Calculate(env);
        if (op == OP::ADD) {
            return env->CaculateBinaryOp([](int a, int b) { return a + b; }, leftVal, rightVal);
        } else {
            return env->CaculateBinaryOp([](int a, int b) { return a - b; }, leftVal, rightVal);
        }
    }
    return mulExpr->Calculate(env);
}

RelExprAST::RelExprAST(unique_ptr<AddExprAST>&& addExpr)
    : addExpr(std::move(addExpr)) {}

RelExprAST::RelExprAST(unique_ptr<RelExprAST>&& left, Op op, unique_ptr<AddExprAST>&& right) 
    : left(std::move(left)), op(op), right(std::move(right)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* RelExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Codegen(env);
        auto* rightVal = right->Codegen(env);
        switch (op) {
            case Op::LT: return env->CreateICmpLT(leftVal, rightVal);
            case Op::GT: return env->CreateICmpGT(leftVal, rightVal);
            case Op::LE: return env->CreateICmpLE(leftVal, rightVal);
            case Op::GE: return env->CreateICmpGE(leftVal, rightVal);
        }
    }
    return addExpr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* RelExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Calculate(env);
        auto* rightVal = right->Calculate(env);
        switch (op) {
            case Op::LT: return env->CaculateBinaryOp([](int a, int b) { return a < b; }, leftVal, rightVal);
            case Op::GT: return env->CaculateBinaryOp([](int a, int b) { return a > b; }, leftVal, rightVal);
            case Op::LE: return env->CaculateBinaryOp([](int a, int b) { return a <= b; }, leftVal, rightVal);
            case Op::GE: return env->CaculateBinaryOp([](int a, int b) { return a >= b; }, leftVal, rightVal);
        }
    }
    return addExpr->Calculate(env);
}

EqExprAST::EqExprAST(unique_ptr<RelExprAST>&& relExpr)
    : relExpr(std::move(relExpr)) {}

EqExprAST::EqExprAST(unique_ptr<EqExprAST>&& left, Op op, unique_ptr<RelExprAST>&& right) 
    : left(std::move(left)), op(op), right(std::move(right)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* EqExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Codegen(env);
        auto* rightVal = right->Codegen(env);
        if (op == Op::EQ) return env->CreateICmpEQ(leftVal, rightVal);
        else return env->CreateICmpNE(leftVal, rightVal);
    }
    return relExpr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* EqExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Calculate(env);
        auto* rightVal = right->Calculate(env);
        if (op == EqExprAST::Op::EQ) {
            return env->CaculateBinaryOp([](int a, int b) { return a == b; }, leftVal, rightVal);
        } else if (op == EqExprAST::Op::NE) {
            return env->CaculateBinaryOp([](int a, int b) { return a != b; }, leftVal, rightVal);
        }
    }
    return relExpr->Calculate(env);
}

LAndExprAST::LAndExprAST(unique_ptr<EqExprAST>&& eqExpr)
    : eqExpr(std::move(eqExpr)) {}

LAndExprAST::LAndExprAST(unique_ptr<LAndExprAST>&& left, unique_ptr<EqExprAST>&& right) 
    : left(std::move(left)), right(std::move(right)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LAndExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Codegen(env);
        auto* func = env->GetFunction();
        auto* rightBB = env->CreateBasicBlock("land_right", func);
        auto* endBB = env->CreateBasicBlock("land_end", func);
        auto* result = env->CreateAlloca(env->GetInt32Type(), "land_result");
        auto* cond = env->CreateICmpNE(leftVal, env->GetInt32(0));

        env->CreateStore(cond, result);
        env->CreateCondBr(cond, rightBB, endBB);

        env->SetInserPointer(rightBB);
        auto* rightVal = right->Codegen(env);
        cond = env->CreateICmpNE(rightVal, env->GetInt32(0));
        env->CreateStore(cond, result);
        env->CreateBr(endBB);

        env->SetInserPointer(endBB);
        return env->CreateLoad(result);
    }
    return eqExpr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LAndExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        
        auto* leftVal = left->Calculate(env);
        auto* rightVal = right->Calculate(env);
        return env->CaculateBinaryOp([](int a, int b) { return a || b; }, leftVal, rightVal);
    }
    return eqExpr->Calculate(env);
}

LOrExprAST::LOrExprAST(unique_ptr<LAndExprAST>&& landExpr)
    : landExpr(std::move(landExpr)) {}

LOrExprAST::LOrExprAST(unique_ptr<LOrExprAST>&& left, unique_ptr<LAndExprAST>&& right) 
    : left(std::move(left)), right(std::move(right)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LOrExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Codegen(env);
        auto* func = env->GetFunction();
        auto* rightBB = env->CreateBasicBlock("lor_right", func);
        auto* endBB = env->CreateBasicBlock("lor_end", func);
        auto* result = env->CreateAlloca(env->GetInt32Type(), "lor_result");
        auto* cond = env->CreateICmpNE(leftVal, env->GetInt32(0));

        env->CreateStore(cond, result);
        env->CreateCondBr(cond, endBB, rightBB);

        env->SetInserPointer(rightBB);
        auto* rightVal = right->Codegen(env);
        cond = env->CreateICmpNE(rightVal, env->GetInt32(0));
        env->CreateStore(cond, result);
        env->CreateBr(endBB);

        env->SetInserPointer(endBB);
        return env->CreateLoad(result);
    }
    return landExpr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LOrExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->Calculate(env);
        auto* rightVal = right->Calculate(env);
        return env->CaculateBinaryOp([](int a, int b) { return a || b; }, leftVal, rightVal);
    }
    return landExpr->Calculate(env);
}

DeclAST::DeclAST(unique_ptr<ConstDeclAST>&& constDecl)
    : constDecl(std::move(constDecl)) {}

DeclAST::DeclAST(unique_ptr<VarDeclAST>&& varDecl)
    : varDecl(std::move(varDecl)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void DeclAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (constDecl) {
        constDecl->Codegen(env);
    } else if (varDecl) {
        varDecl->Codegen(env);
    }
}

ConstDeclAST::ConstDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<ConstDefAST>>&& constDefs)
    : btype(std::move(btype)), constDefs(std::move(constDefs)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void ConstDeclAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    for (auto& def : constDefs) {
        auto* initVal = def->constInitVal->Calculate(env);
        env->AddSymbol(def->ident, VAR_TYPE::CONST, {.value = initVal});
    }
}

VarDeclAST::VarDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<VarDefAST>>&& varDefs)
    : btype(std::move(btype)), varDefs(std::move(varDefs)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void VarDeclAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    auto* type = btype->Codegen(env);
    for (const auto& def : varDefs) {
        Value* varAddr;
        if (env->IsGlobalScope()) {
            auto* initVal = def->initVal ? def->initVal->Calculate(env) : env->CreateZero(type);
            varAddr = env->CreateGlobal(type, def->ident, initVal);
        } else {
            varAddr = env->CreateAlloca(type, def->ident);
            if (def->initVal) {
                env->CreateStore(def->initVal->Codegen(env), varAddr);
            }
        }
        env->AddSymbol(def->ident, VAR_TYPE::VAR, {.value = varAddr});
    }
}

ConstInitValAST::ConstInitValAST(unique_ptr<ConstExprAST>&& constExpr)
    : constExpr(std::move(constExpr)), isArray(false) {}

ConstInitValAST::ConstInitValAST()
    : isArray(true) {}

ConstInitValAST::ConstInitValAST(vector<unique_ptr<ConstExprAST>>&& constExprs)
    : constExprs(std::move(constExprs)), isArray(true) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstInitValAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (!isArray) return constExpr->Codegen(env);
    return constExprs[0]->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstInitValAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    if (!isArray) return constExpr->Calculate(env);
    return constExprs[0]->Calculate(env);
}

InitValAST::InitValAST(unique_ptr<ExprAST>&& expr)
    : expr(std::move(expr)), isArray(false) {}

InitValAST::InitValAST()
    : isArray(true) {}

InitValAST::InitValAST(vector<unique_ptr<ExprAST>>&& exprs)
    : exprs(std::move(exprs)), isArray(true) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* InitValAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (!isArray) return expr->Codegen(env);
    return exprs[0]->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* InitValAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    if (!isArray) return expr->Calculate(env);
    return exprs[0]->Calculate(env);
}

BlockItemAST::BlockItemAST(unique_ptr<DeclAST>&& decl) : decl(std::move(decl)) {}

BlockItemAST::BlockItemAST(unique_ptr<StmtAST>&& stmt) : stmt(std::move(stmt)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void BlockItemAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (decl) {
        decl->Codegen(env);
    } else if (stmt) {
        stmt->Codegen(env);
    }
}

LValAST::LValAST(string ident) : ident(std::move(ident)) {}

LValAST::LValAST(string ident, unique_ptr<ExprAST>&& index)
    : ident(std::move(ident)), index(std::move(index)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LValAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    auto symbol = env->GetSymbolValue(ident);
    if (index) {
        auto* indexVal = index->Codegen(env);
        // return env->CreateGEP(env->GetInt32Type(), symbol.value, indexVal);
    }
    return symbol.value;
}

ConstExprAST::ConstExprAST(unique_ptr<ExprAST>&& expr) : expr(std::move(expr)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    return expr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    return expr->Calculate(env);
}

FuncFParamAST::FuncFParamAST(unique_ptr<BaseType>&& btype, string ident)
    : btype(std::move(btype)), ident(std::move(ident)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* FuncFParamAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    auto* addr = env->CreateAlloca(btype->Codegen(env), ident);
    env->AddSymbol(ident, VAR_TYPE::VAR, {.value = addr});
    return addr;
}

FuncRParamAST::FuncRParamAST(unique_ptr<ExprAST>&& expr) : expr(std::move(expr)) {}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* FuncRParamAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    return expr->Codegen(env);
}

#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/BasicBlock.h"

template void CompUnitAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template void FuncDefAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template void BlockAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template void StmtAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* ExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* PrimaryExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* NumberAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* UnaryExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* MulExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* AddExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* RelExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* EqExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* LAndExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* LOrExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template void DeclAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template void ConstDeclAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template void VarDeclAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* ConstInitValAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* InitValAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template void BlockItemAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* LValAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* ConstExprAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* FuncFParamAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* FuncRParamAST::Codegen<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);

template void CompUnitAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template void FuncDefAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template void BlockAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template void StmtAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* ExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* PrimaryExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* NumberAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* UnaryExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* MulExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* AddExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* RelExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* EqExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* LAndExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* LOrExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template void DeclAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template void ConstDeclAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template void VarDeclAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* ConstInitValAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* InitValAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template void BlockItemAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* LValAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* ConstExprAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* FuncFParamAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* FuncRParamAST::Codegen<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);

template llvm::Value* ExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* PrimaryExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* UnaryExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* MulExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* AddExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* RelExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* EqExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* LAndExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* LOrExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* ConstInitValAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* InitValAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);
template llvm::Value* ConstExprAST::Calculate<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>(Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>*);

// ------------------------------
// 补充Calculate方法的显式实例化（Koopa类型）
// ------------------------------
template koopa::Value* ExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* PrimaryExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* UnaryExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* MulExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* AddExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* RelExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* EqExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* LAndExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* LOrExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* ConstInitValAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* InitValAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);
template koopa::Value* ConstExprAST::Calculate<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>(Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>*);