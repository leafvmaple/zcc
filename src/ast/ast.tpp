#pragma once

#include "ast/ast.h"
#include "../libkoopa/include/function.h"
#include "llvm/IR/Function.h"

template<typename Type, typename Value, typename BasicBlock, typename Function, typename T>
void _Flatten(Env<Type, Value, BasicBlock, Function>* env, vector<Value*>& flatValues, const vector<int>& shape, int dim, T& initVal) {
        if (!initVal->isArray) {
        flatValues.push_back(initVal->expr->ToNumber(env));
        return;
    }
    
    int startIndex = flatValues.size();
    int totalElements = 1;
    for (int i = shape.size() - 2; i >= dim; i--) {
        if (startIndex % (totalElements * shape[i]) == 0) {
            totalElements *= shape[i];
        }
    }
    
    for (auto& val : initVal->subVals) {
        _Flatten(env, flatValues, shape, dim + 1, val);
    }

    int filledElements = flatValues.size() - startIndex;
    if (filledElements < totalElements) {
        for (int i = filledElements; i < totalElements; i++) {
            flatValues.push_back(env->GetInt32(0));
        }
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* _ToArray(Env<Type, Value, BasicBlock, Function>* env, vector<int>::iterator shape, Value**& init) {
    auto size = *shape;
    if (!size) {
        return *init++;
    }
    Type* type{};
    vector<Value*> values;
    for (int i = 0; i < size; ++i) {
        auto* val = _ToArray(env, shape + 1, init);
        values.push_back(val);
        type = env->GetValueType(val);
    }

    auto* arrType = env->GetArrayType(type, size);
    return env->CreateArray(arrType, values);
}

template<typename Type, typename Value, typename BasicBlock, typename Function, typename T>
Value* _FlattenToArray(Env<Type, Value, BasicBlock, Function>* env, vector<int>& shape, T& initVal) {
    vector<Value*> flatValues;
    _Flatten(env, flatValues, shape, 0, initVal);

    auto values_ptr = flatValues.data();
    return _ToArray(env, shape.begin(), values_ptr);
}

template<typename Type, typename Value, typename BasicBlock, typename Function, typename T>
Value* _GetArrayInitValue(Env<Type, Value, BasicBlock, Function>* env, vector<int>& shape, Type*& type, T* def) {
    Value* var{};
    if (def->initVal) {
        var = _FlattenToArray(env, shape, def->initVal);
        type = env->GetValueType(var);
    } else {
        for (auto it = shape.rbegin() + 1; *it; ++it) {
            type = env->GetArrayType(type, *it);
        }
        var = env->CreateZero(type);
    }

    return var;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void _StoreArray(Env<Type, Value, BasicBlock, Function>* env, Value* addr, vector<int>::iterator shape, Value* val) {
    if (*shape == 0) {
        env->CreateStore(val, addr);
        return;
    }

    for (int i = 0; i < *shape; ++i) {
        auto element = env->GetArrayElement(val, i);
        auto type = env->GetValueType(element);
        auto* subAddr = env->CreateGEP(type, addr, { env->GetInt32(i) });
        _StoreArray(env, subAddr, shape + 1, element);
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void ConstDefAST::Codegen(Env<Type, Value, BasicBlock, Function>* env, Type* type) {
    vector<int> shape{};
    int elementCount = 1;
    for (auto& sizeExpr : sizeExprs) {
        int size = sizeExpr->ToInteger(env);
        shape.push_back(size);
        elementCount *= size;
    }
    shape.push_back(0);

    Value* init{};
    if (!sizeExprs.empty()) {
        init = _GetArrayInitValue(env, shape, type, this);
    } else {
        init = initVal->ToNumber(env);
    }

    Value* var{};
    if (env->IsGlobalScope()) {
        var = env->CreateGlobal(type, ident, init);
    } else if (!sizeExprs.empty()) {
        var = env->CreateAlloca(type, ident);
        if (initVal) {
            _StoreArray(env, var, shape.begin(), init);
        }
    } else {
        var = init;
    }

    env->AddSymbol(ident, VAR_TYPE::CONST, {.value = var});
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void VarDefAST::Codegen(Env<Type, Value, BasicBlock, Function>* env, Type* type) {
    Value* init{};
    vector<int> shape{};
    for (auto& sizeExpr : sizeExprs) {
        int size = sizeExpr->ToInteger(env);
        shape.push_back(size);
    }
    shape.push_back(0);

    if (!sizeExprs.empty()) {
        init = _GetArrayInitValue(env, shape, type, this);
    } else {
        init = initVal ? initVal->ToValue(env, init) : env->GetInt32(0);
    }

    Value* var{};
    if (env->IsGlobalScope()) {
        var = env->CreateGlobal(type, ident, init);
        env->AddSymbol(ident, VAR_TYPE::GLOBAL, {.value = var});
    } else {
        var = env->CreateAlloca(type, ident);
        if (initVal) {
            _StoreArray(env, var, shape.begin(), init);
        }
        env->AddSymbol(ident, VAR_TYPE::VAR, {.value = var});
    }
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
        paramTypes.push_back(param->ToType(env));
        paramNames.push_back(param->ident);
    }

    auto* funcType = env->CreateFuncType(this->funcType->Codegen(env), paramTypes);
    auto* func = env->CreateFunction(funcType, ident, paramNames);
    auto* entryBB = env->CreateBasicBlock("entry", func);
    env->SetInserPointer(entryBB);

    env->EnterScope();

    for (size_t i = 0; i < params.size(); ++i) {
        env->CreateStore(env->GetFunctionArg(i), params[i]->Alloca(env, paramTypes[i]));
    }

    block->Codegen(env);

    env->ExitScope();
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void BlockAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    env->EnterScope();
    for (auto&& item : items) {
        item->ToValue(env);
    }
    env->ExitScope();
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void StmtAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Assign: {
            auto* val = expr->ToValue(env);
            auto* addr = lval->ToPointer(env);
            env->CreateStore(val, addr);
            break;
        }
        case TYPE::Expr: {
            if (expr) expr->ToValue(env);
            break;
        }
        case TYPE::Block: {
            block->Codegen(env);
            break;
        }
        case TYPE::If: {
            auto* condVal = cond->ToValue(env);
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
            auto* retVal = expr->ToValue(env);
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
            auto* condVal = cond->ToValue(env);
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
Value* ExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    return lorExpr->ToValue(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    return lorExpr->ToNumber(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* PrimaryExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Expr: return expr->ToValue(env);
        case TYPE::LVal: return lval->ToValue(env);
        case TYPE::Number: return value->ToValue(env);
    }
    return nullptr;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* PrimaryExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Expr: return expr->ToNumber(env);
        case TYPE::LVal: return lval->ToNumber(env);
        case TYPE::Number: return value->ToValue(env);
    }
    return nullptr;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* NumberAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    return env->GetInt32(value);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* UnaryExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Primary:
            return primaryExpr->ToValue(env);
        case TYPE::Unary: {
            auto* exprVal = unaryExpr->ToValue(env);
            switch (op) {
                case OP::PLUS: return exprVal;
                case OP::MINUS: return env->CreateSub(env->GetInt32(0), exprVal);
                case OP::NOT: return env->CreateICmpEQ(exprVal, env->GetInt32(0));
            }
        }
        case TYPE::Call: {
            std::vector<Value*> args;
            for (auto& arg : callArgs) {
                args.push_back(arg->ToValue(env));
            }
            auto symbol = env->GetSymbolValue(ident);
            return env->CreateCall(symbol.function, args);
        }
    }
    return nullptr;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* UnaryExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    switch (type) {
        case TYPE::Primary:
            return primaryExpr->ToNumber(env);
        case TYPE::Unary: {
            auto* exprVal = unaryExpr->ToNumber(env);
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
Value* MulExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToValue(env);
        auto* rightVal = right->ToValue(env);
        switch (op) {
            case OP::MUL: return env->CreateMul(leftVal, rightVal);
            case OP::DIV: return env->CreateDiv(leftVal, rightVal);
            case OP::MOD: return env->CreateMod(leftVal, rightVal);
        }
    }
    return unaryExpr->ToValue(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* MulExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToNumber(env);
        auto* rightVal = right->ToNumber(env);
        switch (op) {
            case OP::MUL: return env->CaculateBinaryOp([](int a, int b) { return a * b; }, leftVal, rightVal);
            case OP::DIV: return env->CaculateBinaryOp([](int a, int b) { return a / b; }, leftVal, rightVal);
            case OP::MOD: return env->CaculateBinaryOp([](int a, int b) { return a % b; }, leftVal, rightVal);
        }
    }
    return unaryExpr->ToNumber(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* AddExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToValue(env);
        auto* rightVal = right->ToValue(env);
        if (op == OP::ADD) return env->CreateAdd(leftVal, rightVal);
        else return env->CreateSub(leftVal, rightVal);
    }
    return mulExpr->ToValue(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* AddExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToNumber(env);
        auto* rightVal = right->ToNumber(env);
        if (op == OP::ADD) {
            return env->CaculateBinaryOp([](int a, int b) { return a + b; }, leftVal, rightVal);
        } else {
            return env->CaculateBinaryOp([](int a, int b) { return a - b; }, leftVal, rightVal);
        }
    }
    return mulExpr->ToNumber(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* RelExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToValue(env);
        auto* rightVal = right->ToValue(env);
        switch (op) {
            case Op::LT: return env->CreateICmpLT(leftVal, rightVal);
            case Op::GT: return env->CreateICmpGT(leftVal, rightVal);
            case Op::LE: return env->CreateICmpLE(leftVal, rightVal);
            case Op::GE: return env->CreateICmpGE(leftVal, rightVal);
        }
    }
    return addExpr->ToValue(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* RelExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToNumber(env);
        auto* rightVal = right->ToNumber(env);
        switch (op) {
            case Op::LT: return env->CaculateBinaryOp([](int a, int b) { return a < b; }, leftVal, rightVal);
            case Op::GT: return env->CaculateBinaryOp([](int a, int b) { return a > b; }, leftVal, rightVal);
            case Op::LE: return env->CaculateBinaryOp([](int a, int b) { return a <= b; }, leftVal, rightVal);
            case Op::GE: return env->CaculateBinaryOp([](int a, int b) { return a >= b; }, leftVal, rightVal);
        }
    }
    return addExpr->ToNumber(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* EqExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToValue(env);
        auto* rightVal = right->ToValue(env);
        if (op == Op::EQ) return env->CreateICmpEQ(leftVal, rightVal);
        else return env->CreateICmpNE(leftVal, rightVal);
    }
    return relExpr->ToValue(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* EqExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToNumber(env);
        auto* rightVal = right->ToNumber(env);
        if (op == EqExprAST::Op::EQ) {
            return env->CaculateBinaryOp([](int a, int b) { return a == b; }, leftVal, rightVal);
        } else if (op == EqExprAST::Op::NE) {
            return env->CaculateBinaryOp([](int a, int b) { return a != b; }, leftVal, rightVal);
        }
    }
    return relExpr->ToNumber(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LAndExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToValue(env);
        auto* func = env->GetFunction();
        auto* rightBB = env->CreateBasicBlock("land_right", func);
        auto* endBB = env->CreateBasicBlock("land_end", func);
        auto* result = env->CreateAlloca(env->GetInt32Type(), "land_result");
        auto* cond = env->CreateICmpNE(leftVal, env->GetInt32(0));

        env->CreateStore(cond, result);
        env->CreateCondBr(cond, rightBB, endBB);

        env->SetInserPointer(rightBB);
        auto* rightVal = right->ToValue(env);
        cond = env->CreateICmpNE(rightVal, env->GetInt32(0));
        env->CreateStore(cond, result);
        env->CreateBr(endBB);

        env->SetInserPointer(endBB);
        return env->CreateLoad(result);
    }
    return eqExpr->ToValue(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LAndExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToNumber(env);
        auto* rightVal = right->ToNumber(env);
        return env->CaculateBinaryOp([](int a, int b) { return a && b; }, leftVal, rightVal);
    }
    return eqExpr->ToNumber(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LOrExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToValue(env);
        auto* func = env->GetFunction();
        auto* rightBB = env->CreateBasicBlock("lor_right", func);
        auto* endBB = env->CreateBasicBlock("lor_end", func);
        auto* result = env->CreateAlloca(env->GetInt32Type(), "lor_result");
        auto* cond = env->CreateICmpNE(leftVal, env->GetInt32(0));

        env->CreateStore(cond, result);
        env->CreateCondBr(cond, endBB, rightBB);

        env->SetInserPointer(rightBB);
        auto* rightVal = right->ToValue(env);
        cond = env->CreateICmpNE(rightVal, env->GetInt32(0));
        env->CreateStore(cond, result);
        env->CreateBr(endBB);

        env->SetInserPointer(endBB);
        return env->CreateLoad(result);
    }
    return landExpr->ToValue(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LOrExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    if (left) {
        auto* leftVal = left->ToNumber(env);
        auto* rightVal = right->ToNumber(env);
        return env->CaculateBinaryOp([](int a, int b) { return a || b; }, leftVal, rightVal);
    }
    return landExpr->ToNumber(env);
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
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void VarDeclAST::Codegen(Env<Type, Value, BasicBlock, Function>* env) {
    auto* type = btype->Codegen(env);
    for (const auto& def : varDefs) {
        def->Codegen(env, type);
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstInitValAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    assert(!isArray);
    return expr->ToNumber(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* InitValAST::ToValue(Env<Type, Value, BasicBlock, Function>* env, Value* addr) {
    return expr->ToValue(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* InitValAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env, vector<int> shape, int dim) {
    if (!isArray) return expr->ToNumber(env);
    vector<Value*> values;
    Type* type{};
    auto size = shape[dim];
    for (int i = 0; i < size; ++i) {
        auto* val = i < subVals.size() ? subVals[i]->ToNumber(env, shape, dim + 1) : env->CreateZero(type);
        values.push_back(val);
        type = env->GetValueType(val);
    }

    auto* arrType = env->GetArrayType(type, size);
    return env->CreateArray(arrType, values);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
void BlockItemAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    if (decl) {
        decl->Codegen(env);
    } else if (stmt) {
        stmt->Codegen(env);
    }
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LValAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    auto symbol = env->GetSymbolValue(ident);
    auto value = symbol.value;
    auto symbolType = env->GetSymbolType({.value = value});

    if (symbolType == VAR_TYPE::VAR) {
        auto type = env->GetValueType(value);
        type = env->GetElementType(type);
        if (env->IsArrayType(type) && indies.empty()) {
            return env->CreateGEP(env->GetInt32Type(), value, { env->GetInt32(0) });
        }
    }
    if (!indies.empty()) {
        vector<Value*> indexVals;
        for (auto& index : indies) {
            indexVals.push_back(index->ToValue(env));
        }
        // value = env->CreateLoad(value);
        value = env->CreateGEP(env->GetInt32Type(), value, indexVals);
    }
    return env->CreateLoad(value);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LValAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    auto symbol = env->GetSymbolValue(ident);
    auto value = symbol.value;
    return env->GetBaseValue(value);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* LValAST::ToPointer(Env<Type, Value, BasicBlock, Function>* env) {
    auto symbol = env->GetSymbolValue(ident);
    auto value = symbol.value;
    return value;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstExprAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    return expr->ToValue(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* ConstExprAST::ToNumber(Env<Type, Value, BasicBlock, Function>* env) {
    return expr->ToNumber(env);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
int ConstExprAST::ToInteger(Env<Type, Value, BasicBlock, Function>* env) {
    auto value = expr->ToNumber(env);
    return env->GetValueInt(value);
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Type* FuncFParamAST::ToType(Env<Type, Value, BasicBlock, Function>* env) {
    Type* type = btype->Codegen(env);
    if (isArray) {
        for (auto& sizeExpr : sizeExprs) {
            int size = sizeExpr->ToInteger(env);
            type = env->GetArrayType(type, size);
        }
        type = env->GetPointerType(type);
    }
    return type;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* FuncFParamAST::Alloca(Env<Type, Value, BasicBlock, Function>* env, Type* type) {
    auto* addr = env->CreateAlloca(type, ident);
    env->AddSymbol(ident, VAR_TYPE::VAR, {.value = addr});
    return addr;
}

template<typename Type, typename Value, typename BasicBlock, typename Function>
Value* FuncRParamAST::ToValue(Env<Type, Value, BasicBlock, Function>* env) {
    return expr->ToValue(env);
}
