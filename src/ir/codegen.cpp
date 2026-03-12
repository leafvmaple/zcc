#include "codegen.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Support/raw_os_ostream.h"

#include <fstream>

// --- Lifecycle ---

CodeGen::CodeGen(const std::string& moduleName)
    : Module(moduleName, Context), Builder(Context) {
    EnterScope();
}

void CodeGen::Optimize() {
#if 0
    llvm::PassBuilder pb;
    llvm::FunctionAnalysisManager fam;
    llvm::ModuleAnalysisManager mam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::LoopAnalysisManager lam;

    pb.registerModuleAnalyses(mam);
    pb.registerFunctionAnalyses(fam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);

    llvm::FunctionPassManager fpm;
    fpm.addPass(llvm::ADCEPass());
    fpm.addPass(llvm::SimplifyCFGPass());

    llvm::ModulePassManager mpm;
    mpm.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(fpm)));
    mpm.run(Module, mam);
#endif
}

void CodeGen::Print() {
    Module.print(llvm::outs(), nullptr);
}

void CodeGen::Dump(const char* output) {
    std::ofstream outFile(output);
    llvm::raw_os_ostream rawOutFile(outFile);
    Module.print(rawOutFile, nullptr);
}

// --- Types ---

llvm::FunctionType* CodeGen::CreateFuncType(llvm::Type* retType, std::vector<llvm::Type*> params) {
    return llvm::FunctionType::get(retType, params, false);
}

llvm::Type* CodeGen::GetInt32Type() { return llvm::Type::getInt32Ty(Context); }
llvm::Type* CodeGen::GetInt8Type()  { return llvm::Type::getInt8Ty(Context); }
llvm::Type* CodeGen::GetVoidType()  { return llvm::Type::getVoidTy(Context); }

llvm::Type* CodeGen::GetArrayType(llvm::Type* type, int size) {
    return llvm::ArrayType::get(type, size);
}

llvm::Type* CodeGen::GetPointerType(llvm::Type* type) {
    return llvm::PointerType::get(type, 0);
}

llvm::Type* CodeGen::GetValueType(llvm::Value* value) {
    return value->getType();
}

llvm::Type* CodeGen::GetElementType(llvm::Type* type) {
    return type->isArrayTy() ? type->getArrayElementType() : nullptr;
}

llvm::Type* CodeGen::GetAllocatedType(llvm::Value* value) {
    if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(value))
        return alloca->getAllocatedType();
    if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(value))
        return gep->getSourceElementType();
    if (auto* gv = llvm::dyn_cast<llvm::GlobalVariable>(value))
        return gv->getValueType();
    return nullptr;
}

bool CodeGen::IsArrayType(llvm::Type* type)   { return type->isArrayTy(); }
bool CodeGen::IsPointerType(llvm::Type* type) { return type->isPointerTy(); }

// --- Functions ---

llvm::BasicBlock* CodeGen::CreateBasicBlock(const std::string& name, llvm::Function* func) {
    return llvm::BasicBlock::Create(Context, name, func);
}

llvm::Function* CodeGen::CreateFunction(llvm::FunctionType* funcType, const std::string& name, std::vector<std::string> names) {
    auto* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, &Module);
    auto args = func->arg_begin();
    for (size_t i = 0; i < names.size(); ++i) {
        args->setName(names[i]);
        ++args;
    }
    AddSymbol(name, VAR_TYPE::FUNC, { func });
    return func;
}

void CodeGen::CreateBuiltin(const std::string& name, llvm::Type* retType, std::vector<llvm::Type*> params) {
    auto* funcType = llvm::FunctionType::get(retType, params, false);
    auto* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, Module);
    AddSymbol(name, VAR_TYPE::FUNC, { func });
}

llvm::Function* CodeGen::GetFunction() {
    return Builder.GetInsertBlock()->getParent();
}

llvm::Value* CodeGen::GetFunctionArg(int index) {
    auto* func = GetFunction();
    auto argIt = func->arg_begin();
    std::advance(argIt, index);
    return &(*argIt);
}

// --- Memory ---

llvm::Value* CodeGen::CreateAlloca(llvm::Type* type, const std::string& name) {
    return Builder.CreateAlloca(type, nullptr, name);
}

llvm::Value* CodeGen::CreateGlobal(llvm::Type* type, const std::string& name, llvm::Value* init) {
    llvm::Constant* initVal = init ? llvm::dyn_cast<llvm::Constant>(init) : llvm::Constant::getNullValue(type);
    return new llvm::GlobalVariable(Module, type, false, llvm::GlobalValue::ExternalLinkage, initVal, name);
}

void CodeGen::CreateStore(llvm::Value* value, llvm::Value* dest) {
    Builder.CreateStore(value, dest);
}

llvm::Value* CodeGen::CreateLoad(llvm::Value* src) {
    llvm::Type* loadType = GetAllocatedType(src);
    if (!loadType) loadType = GetInt32Type();
    return Builder.CreateLoad(loadType, src);
}

llvm::Value* CodeGen::CreateGEP(llvm::Type* type, llvm::Value* array, std::vector<llvm::Value*> index) {
    return Builder.CreateGEP(type, array, index);
}

// --- Constants ---

llvm::Value* CodeGen::GetInt32(int value) { return llvm::ConstantInt::get(GetInt32Type(), value); }
llvm::Value* CodeGen::GetInt8(int value)  { return llvm::ConstantInt::get(GetInt8Type(), value); }

llvm::Value* CodeGen::CreateZero(llvm::Type* type) {
    return llvm::Constant::getNullValue(type);
}

llvm::Value* CodeGen::CreateArray(llvm::Type* type, std::vector<llvm::Value*> values) {
    std::vector<llvm::Constant*> constants;
    for (auto* val : values) {
        if (auto* c = llvm::dyn_cast<llvm::Constant>(val))
            constants.push_back(c);
    }
    return llvm::ConstantArray::get((llvm::ArrayType*)type, constants);
}

llvm::Value* CodeGen::CreateGlobalString(const std::string& str) {
    return Builder.CreateGlobalStringPtr(str);
}

// --- Arithmetic ---

llvm::Value* CodeGen::CreateAdd(llvm::Value* lhs, llvm::Value* rhs) { return Builder.CreateAdd(lhs, rhs); }
llvm::Value* CodeGen::CreateSub(llvm::Value* lhs, llvm::Value* rhs) { return Builder.CreateSub(lhs, rhs); }
llvm::Value* CodeGen::CreateMul(llvm::Value* lhs, llvm::Value* rhs) { return Builder.CreateMul(lhs, rhs); }
llvm::Value* CodeGen::CreateDiv(llvm::Value* lhs, llvm::Value* rhs) { return Builder.CreateSDiv(lhs, rhs); }
llvm::Value* CodeGen::CreateMod(llvm::Value* lhs, llvm::Value* rhs) { return Builder.CreateSRem(lhs, rhs); }
llvm::Value* CodeGen::CreateAnd(llvm::Value* lhs, llvm::Value* rhs) { return Builder.CreateAnd(lhs, rhs); }
llvm::Value* CodeGen::CreateOr(llvm::Value* lhs, llvm::Value* rhs)  { return Builder.CreateOr(lhs, rhs); }

// --- Comparisons ---

llvm::Value* CodeGen::CreateICmpNE(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateZExt(Builder.CreateICmpNE(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpEQ(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateZExt(Builder.CreateICmpEQ(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpLT(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateZExt(Builder.CreateICmpSLT(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpGT(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateZExt(Builder.CreateICmpSGT(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpLE(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateZExt(Builder.CreateICmpSLE(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpGE(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateZExt(Builder.CreateICmpSGE(lhs, rhs), GetInt32Type());
}

// --- Control flow ---

void CodeGen::CreateCondBr(llvm::Value* cond, llvm::BasicBlock* trueBB, llvm::BasicBlock* falseBB) {
    auto* boolCond = Builder.CreateICmpNE(cond, GetInt32(0));
    Builder.CreateCondBr(boolCond, trueBB, falseBB);
}

void CodeGen::CreateBr(llvm::BasicBlock* dest) { Builder.CreateBr(dest); }

void CodeGen::CreateRet(llvm::Value* value) {
    if (value)
        Builder.CreateRet(value);
    else
        Builder.CreateRetVoid();
}

llvm::Value* CodeGen::CreateCall(llvm::Function* func, std::vector<llvm::Value*> args) {
    return Builder.CreateCall(func, args);
}

void CodeGen::SetInsertPoint(llvm::BasicBlock* bb) { Builder.SetInsertPoint(bb); }

bool CodeGen::EndWithTerminator() {
    auto* bb = Builder.GetInsertBlock();
    return !bb->empty() && bb->back().isTerminator();
}

// --- Type conversions ---

llvm::Value* CodeGen::CreateTrunc(llvm::Value* value, llvm::Type* type) { return Builder.CreateTrunc(value, type); }
llvm::Value* CodeGen::CreateZExt(llvm::Value* value, llvm::Type* type)  { return Builder.CreateZExt(value, type); }

// --- Value utilities ---

llvm::Value* CodeGen::CalculateBinaryOp(const std::function<int(int, int)>& func, llvm::Value* lhs, llvm::Value* rhs) {
    auto* lhsConst = llvm::dyn_cast<llvm::ConstantInt>(lhs);
    auto* rhsConst = llvm::dyn_cast<llvm::ConstantInt>(rhs);
    return GetInt32(func(lhsConst->getSExtValue(), rhsConst->getSExtValue()));
}

int CodeGen::GetValueInt(llvm::Value* value) {
    if (auto* ci = llvm::dyn_cast<llvm::ConstantInt>(value))
        return ci->getSExtValue();
    return 0;
}

llvm::Value* CodeGen::GetArrayElement(llvm::Value* array, int index) {
    auto* arrayType = llvm::dyn_cast<llvm::ArrayType>(array->getType());
    return Builder.CreateGEP(arrayType->getElementType(), array, {GetInt32(index)});
}

llvm::Value* CodeGen::GetBaseValue(llvm::Value* value) { return value; }

// --- Scope management ---

void CodeGen::EnterScope() { locals.push_back({}); types.push_back({}); }
void CodeGen::ExitScope()  { locals.pop_back(); types.pop_back(); }
bool CodeGen::IsGlobalScope() const { return locals.size() == 1; }

void CodeGen::AddSymbol(const std::string& name, VAR_TYPE type, SymbolValue value) {
    locals.back()[name] = value;
    types.back()[value] = type;
}

CodeGen::SymbolValue CodeGen::GetSymbolValue(const std::string& name) {
    for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }
    return { nullptr };
}

VAR_TYPE CodeGen::GetSymbolType(SymbolValue value) {
    for (auto it = types.rbegin(); it != types.rend(); ++it) {
        auto found = it->find(value);
        if (found != it->end()) return found->second;
    }
    return VAR_TYPE::VAR;
}

// --- While loop tracking ---

void CodeGen::EnterWhile(llvm::BasicBlock* entry, llvm::BasicBlock* end) { whiles.push_back({entry, end}); }
void CodeGen::ExitWhile() { whiles.pop_back(); }
llvm::BasicBlock* CodeGen::GetWhileEntry() { return whiles.back().entry; }
llvm::BasicBlock* CodeGen::GetWhileEnd()   { return whiles.back().end; }
