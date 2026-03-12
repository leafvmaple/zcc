#include "ast/ast.h"
#include "ir/codegen.h"

#include <cassert>

// ========== Constructors ==========

ConstDefAST::ConstDefAST(string ident, unique_ptr<ConstInitValAST>&& initVal)
    : ident(std::move(ident)), initVal(std::move(initVal)) {}

ConstDefAST::ConstDefAST(string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs, unique_ptr<ConstInitValAST>&& initVal)
    : ident(std::move(ident)), sizeExprs(std::move(sizeExprs)), initVal(std::move(initVal)) {}

VarDefAST::VarDefAST(string ident)
    : ident(std::move(ident)) {}

VarDefAST::VarDefAST(string ident, unique_ptr<InitValAST>&& initVal)
    : ident(std::move(ident)), initVal(std::move(initVal)) {}

VarDefAST::VarDefAST(string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs)
    : ident(std::move(ident)), sizeExprs(std::move(sizeExprs)) {}

VarDefAST::VarDefAST(string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs, unique_ptr<InitValAST>&& initVal)
    : ident(std::move(ident)), sizeExprs(std::move(sizeExprs)), initVal(std::move(initVal)) {}

void CompUnitAST::AddFuncDef(unique_ptr<FuncDefAST>&& funcDef) {
    funcDefs.emplace_back(std::move(funcDef));
}

void CompUnitAST::AddDecl(unique_ptr<DeclAST>&& decl) {
    decls.emplace_back(std::move(decl));
}

FuncDefAST::FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BlockAST>&& block)
    : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}

FuncDefAST::FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, vector<unique_ptr<FuncFParamAST>>&& params, unique_ptr<BlockAST>&& block)
    : funcType(std::move(funcType)), ident(std::move(ident)), params(std::move(params)), block(std::move(block)) {}

BlockAST::BlockAST(vector<unique_ptr<BlockItemAST>>&& items)
    : items(std::move(items)) {}

StmtAST::StmtAST(TYPE type) : type(type) {}
StmtAST::StmtAST(TYPE type, unique_ptr<ExprAST>&& expr) : type(type), expr(std::move(expr)) {}
StmtAST::StmtAST(TYPE type, unique_ptr<BlockAST>&& block) : type(type), block(std::move(block)) {}
StmtAST::StmtAST(TYPE type, unique_ptr<LValAST>&& lval, unique_ptr<ExprAST>&& expr)
    : type(type), lval(std::move(lval)), expr(std::move(expr)) {}
StmtAST::StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt)
    : type(type), cond(std::move(cond)), thenStmt(std::move(thenStmt)) {}
StmtAST::StmtAST(TYPE type, unique_ptr<ExprAST>&& cond, unique_ptr<StmtAST>&& thenStmt, unique_ptr<StmtAST>&& elseStmt)
    : type(type), cond(std::move(cond)), thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}

ExprAST::ExprAST(unique_ptr<LOrExprAST>&& lorExpr) : lorExpr(std::move(lorExpr)) {}

PrimaryExprAST::PrimaryExprAST(TYPE type, unique_ptr<ExprAST>&& expr) : type(type), expr(std::move(expr)) {}
PrimaryExprAST::PrimaryExprAST(TYPE type, unique_ptr<LValAST>&& lval) : type(type), lval(std::move(lval)) {}
PrimaryExprAST::PrimaryExprAST(TYPE type, unique_ptr<NumberAST>&& value) : type(type), value(std::move(value)) {}
PrimaryExprAST::PrimaryExprAST(string strVal) : type(TYPE::String), strVal(std::move(strVal)) {}

NumberAST::NumberAST(int value) : value(value) {}

UnaryExprAST::UnaryExprAST(TYPE type, unique_ptr<PrimaryExprAST>&& primaryExpr)
    : type(type), primaryExpr(std::move(primaryExpr)) {}
UnaryExprAST::UnaryExprAST(TYPE type, OP op, unique_ptr<UnaryExprAST>&& unaryExpr)
    : type(type), op(op), unaryExpr(std::move(unaryExpr)) {}
UnaryExprAST::UnaryExprAST(TYPE type, string ident) : type(type), ident(std::move(ident)) {}
UnaryExprAST::UnaryExprAST(TYPE type, string ident, vector<unique_ptr<ExprAST>>&& callArgs)
    : type(type), ident(std::move(ident)), callArgs(std::move(callArgs)) {}

BinaryExprAST::BinaryExprAST(unique_ptr<UnaryExprAST>&& operand)
    : operand(std::move(operand)) {}
BinaryExprAST::BinaryExprAST(Op op, unique_ptr<BinaryExprAST>&& lhs, unique_ptr<BinaryExprAST>&& rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

LAndExprAST::LAndExprAST(unique_ptr<BinaryExprAST>&& operand) : operand(std::move(operand)) {}
LAndExprAST::LAndExprAST(unique_ptr<LAndExprAST>&& left, unique_ptr<BinaryExprAST>&& right)
    : left(std::move(left)), right(std::move(right)) {}

LOrExprAST::LOrExprAST(unique_ptr<LAndExprAST>&& operand) : operand(std::move(operand)) {}
LOrExprAST::LOrExprAST(unique_ptr<LOrExprAST>&& left, unique_ptr<LAndExprAST>&& right)
    : left(std::move(left)), right(std::move(right)) {}

DeclAST::DeclAST(unique_ptr<ConstDeclAST>&& constDecl) : constDecl(std::move(constDecl)) {}
DeclAST::DeclAST(unique_ptr<VarDeclAST>&& varDecl) : varDecl(std::move(varDecl)) {}

ConstDeclAST::ConstDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<ConstDefAST>>&& constDefs)
    : btype(std::move(btype)), constDefs(std::move(constDefs)) {}

VarDeclAST::VarDeclAST(unique_ptr<BaseType>&& btype, vector<unique_ptr<VarDefAST>>&& varDefs)
    : btype(std::move(btype)), varDefs(std::move(varDefs)) {}

ConstInitValAST::ConstInitValAST() : isArray(true) {}
ConstInitValAST::ConstInitValAST(unique_ptr<ConstExprAST>&& expr) : expr(std::move(expr)), isArray(false) {}
ConstInitValAST::ConstInitValAST(vector<unique_ptr<ConstInitValAST>>&& subVals) : subVals(std::move(subVals)), isArray(true) {}

InitValAST::InitValAST() : isArray(true) {}
InitValAST::InitValAST(unique_ptr<ExprAST>&& expr) : expr(std::move(expr)), isArray(false) {}
InitValAST::InitValAST(vector<unique_ptr<InitValAST>>&& subVals) : subVals(std::move(subVals)), isArray(true) {}

BlockItemAST::BlockItemAST(unique_ptr<DeclAST>&& decl) : decl(std::move(decl)) {}
BlockItemAST::BlockItemAST(unique_ptr<StmtAST>&& stmt) : stmt(std::move(stmt)) {}

LValAST::LValAST(string ident) : ident(std::move(ident)) {}
LValAST::LValAST(string ident, vector<unique_ptr<ExprAST>>&& indies)
    : ident(std::move(ident)), indies(std::move(indies)) {}

ConstExprAST::ConstExprAST(unique_ptr<ExprAST>&& expr) : expr(std::move(expr)) {}

FuncFParamAST::FuncFParamAST(unique_ptr<BaseType>&& btype, string ident, bool isArray)
    : btype(std::move(btype)), ident(std::move(ident)), isArray(isArray) {}

FuncFParamAST::FuncFParamAST(unique_ptr<BaseType>&& btype, string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs)
    : btype(std::move(btype)), ident(std::move(ident)), sizeExprs(std::move(sizeExprs)), isArray(true) {}

FuncRParamAST::FuncRParamAST(unique_ptr<ExprAST>&& expr) : expr(std::move(expr)) {}

// ========== Array initialization helpers ==========

namespace {

template<typename T>
void Flatten(CodeGen* cg, vector<llvm::Value*>& flatValues, const vector<int>& shape, int dim, T& initVal) {
    if (!initVal->isArray) {
        flatValues.push_back(initVal->expr->ToNumber(cg));
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
        Flatten(cg, flatValues, shape, dim + 1, val);
    }

    int filledElements = flatValues.size() - startIndex;
    if (filledElements < totalElements) {
        for (int i = filledElements; i < totalElements; i++) {
            flatValues.push_back(cg->GetInt32(0));
        }
    }
}

llvm::Value* ToArray(CodeGen* cg, vector<int>::iterator shape, llvm::Value**& init) {
    auto size = *shape;
    if (!size) return *init++;

    llvm::Type* type{};
    vector<llvm::Value*> values;
    for (int i = 0; i < size; ++i) {
        auto* val = ToArray(cg, shape + 1, init);
        values.push_back(val);
        type = cg->GetValueType(val);
    }
    return cg->CreateArray(cg->GetArrayType(type, size), values);
}

template<typename T>
llvm::Value* FlattenToArray(CodeGen* cg, vector<int>& shape, T& initVal) {
    vector<llvm::Value*> flatValues;
    Flatten(cg, flatValues, shape, 0, initVal);
    auto* ptr = flatValues.data();
    return ToArray(cg, shape.begin(), ptr);
}

template<typename T>
llvm::Value* GetArrayInitValue(CodeGen* cg, vector<int>& shape, llvm::Type*& type, T* def) {
    if (def->initVal) {
        auto* var = FlattenToArray(cg, shape, def->initVal);
        type = cg->GetValueType(var);
        return var;
    }
    for (auto it = shape.rbegin() + 1; *it; ++it)
        type = cg->GetArrayType(type, *it);
    return cg->CreateZero(type);
}

void StoreArray(CodeGen* cg, llvm::Value* addr, vector<int>::iterator shape, llvm::Value* val) {
    if (*shape == 0) {
        cg->CreateStore(val, addr);
        return;
    }
    for (int i = 0; i < *shape; ++i) {
        auto* element = cg->GetArrayElement(val, i);
        auto* subAddr = cg->CreateGEP(cg->GetValueType(element), addr, {cg->GetInt32(i)});
        StoreArray(cg, subAddr, shape + 1, element);
    }
}

} // anonymous namespace

// ========== Code Generation ==========

void ConstDefAST::Codegen(CodeGen* cg, llvm::Type* type) {
    vector<int> shape{};
    for (auto& sizeExpr : sizeExprs)
        shape.push_back(sizeExpr->ToInteger(cg));
    shape.push_back(0);

    llvm::Value* init{};
    if (!sizeExprs.empty())
        init = GetArrayInitValue(cg, shape, type, this);
    else
        init = initVal->ToNumber(cg);

    llvm::Value* var{};
    if (cg->IsGlobalScope()) {
        var = cg->CreateGlobal(type, ident, init);
    } else if (!sizeExprs.empty()) {
        var = cg->CreateAlloca(type, ident);
        if (initVal) StoreArray(cg, var, shape.begin(), init);
    } else {
        var = init;
    }
    cg->AddSymbol(ident, VAR_TYPE::CONST, {.value = var});
}

void VarDefAST::Codegen(CodeGen* cg, llvm::Type* type) {
    vector<int> shape{};
    for (auto& sizeExpr : sizeExprs)
        shape.push_back(sizeExpr->ToInteger(cg));
    shape.push_back(0);

    llvm::Value* init{};
    if (!sizeExprs.empty())
        init = GetArrayInitValue(cg, shape, type, this);
    else
        init = initVal ? initVal->ToValue(cg, init) : cg->GetInt32(0);

    if (cg->IsGlobalScope()) {
        auto* var = cg->CreateGlobal(type, ident, init);
        cg->AddSymbol(ident, VAR_TYPE::GLOBAL, {.value = var});
    } else {
        auto* var = cg->CreateAlloca(type, ident);
        if (initVal) StoreArray(cg, var, shape.begin(), init);
        cg->AddSymbol(ident, VAR_TYPE::VAR, {.value = var});
    }
}

void CompUnitAST::Codegen(CodeGen* cg) {
    auto* intType = cg->GetInt32Type();
    auto* ptrType = cg->GetPointerType(cg->GetInt8Type());

    cg->CreateBuiltin("printf", intType, {ptrType}, true);
    cg->CreateBuiltin("scanf", intType, {ptrType}, true);

    for (auto& decl : decls) decl->Codegen(cg);
    for (auto& funcDef : funcDefs) funcDef->Codegen(cg);
}

void FuncDefAST::Codegen(CodeGen* cg) {
    std::vector<llvm::Type*> paramTypes;
    std::vector<std::string> paramNames;
    for (auto& param : params) {
        paramTypes.push_back(param->ToType(cg));
        paramNames.push_back(param->ident);
    }

    auto* funcType = cg->CreateFuncType(this->funcType->Codegen(cg), paramTypes);
    auto* func = cg->CreateFunction(funcType, ident, paramNames);
    cg->SetInsertPoint(cg->CreateBasicBlock("entry", func));
    cg->EnterScope();

    for (size_t i = 0; i < params.size(); ++i)
        cg->CreateStore(cg->GetFunctionArg(i), params[i]->Alloca(cg, paramTypes[i]));

    block->Codegen(cg);
    if (!cg->EndWithTerminator()) cg->CreateRet(nullptr);
    cg->ExitScope();
}

void BlockAST::Codegen(CodeGen* cg) {
    cg->EnterScope();
    for (auto& item : items) item->ToValue(cg);
    cg->ExitScope();
}

void StmtAST::Codegen(CodeGen* cg) {
    switch (type) {
    case TYPE::Assign: {
        cg->CreateStore(expr->ToValue(cg), lval->ToPointer(cg));
        break;
    }
    case TYPE::Expr: {
        if (expr) expr->ToValue(cg);
        break;
    }
    case TYPE::Block: {
        block->Codegen(cg);
        break;
    }
    case TYPE::If: {
        auto* condVal = cond->ToValue(cg);
        auto* func = cg->GetFunction();
        auto* thenBB = cg->CreateBasicBlock("then", func);
        llvm::BasicBlock* endBB{};

        if (elseStmt) {
            auto* elseBB = cg->CreateBasicBlock("else", func);
            endBB = cg->CreateBasicBlock("if_end", func);
            cg->CreateCondBr(condVal, thenBB, elseBB);
            cg->SetInsertPoint(elseBB);
            elseStmt->Codegen(cg);
            cg->CreateBr(endBB);
        } else {
            endBB = cg->CreateBasicBlock("if_end", func);
            cg->CreateCondBr(condVal, thenBB, endBB);
        }

        cg->SetInsertPoint(thenBB);
        thenStmt->Codegen(cg);
        if (!cg->EndWithTerminator()) cg->CreateBr(endBB);
        cg->SetInsertPoint(endBB);
        break;
    }
    case TYPE::Ret: {
        cg->CreateRet(expr ? expr->ToValue(cg) : nullptr);
        break;
    }
    case TYPE::While: {
        auto* func = cg->GetFunction();
        auto* condBB = cg->CreateBasicBlock("while_cond", func);
        auto* bodyBB = cg->CreateBasicBlock("while_body", func);
        auto* endBB = cg->CreateBasicBlock("while_end", func);

        cg->EnterWhile(condBB, endBB);
        cg->CreateBr(condBB);
        cg->SetInsertPoint(condBB);
        cg->CreateCondBr(cond->ToValue(cg), bodyBB, endBB);
        cg->SetInsertPoint(bodyBB);
        thenStmt->Codegen(cg);
        cg->CreateBr(condBB);
        cg->SetInsertPoint(endBB);
        cg->ExitWhile();
        break;
    }
    case TYPE::Break: {
        cg->CreateBr(cg->GetWhileEnd());
        break;
    }
    case TYPE::Continue: {
        cg->CreateBr(cg->GetWhileEntry());
        break;
    }
    case TYPE::For: {
        bool hasScope = (forDecl != nullptr);
        if (hasScope) cg->EnterScope();
        if (forDecl) forDecl->Codegen(cg);
        else if (forInitStmt) forInitStmt->Codegen(cg);

        auto* func = cg->GetFunction();
        auto* condBB = cg->CreateBasicBlock("for_cond", func);
        auto* bodyBB = cg->CreateBasicBlock("for_body", func);
        auto* stepBB = cg->CreateBasicBlock("for_step", func);
        auto* endBB = cg->CreateBasicBlock("for_end", func);

        cg->EnterWhile(stepBB, endBB);
        cg->CreateBr(condBB);
        cg->SetInsertPoint(condBB);
        if (cond) cg->CreateCondBr(cond->ToValue(cg), bodyBB, endBB);
        else cg->CreateBr(bodyBB);

        cg->SetInsertPoint(bodyBB);
        thenStmt->Codegen(cg);
        if (!cg->EndWithTerminator()) cg->CreateBr(stepBB);

        cg->SetInsertPoint(stepBB);
        if (forStepStmt) forStepStmt->Codegen(cg);
        cg->CreateBr(condBB);

        cg->SetInsertPoint(endBB);
        cg->ExitWhile();
        if (hasScope) cg->ExitScope();
        break;
    }
    }
}

llvm::Value* ExprAST::ToValue(CodeGen* cg)  { return lorExpr->ToValue(cg); }
llvm::Value* ExprAST::ToNumber(CodeGen* cg) { return lorExpr->ToNumber(cg); }

llvm::Value* PrimaryExprAST::ToValue(CodeGen* cg) {
    switch (type) {
        case TYPE::Expr:   return expr->ToValue(cg);
        case TYPE::LVal:   return lval->ToValue(cg);
        case TYPE::Number: return value->ToValue(cg);
        case TYPE::String: return cg->CreateGlobalString(strVal);
    }
    return nullptr;
}

llvm::Value* PrimaryExprAST::ToNumber(CodeGen* cg) {
    switch (type) {
        case TYPE::Expr:   return expr->ToNumber(cg);
        case TYPE::LVal:   return lval->ToNumber(cg);
        case TYPE::Number: return value->ToValue(cg);
        case TYPE::String: return cg->CreateGlobalString(strVal);
    }
    return nullptr;
}

llvm::Value* NumberAST::ToValue(CodeGen* cg) { return cg->GetInt32(value); }

llvm::Value* UnaryExprAST::ToValue(CodeGen* cg) {
    switch (type) {
    case TYPE::Primary: return primaryExpr->ToValue(cg);
    case TYPE::Unary: {
        auto* val = unaryExpr->ToValue(cg);
        switch (op) {
            case OP::PLUS:  return val;
            case OP::MINUS: return cg->CreateSub(cg->GetInt32(0), val);
            case OP::NOT:   return cg->CreateICmpEQ(val, cg->GetInt32(0));
        }
    }
    case TYPE::Call: {
        std::vector<llvm::Value*> args;
        for (auto& arg : callArgs) args.push_back(arg->ToValue(cg));
        return cg->CreateCall(cg->GetSymbolValue(ident).function, args);
    }
    }
    return nullptr;
}

llvm::Value* UnaryExprAST::ToNumber(CodeGen* cg) {
    switch (type) {
    case TYPE::Primary: return primaryExpr->ToNumber(cg);
    case TYPE::Unary: {
        auto* val = unaryExpr->ToNumber(cg);
        switch (op) {
            case OP::PLUS:  return val;
            case OP::MINUS: return cg->CalculateBinaryOp([](int a, int b) { return a - b; }, cg->GetInt32(0), val);
            case OP::NOT:   return cg->CalculateBinaryOp([](int a, int b) { return a == b; }, val, cg->GetInt32(0));
        }
    }
    case TYPE::Call:
        assert(false && "Call expressions should not be calculated directly");
    }
    return nullptr;
}

llvm::Value* BinaryExprAST::ToValue(CodeGen* cg) {
    if (!lhs) return operand->ToValue(cg);
    auto *l = lhs->ToValue(cg), *r = rhs->ToValue(cg);
    switch (op) {
        case Op::ADD: return cg->CreateAdd(l, r);
        case Op::SUB: return cg->CreateSub(l, r);
        case Op::MUL: return cg->CreateMul(l, r);
        case Op::DIV: return cg->CreateDiv(l, r);
        case Op::MOD: return cg->CreateMod(l, r);
        case Op::LT:  return cg->CreateICmpLT(l, r);
        case Op::GT:  return cg->CreateICmpGT(l, r);
        case Op::LE:  return cg->CreateICmpLE(l, r);
        case Op::GE:  return cg->CreateICmpGE(l, r);
        case Op::EQ:  return cg->CreateICmpEQ(l, r);
        case Op::NE:  return cg->CreateICmpNE(l, r);
    }
    return nullptr;
}

llvm::Value* BinaryExprAST::ToNumber(CodeGen* cg) {
    if (!lhs) return operand->ToNumber(cg);
    auto *l = lhs->ToNumber(cg), *r = rhs->ToNumber(cg);
    switch (op) {
        case Op::ADD: return cg->CalculateBinaryOp([](int a, int b) { return a + b; }, l, r);
        case Op::SUB: return cg->CalculateBinaryOp([](int a, int b) { return a - b; }, l, r);
        case Op::MUL: return cg->CalculateBinaryOp([](int a, int b) { return a * b; }, l, r);
        case Op::DIV: return cg->CalculateBinaryOp([](int a, int b) { return a / b; }, l, r);
        case Op::MOD: return cg->CalculateBinaryOp([](int a, int b) { return a % b; }, l, r);
        case Op::LT:  return cg->CalculateBinaryOp([](int a, int b) { return a < b; }, l, r);
        case Op::GT:  return cg->CalculateBinaryOp([](int a, int b) { return a > b; }, l, r);
        case Op::LE:  return cg->CalculateBinaryOp([](int a, int b) { return a <= b; }, l, r);
        case Op::GE:  return cg->CalculateBinaryOp([](int a, int b) { return a >= b; }, l, r);
        case Op::EQ:  return cg->CalculateBinaryOp([](int a, int b) { return a == b; }, l, r);
        case Op::NE:  return cg->CalculateBinaryOp([](int a, int b) { return a != b; }, l, r);
    }
    return nullptr;
}

llvm::Value* LAndExprAST::ToValue(CodeGen* cg) {
    if (!left) return operand->ToValue(cg);

    auto* leftVal = left->ToValue(cg);
    auto* func = cg->GetFunction();
    auto* rightBB = cg->CreateBasicBlock("land_right", func);
    auto* endBB = cg->CreateBasicBlock("land_end", func);
    auto* result = cg->CreateAlloca(cg->GetInt32Type(), "land_result");
    auto* cond = cg->CreateICmpNE(leftVal, cg->GetInt32(0));

    cg->CreateStore(cond, result);
    cg->CreateCondBr(cond, rightBB, endBB);

    cg->SetInsertPoint(rightBB);
    cond = cg->CreateICmpNE(right->ToValue(cg), cg->GetInt32(0));
    cg->CreateStore(cond, result);
    cg->CreateBr(endBB);

    cg->SetInsertPoint(endBB);
    return cg->CreateLoad(result);
}

llvm::Value* LAndExprAST::ToNumber(CodeGen* cg) {
    if (!left) return operand->ToNumber(cg);
    return cg->CalculateBinaryOp([](int a, int b) { return a && b; }, left->ToNumber(cg), right->ToNumber(cg));
}

llvm::Value* LOrExprAST::ToValue(CodeGen* cg) {
    if (!left) return operand->ToValue(cg);

    auto* leftVal = left->ToValue(cg);
    auto* func = cg->GetFunction();
    auto* rightBB = cg->CreateBasicBlock("lor_right", func);
    auto* endBB = cg->CreateBasicBlock("lor_end", func);
    auto* result = cg->CreateAlloca(cg->GetInt32Type(), "lor_result");
    auto* cond = cg->CreateICmpNE(leftVal, cg->GetInt32(0));

    cg->CreateStore(cond, result);
    cg->CreateCondBr(cond, endBB, rightBB);

    cg->SetInsertPoint(rightBB);
    cond = cg->CreateICmpNE(right->ToValue(cg), cg->GetInt32(0));
    cg->CreateStore(cond, result);
    cg->CreateBr(endBB);

    cg->SetInsertPoint(endBB);
    return cg->CreateLoad(result);
}

llvm::Value* LOrExprAST::ToNumber(CodeGen* cg) {
    if (!left) return operand->ToNumber(cg);
    return cg->CalculateBinaryOp([](int a, int b) { return a || b; }, left->ToNumber(cg), right->ToNumber(cg));
}

void DeclAST::Codegen(CodeGen* cg) {
    if (constDecl) constDecl->Codegen(cg);
    else if (varDecl) varDecl->Codegen(cg);
}

void ConstDeclAST::Codegen(CodeGen* cg) {
    auto* type = btype->Codegen(cg);
    for (auto& def : constDefs) def->Codegen(cg, type);
}

void VarDeclAST::Codegen(CodeGen* cg) {
    auto* type = btype->Codegen(cg);
    for (auto& def : varDefs) def->Codegen(cg, type);
}

llvm::Value* ConstInitValAST::ToNumber(CodeGen* cg) {
    assert(!isArray);
    return expr->ToNumber(cg);
}

llvm::Value* InitValAST::ToValue(CodeGen* cg, llvm::Value* addr) {
    return expr->ToValue(cg);
}

llvm::Value* InitValAST::ToNumber(CodeGen* cg, vector<int> shape, int dim) {
    if (!isArray) return expr->ToNumber(cg);

    vector<llvm::Value*> values;
    llvm::Type* type{};
    auto size = shape[dim];
    for (int i = 0; i < size; ++i) {
        auto* val = i < (int)subVals.size() ? subVals[i]->ToNumber(cg, shape, dim + 1) : cg->CreateZero(type);
        values.push_back(val);
        type = cg->GetValueType(val);
    }
    return cg->CreateArray(cg->GetArrayType(type, size), values);
}

void BlockItemAST::ToValue(CodeGen* cg) {
    if (decl) decl->Codegen(cg);
    else if (stmt) stmt->Codegen(cg);
}

llvm::Value* LValAST::ToValue(CodeGen* cg) {
    auto* ptr = ToPointer(cg);
    auto* type = cg->GetValueType(ptr);
    if (cg->IsPointerType(type)) {
        auto* elemType = cg->GetAllocatedType(ptr);
        if (elemType && cg->IsArrayType(elemType))
            return cg->CreateGEP(elemType, ptr, {cg->GetInt32(0)});
    }
    return cg->CreateLoad(ptr);
}

llvm::Value* LValAST::ToNumber(CodeGen* cg) {
    return cg->GetBaseValue(cg->GetSymbolValue(ident).value);
}

llvm::Value* LValAST::ToPointer(CodeGen* cg) {
    auto* value = cg->GetSymbolValue(ident).value;
    if (!indies.empty()) {
        vector<llvm::Value*> indexVals;
        for (auto& index : indies) indexVals.push_back(index->ToValue(cg));
        value = cg->CreateGEP(cg->GetInt32Type(), value, indexVals);
    }
    return value;
}

llvm::Value* ConstExprAST::ToValue(CodeGen* cg)  { return expr->ToValue(cg); }
llvm::Value* ConstExprAST::ToNumber(CodeGen* cg) { return expr->ToNumber(cg); }
int ConstExprAST::ToInteger(CodeGen* cg) { return cg->GetValueInt(expr->ToNumber(cg)); }

llvm::Type* FuncFParamAST::ToType(CodeGen* cg) {
    llvm::Type* type = btype->Codegen(cg);
    if (isArray) {
        for (auto& sizeExpr : sizeExprs)
            type = cg->GetArrayType(type, sizeExpr->ToInteger(cg));
        type = cg->GetPointerType(type);
    }
    return type;
}

llvm::Value* FuncFParamAST::Alloca(CodeGen* cg, llvm::Type* type) {
    auto* addr = cg->CreateAlloca(type, ident);
    cg->AddSymbol(ident, VAR_TYPE::VAR, {.value = addr});
    return addr;
}

llvm::Value* FuncRParamAST::ToValue(CodeGen* cg) { return expr->ToValue(cg); }
