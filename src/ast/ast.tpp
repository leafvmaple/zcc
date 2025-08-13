#pragma once

#include "ast/ast.h"
#include "../libkoopa/include/function.h"
#include "llvm/IR/Function.h"

template<typename Type, typename Value, typename BasicBlock, typename Function>
void ConstDefAST::Codegen(Env<Type, Value, BasicBlock, Function>* env, Type* type) {
    vector<int> shape{};
    if (!sizeExprs.empty()) {
        for (auto& sizeExpr : sizeExprs) {
            int size = sizeExpr->ToInteger(env);
            shape.push_back(size);
        }
    }
    auto var = constInitVal->Calculate(env, shape, 0);
    env->AddSymbol(ident, VAR_TYPE::CONST, {.value = var});
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* VarDefAST::Codegen(Env<Type, Value, BasicBlock, Function>* env, Type* type) {
    Value* var{};
    vector<int> shape{};
    vector<Type*> types{};

    types.push_back(type);
    if (!sizeExprs.empty()) {
        for (auto it = sizeExprs.rbegin(); it != sizeExprs.rend(); ++it) {
            int size = (*it)->ToInteger(env);
            shape.push_back(size);
            type = env->GetArrayType(type, size);
            types.push_back(type);
        }
    }
    std::reverse(shape.begin(), shape.end());

    if (env->IsGlobalScope()) {
        auto* value = initVal ? initVal->Calculate(env, shape, 0) : env->CreateZero(type);
        var = env->CreateGlobal(type, ident, value);
    } else {
        var = env->CreateAlloca(type, ident);
        if (initVal) {
            initVal->Codegen(env, var, types, shape, 0);
        }
    }

    return var;
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

template<typename Type, typename Value, typename BasicBlock, typename Function>
void BlockAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    env->EnterScope();
    for (auto&& item : items) {
        item->Codegen(env);
    }
    env->ExitScope();
}

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
            if (!env->EndWithTerminator()) {
                env->CreateBr(endBB);
            }

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

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    return lorExpr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    return lorExpr->Calculate(env);
}

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

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* NumberAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    return env->GetInt32(value);
}

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
                case OP::NOT: return env->CaculateBinaryOp([](int a, int b) { return a == b; }, exprVal, env->GetInt32(0));
            }
        }
        case TYPE::Call: {
            assert(false && "Call expressions should not be calculated directly");
        }
    }
    return nullptr;
}

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
        return env->CaculateBinaryOp([](int a, int b) { return a && b; }, leftVal, rightVal);
    }
    return eqExpr->Calculate(env);
}

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

template<typename Type, typename Value, typename BasicBlock, typename Function>
void DeclAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (constDecl) {
        constDecl->Codegen(env);
    } else if (varDecl) {
        varDecl->Codegen(env);
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void ConstDeclAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    auto* type = btype->Codegen(env);
    for (const auto& def : constDefs) {
        def->Codegen(env, type);
        // env->AddSymbol(def->ident, VAR_TYPE::CONST, {.value = var});
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void VarDeclAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    auto* type = btype->Codegen(env);
    for (const auto& def : varDefs) {
        auto* var = def->Codegen(env, type);
        env->AddSymbol(def->ident, VAR_TYPE::VAR, {.value = var});
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstInitValAST::Calculate(Env<Type, Value, BasicBlock, Function>* env, vector<int> shape, int dim) {
    if (!isArray) return constExpr->Calculate(env);
    vector<Value*> values;
    Type* type{};
    auto size = shape[dim];
    for (int i = 0; i < size; ++i) {
        auto* val = i < subVals.size() ? subVals[i]->Calculate(env, shape, dim + 1) : env->CreateZero(type);
        values.push_back(val);
        type = env->GetValueType(val);
    }

    auto* arrType = env->GetArrayType(type, size);
    return env->CreateArray(arrType, values);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void InitValAST::Codegen(Env<Type, Value, BasicBlock, Function>* env, Value* addr, vector<Type*> types, vector<int> shape, int dim) {
    if (!isArray) {
        env->CreateStore(expr->Codegen(env), addr);
    } else {
        auto size = shape[dim];
        auto type = types[types.size() - 2 - dim];
        for (int i = 0; i < size; ++i) {
            auto* subAddr = env->CreateGEP(type, addr, { env->GetInt32(i) });
            if (i < subVals.size()) {
                subVals[i]->Codegen(env, subAddr, types, shape, dim + 1);
            } else {
                // If there are not enough subVals, we initialize with zero
                env->CreateStore(env->CreateZero(type), subAddr);
            }
        }
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* InitValAST::Calculate(Env<Type, Value, BasicBlock, Function>* env, vector<int> shape, int dim) {
    if (!isArray) return expr->Calculate(env);
    vector<Value*> values;
    Type* type{};
    auto size = shape[dim];
    for (int i = 0; i < size; ++i) {
        auto* val = i < subVals.size() ? subVals[i]->Calculate(env, shape, dim + 1) : env->CreateZero(type);
        values.push_back(val);
        type = env->GetValueType(val);
    }

    auto* arrType = env->GetArrayType(type, size);
    return env->CreateArray(arrType, values);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void BlockItemAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    if (decl) {
        decl->Codegen(env);
    } else if (stmt) {
        stmt->Codegen(env);
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LValAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    auto symbol = env->GetSymbolValue(ident);
    if (!indies.empty()) {
        vector<Value*> indexVals;
        for (auto& index : indies) {
            indexVals.push_back(index->Codegen(env));
        }
        return env->CreateGEP(env->GetInt32Type(), symbol.value, indexVals);
    }
    return symbol.value;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstExprAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    return expr->Codegen(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstExprAST::Calculate(Env<Type, Value, BasicBlock, Function>* env) {
    return expr->Calculate(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
int ConstExprAST::ToInteger(Env<Type, Value, BasicBlock, Function>* env) {
    auto value = expr->Calculate(env);
    return env->GetValueInt(value);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* FuncFParamAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    auto* addr = env->CreateAlloca(btype->Codegen(env), ident);
    env->AddSymbol(ident, VAR_TYPE::VAR, {.value = addr});
    return addr;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* FuncRParamAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    return expr->Codegen(env);
}
