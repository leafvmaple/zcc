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

UnaryExprAST::UnaryExprAST(TYPE type, unique_ptr<BaseType>&& castType, unique_ptr<UnaryExprAST>&& operand)
    : type(type), unaryExpr(std::move(operand)), castType(std::move(castType)) {}
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

ConstExprAST::ConstExprAST(unique_ptr<ExprAST>&& expr) : expr(std::move(expr)) {}

FuncFParamAST::FuncFParamAST(unique_ptr<BaseType>&& btype, string ident, bool isArray)
    : btype(std::move(btype)), ident(std::move(ident)), isArray(isArray) {}

FuncFParamAST::FuncFParamAST(unique_ptr<BaseType>&& btype, string ident, vector<unique_ptr<ConstExprAST>>&& sizeExprs)
    : btype(std::move(btype)), ident(std::move(ident)), sizeExprs(std::move(sizeExprs)), isArray(true) {}

FuncRParamAST::FuncRParamAST(unique_ptr<ExprAST>&& expr) : expr(std::move(expr)) {}

// ========== Typed declarations and initialization ==========

namespace {
TypeRef DeclaredType(CodeGen* cg, TypeRef type, const vector<unique_ptr<ConstExprAST>>& dimensions) {
    for (auto it = dimensions.rbegin(); it != dimensions.rend(); ++it) {
        int count = (*it)->ToInteger(cg);
        if (count <= 0) throw std::runtime_error("array extent must be positive");
        type = BaseType::Array(type, count);
    }
    return type;
}
template<class Init>
llvm::Value* InitialValue(CodeGen* cg, TypeRef type, Init* init, bool constant) {
    if (!init) return cg->CreateZero(type->Codegen(cg));
    if (type->type != BaseType::TYPE::ARRAY) {
        if (init->isArray) throw std::runtime_error("aggregate initializer for scalar");
        return cg->Cast(constant ? init->expr->ToNumber(cg) : init->expr->ToValue(cg), type);
    }
    if (!init->isArray) throw std::runtime_error("array initializer requires braces");
    vector<llvm::Value*> values;
    for (auto& element : init->subVals) values.push_back(InitialValue(cg, type->element, element.get(), constant));
    return cg->Initializer(type, values);
}
template<class Init>
void InitializeLocal(CodeGen* cg, llvm::Value* address, TypeRef type, Init* init) {
    if (type->type != BaseType::TYPE::ARRAY) {
        cg->CreateStore(init ? init->expr->ToValue(cg) : cg->CreateZero(type->Codegen(cg)), address);
        return;
    }
    if (init && (!init->isArray || init->subVals.size() > static_cast<size_t>(type->count)))
        throw std::runtime_error("invalid array initializer");
    for (int i = 0; i < type->count; ++i) {
        auto* element = cg->Index(address, cg->GetInt32(i));
        InitializeLocal(cg, element, type->element, init && i < static_cast<int>(init->subVals.size()) ? init->subVals[i].get() : nullptr);
    }
}
}
void ConstDefAST::Codegen(CodeGen* cg, TypeRef type) {
    type = DeclaredType(cg, type, sizeExprs);
    auto* init = InitialValue(cg, type, initVal.get(), true);
    if (cg->IsGlobalScope()) {
        cg->AddSymbol(ident, VAR_TYPE::CONST, {cg->Global(type, ident, init)});
    } else if (type->type == BaseType::TYPE::ARRAY || type->type == BaseType::TYPE::STRUCT) {
        auto* var = cg->Alloca(type, ident);
        cg->CreateStore(init, var);
        cg->AddSymbol(ident, VAR_TYPE::CONST, {var});
    } else cg->AddSymbol(ident, VAR_TYPE::CONST, {init});
}
void VarDefAST::Codegen(CodeGen* cg, TypeRef type) {
    type = DeclaredType(cg, type, sizeExprs);
    if (cg->IsGlobalScope()) {
        auto* init = InitialValue(cg, type, initVal.get(), true);
        cg->AddSymbol(ident, VAR_TYPE::GLOBAL, {cg->Global(type, ident, init)});
    } else {
        auto* var = cg->Alloca(type, ident);
        if (initVal) InitializeLocal(cg, var, type, initVal.get());
        cg->AddSymbol(ident, VAR_TYPE::VAR, {var});
    }
}

void CompUnitAST::Codegen(CodeGen* cg) {
    auto* intType = cg->GetInt32Type();
    auto* ptrType = cg->GetPointerType(cg->GetInt8Type());

    cg->CreateBuiltin("printf", intType, {ptrType}, true);
    cg->CreateBuiltin("scanf", intType, {ptrType}, true);

    for (auto& record : structs) cg->DefineStruct(record.first, record.second);
    for (auto& func : funcDefs) {
        vector<TypeRef> params;
        for (auto& param : func->params) params.push_back(param->SourceType(cg));
        cg->DeclareFunction(func->ident, func->funcType->Copy(), params);
    }
    for (auto& decl : decls) decl->Codegen(cg);
    for (auto& funcDef : funcDefs) if (funcDef->block) funcDef->Codegen(cg);
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

    for (size_t i = 0; i < params.size(); ++i) {
        cg->Annotate(cg->GetFunctionArg(i), params[i]->SourceType(cg));
        cg->CreateStore(cg->GetFunctionArg(i), params[i]->Alloca(cg, paramTypes[i]));
    }

    block->Codegen(cg);
    if (!cg->EndWithTerminator()) cg->CreateRet(nullptr);
    cg->ExitScope();
}

void BlockAST::Codegen(CodeGen* cg) {
    cg->EnterScope();
    for (auto& item : items) {
        if (cg->EndWithTerminator()) break;
        item->ToValue(cg);
    }
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
    case TYPE::Cast: return cg->Cast(unaryExpr->ToValue(cg), castType->Copy());
    case TYPE::Sizeof: return cg->SizeOf(castType->Copy());
    case TYPE::Unary: {
        if (op == OP::ADDRESS) return unaryExpr->ToPointer(cg);
        auto* val = unaryExpr->ToValue(cg);
        switch (op) {
            case OP::ADDRESS:
                throw std::runtime_error("invalid address operation");
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
    case TYPE::Cast: return cg->Cast(unaryExpr->ToNumber(cg), castType->Copy());
    case TYPE::Sizeof: return cg->SizeOf(castType->Copy());
    case TYPE::Unary: {
        auto* val = unaryExpr->ToNumber(cg);
        switch (op) {
            case OP::ADDRESS:
                throw std::runtime_error("invalid address operation");
            case OP::PLUS:  return val;
            case OP::MINUS: return cg->CalculateBinaryOp([](int a, int b) { return a - b; }, cg->GetInt32(0), val);
            case OP::NOT:   return cg->CalculateBinaryOp([](int a, int b) { return a == b; }, val, cg->GetInt32(0));
        }
    }
    case TYPE::Call:
        throw std::runtime_error("call is not a constant expression");
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
        case Op::BIT_AND: return cg->CreateAnd(l, r);
        case Op::BIT_OR: return cg->CreateOr(l, r);
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
        case Op::BIT_AND: return cg->CalculateBinaryOp([](int a, int b) { return a & b; }, l, r);
        case Op::BIT_OR: return cg->CalculateBinaryOp([](int a, int b) { return a | b; }, l, r);
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
    auto type = btype->Copy();
    for (auto& def : constDefs) def->Codegen(cg, type);
}

void VarDeclAST::Codegen(CodeGen* cg) {
    auto type = btype->Copy();
    for (auto& def : varDefs) def->Codegen(cg, type);
}

void BlockItemAST::ToValue(CodeGen* cg) {
    if (decl) decl->Codegen(cg);
    else if (stmt) stmt->Codegen(cg);
}

llvm::Value* UnaryExprAST::ToPointer(CodeGen* cg) {
    if (type == TYPE::Primary && primaryExpr->type == PrimaryExprAST::TYPE::LVal)
        return primaryExpr->lval->ToPointer(cg);
    throw std::runtime_error("address of a non-lvalue");
}
LValAST::LValAST(unique_ptr<LValAST>&& base, string member) : base(std::move(base)), member(std::move(member)) {}
LValAST::LValAST(unique_ptr<UnaryExprAST>&& pointer) : pointer(std::move(pointer)) {}
LValAST::LValAST(unique_ptr<LValAST>&& base, unique_ptr<ExprAST>&& index) : index(std::move(index)), base(std::move(base)) {}
llvm::Value* LValAST::ToValue(CodeGen* cg) {
    if (!base && !pointer && !index && cg->GetSymbolType(cg->GetSymbolValue(ident)) == VAR_TYPE::CONST) {
        auto* value = cg->GetSymbolValue(ident).value;
        if (llvm::isa<llvm::ConstantInt>(value)) return value;
    }
    return cg->Decay(ToPointer(cg));
}
llvm::Value* LValAST::ToNumber(CodeGen* cg) {
    auto* value = cg->GetSymbolValue(ident).value;
    if (auto* global = llvm::dyn_cast_or_null<llvm::GlobalVariable>(value)) return global->getInitializer();
    if (!value || !llvm::isa<llvm::Constant>(value)) throw std::runtime_error("nonconstant expression");
    return value;
}
llvm::Value* LValAST::ToPointer(CodeGen* cg) {
    auto* value = pointer ? pointer->ToValue(cg) : (base ? base->ToPointer(cg) : cg->GetSymbolValue(ident).value);
    if (!value) throw std::runtime_error("undeclared identifier: " + ident);
    if (!member.empty()) value = cg->Member(value, member);
    if (index) value = cg->Index(value, index->ToValue(cg));
    return value;
}

llvm::Value* ConstExprAST::ToValue(CodeGen* cg)  { return expr->ToValue(cg); }
llvm::Value* ConstExprAST::ToNumber(CodeGen* cg) { return expr->ToNumber(cg); }
int ConstExprAST::ToInteger(CodeGen* cg) { return cg->GetValueInt(expr->ToNumber(cg)); }

TypeRef FuncFParamAST::SourceType(CodeGen* cg) {
    auto type = DeclaredType(cg, btype->Copy(), sizeExprs);
    return isArray ? BaseType::Pointer(type) : type;
}
llvm::Type* FuncFParamAST::ToType(CodeGen* cg) { return SourceType(cg)->Codegen(cg); }
llvm::Value* FuncFParamAST::Alloca(CodeGen* cg, llvm::Type*) {
    auto* addr = cg->Alloca(SourceType(cg), ident);
    cg->AddSymbol(ident, VAR_TYPE::VAR, {.value = addr});
    return addr;
}

llvm::Value* FuncRParamAST::ToValue(CodeGen* cg) { return expr->ToValue(cg); }
