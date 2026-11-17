#include "codegen.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/ADCE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Support/raw_os_ostream.h"

#include <fstream>
#include "llvm/Config/llvm-config.h"
#if __has_include("llvm/TargetParser/Triple.h")
#include "llvm/TargetParser/Triple.h"
#else
#include "llvm/ADT/Triple.h"
#endif

// --- Lifecycle ---

CodeGen::CodeGen(const std::string& moduleName)
    : Module(moduleName, Context), Builder(Context) {
    EnterScope();
}

void CodeGen::Optimize() {
    if (llvm::verifyModule(Module, &llvm::errs())) throw std::runtime_error("invalid generated LLVM IR");
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
    return llvm::PointerType::get(Context, 0);
}

llvm::Type* CodeGen::GetValueType(llvm::Value* value) {
    return value->getType();
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

bool CodeGen::IsPointerType(llvm::Type* type) { return type->isPointerTy(); }

// --- Functions ---

llvm::BasicBlock* CodeGen::CreateBasicBlock(const std::string& name, llvm::Function* func) {
    return llvm::BasicBlock::Create(Context, name, func);
}

llvm::Function* CodeGen::CreateFunction(llvm::FunctionType* funcType, const std::string& name, std::vector<std::string> names) {
    auto* func = Module.getFunction(name);
    if (func) {
        if (func->getFunctionType() != funcType || !func->empty()) throw std::runtime_error("conflicting function: " + name);
    } else func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, &Module);
    auto args = func->arg_begin();
    for (size_t i = 0; i < names.size(); ++i) {
        args->setName(names[i]);
        ++args;
    }
    AddSymbol(name, VAR_TYPE::FUNC, { func });
    return func;
}

void CodeGen::CreateBuiltin(const std::string& name, llvm::Type* retType, std::vector<llvm::Type*> params, bool isVarArg) {
    auto* funcType = llvm::FunctionType::get(retType, params, isVarArg);
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
    // All supported automatic objects have fixed size: reserve once per call,
    // even when the declaration or short-circuit expression sits inside a loop.
    auto& entry = GetFunction()->getEntryBlock();
    llvm::IRBuilder<> allocations(&entry, entry.begin());
    return allocations.CreateAlloca(type, nullptr, name);
}

llvm::Value* CodeGen::CreateGlobal(llvm::Type* type, const std::string& name, llvm::Value* init) {
    llvm::Constant* initVal = init ? llvm::dyn_cast<llvm::Constant>(init) : llvm::Constant::getNullValue(type);
    return new llvm::GlobalVariable(Module, type, false, llvm::GlobalValue::ExternalLinkage, initVal, name);
}

void CodeGen::CreateStore(llvm::Value* value, llvm::Value* dest) {
    auto type = TypeOf(dest);
    if (type && type->type == BaseType::TYPE::POINTER) value = Cast(value, type->element);
    Builder.CreateStore(value, dest);
}

llvm::Value* CodeGen::CreateLoad(llvm::Value* src) {
    auto type = TypeOf(src);
    if (type && type->type == BaseType::TYPE::POINTER)
        return Annotate(Builder.CreateLoad(type->element->Codegen(this), src), type->element);
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

llvm::Value* CodeGen::CreateGlobalString(const std::string& str) {
    return Annotate(Builder.CreateGlobalString(str, "", 0, &Module), BaseType::Pointer(std::make_shared<BaseType>(BaseType::TYPE::CHAR)));
}

// --- Arithmetic ---

llvm::Value* CodeGen::CreateAdd(llvm::Value* lhs, llvm::Value* rhs) {
    if (lhs->getType()->isPointerTy()) {
        auto type = TypeOf(lhs);
        if (!type || !rhs->getType()->isIntegerTy()) throw std::runtime_error("invalid pointer addition");
        return Annotate(Builder.CreateGEP(type->element->Codegen(this), lhs, rhs), type);
    }
    if (rhs->getType()->isPointerTy()) return CreateAdd(rhs, lhs);
    CommonIntegers(lhs, rhs); return Builder.CreateAdd(lhs, rhs);
}
llvm::Value* CodeGen::CreateSub(llvm::Value* lhs, llvm::Value* rhs) {
    if (lhs->getType()->isPointerTy() && rhs->getType()->isIntegerTy())
        return CreateAdd(lhs, Builder.CreateNeg(rhs));
    CommonIntegers(lhs, rhs); return Builder.CreateSub(lhs, rhs);
}
llvm::Value* CodeGen::CreateMul(llvm::Value* lhs, llvm::Value* rhs) { CommonIntegers(lhs, rhs); return Builder.CreateMul(lhs, rhs); }
llvm::Value* CodeGen::CreateDiv(llvm::Value* lhs, llvm::Value* rhs) { CommonIntegers(lhs, rhs); return Builder.CreateSDiv(lhs, rhs); }
llvm::Value* CodeGen::CreateMod(llvm::Value* lhs, llvm::Value* rhs) { CommonIntegers(lhs, rhs); return Builder.CreateSRem(lhs, rhs); }
llvm::Value* CodeGen::CreateAnd(llvm::Value* lhs, llvm::Value* rhs) { CommonIntegers(lhs, rhs); return Builder.CreateAnd(lhs, rhs); }
llvm::Value* CodeGen::CreateOr(llvm::Value* lhs, llvm::Value* rhs)  { return Builder.CreateOr(lhs, rhs); }

// --- Comparisons ---

llvm::Value* CodeGen::CreateICmpNE(llvm::Value* lhs, llvm::Value* rhs) {
    CommonIntegers(lhs, rhs);
    return Builder.CreateZExt(Builder.CreateICmpNE(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpEQ(llvm::Value* lhs, llvm::Value* rhs) {
    CommonIntegers(lhs, rhs);
    return Builder.CreateZExt(Builder.CreateICmpEQ(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpLT(llvm::Value* lhs, llvm::Value* rhs) {
    CommonIntegers(lhs, rhs);
    return Builder.CreateZExt(Builder.CreateICmpSLT(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpGT(llvm::Value* lhs, llvm::Value* rhs) {
    CommonIntegers(lhs, rhs);
    return Builder.CreateZExt(Builder.CreateICmpSGT(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpLE(llvm::Value* lhs, llvm::Value* rhs) {
    CommonIntegers(lhs, rhs);
    return Builder.CreateZExt(Builder.CreateICmpSLE(lhs, rhs), GetInt32Type());
}
llvm::Value* CodeGen::CreateICmpGE(llvm::Value* lhs, llvm::Value* rhs) {
    CommonIntegers(lhs, rhs);
    return Builder.CreateZExt(Builder.CreateICmpSGE(lhs, rhs), GetInt32Type());
}

// --- Control flow ---

void CodeGen::CreateCondBr(llvm::Value* cond, llvm::BasicBlock* trueBB, llvm::BasicBlock* falseBB) {
    auto* boolCond = Builder.CreateICmpNE(cond, llvm::Constant::getNullValue(cond->getType()));
    Builder.CreateCondBr(boolCond, trueBB, falseBB);
}

void CodeGen::CreateBr(llvm::BasicBlock* dest) { if (!EndWithTerminator()) Builder.CreateBr(dest); }

void CodeGen::CreateRet(llvm::Value* value) {
    auto* result = GetFunction()->getReturnType();
    if (result->isVoidTy()) Builder.CreateRetVoid();
    else {
        if (!value) value = llvm::Constant::getNullValue(result);
        auto found = returnTypes.find(GetFunction());
        if (found != returnTypes.end()) value = Cast(value, found->second);
        Builder.CreateRet(value);
    }
}

llvm::Value* CodeGen::CreateCall(llvm::Function* func, std::vector<llvm::Value*> args) {
    if (!func) throw std::runtime_error("call to undeclared function");
    auto found = parameterTypes.find(func);
    if (args.size() < func->arg_size() || (!func->isVarArg() && args.size() != func->arg_size()))
        throw std::runtime_error("wrong argument count for " + func->getName().str());
    for (size_t i = 0; i < args.size(); ++i) {
        if (found != parameterTypes.end() && i < found->second.size()) args[i] = Cast(args[i], found->second[i]);
        else if (i < func->arg_size()) {
            auto* target = func->getFunctionType()->getParamType(i);
            if (target->isIntegerTy() && args[i]->getType()->isIntegerTy()) args[i] = Builder.CreateIntCast(args[i], target, true);
        } else if (args[i]->getType()->isIntegerTy(8)) args[i] = Builder.CreateSExt(args[i], GetInt32Type());
    }
    auto* result = Builder.CreateCall(func, args);
    return found != parameterTypes.end() ? Annotate(result, returnTypes.at(func)) : result;
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

// Source type operations shared by declarations, parameters and expressions.
llvm::Type* CodeGen::GetInt64Type() { return llvm::Type::getInt64Ty(Context); }
void CodeGen::SetTarget(const std::string& target) {
#if LLVM_VERSION_MAJOR >= 21
    Module.setTargetTriple(llvm::Triple(target));
#else
    Module.setTargetTriple(target);
#endif
    Module.setDataLayout("e-p:64:64-i64:64-n8:16:32:64-S128");
}
TypeRef CodeGen::TypeOf(llvm::Value* value) {
    auto it = sourceTypes.find(value);
    if (it != sourceTypes.end()) return it->second;
    if (value->getType()->isIntegerTy(8)) return std::make_shared<BaseType>(BaseType::TYPE::CHAR);
    if (value->getType()->isIntegerTy(64)) return std::make_shared<BaseType>(BaseType::TYPE::LONG);
    if (value->getType()->isIntegerTy()) return std::make_shared<BaseType>(BaseType::TYPE::INT);
    return nullptr;
}
llvm::Value* CodeGen::Annotate(llvm::Value* value, TypeRef type) { sourceTypes[value] = std::move(type); return value; }
llvm::Value* CodeGen::Cast(llvm::Value* value, TypeRef type) {
    auto* target = type->Codegen(this);
    if (value->getType() == target) return Annotate(value, type);
    if (target->isIntegerTy() && value->getType()->isIntegerTy()) {
        if (auto* constant = llvm::dyn_cast<llvm::ConstantInt>(value)) return Annotate(llvm::ConstantInt::get(target, constant->getValue().sextOrTrunc(target->getIntegerBitWidth())), type);
        return Annotate(Builder.CreateIntCast(value, target, true), type);
    }
    if (target->isPointerTy() && value->getType()->isIntegerTy()) {
        if (auto* constant = llvm::dyn_cast<llvm::Constant>(value)) return Annotate(llvm::ConstantExpr::getIntToPtr(constant, target), type);
        return Annotate(Builder.CreateIntToPtr(value, target), type);
    }
    if (target->isIntegerTy() && value->getType()->isPointerTy()) return Annotate(Builder.CreatePtrToInt(value, target), type);
    throw std::runtime_error("incompatible C types");
}
void CodeGen::CommonIntegers(llvm::Value*& lhs, llvm::Value*& rhs) {
    if (lhs->getType()->isPointerTy() || rhs->getType()->isPointerTy()) {
        if (!lhs->getType()->isPointerTy()) lhs = Cast(lhs, TypeOf(rhs));
        if (!rhs->getType()->isPointerTy()) rhs = Cast(rhs, TypeOf(lhs));
        return;
    }
    auto* target = lhs->getType()->getIntegerBitWidth() > 32 || rhs->getType()->getIntegerBitWidth() > 32
        ? GetInt64Type() : GetInt32Type();
    lhs = Builder.CreateIntCast(lhs, target, true);
    rhs = Builder.CreateIntCast(rhs, target, true);
}
llvm::Value* CodeGen::Alloca(TypeRef type, const std::string& name) {
    return Annotate(CreateAlloca(type->Codegen(this), name), BaseType::Pointer(type));
}
llvm::Value* CodeGen::Global(TypeRef type, const std::string& name, llvm::Value* init) {
    if (init) init = Cast(init, type);
    return Annotate(CreateGlobal(type->Codegen(this), name, init), BaseType::Pointer(type));
}
llvm::Value* CodeGen::Decay(llvm::Value* address) {
    auto type = TypeOf(address)->element;
    if (type->type != BaseType::TYPE::ARRAY) return CreateLoad(address);
    return Annotate(Builder.CreateGEP(type->Codegen(this), address, {GetInt32(0), GetInt32(0)}), BaseType::Pointer(type->element));
}
llvm::Value* CodeGen::Index(llvm::Value* address, llvm::Value* index) {
    auto type = TypeOf(address)->element;
    if (type->type == BaseType::TYPE::ARRAY)
        return Annotate(Builder.CreateGEP(type->Codegen(this), address, {GetInt32(0), index}), BaseType::Pointer(type->element));
    if (type->type != BaseType::TYPE::POINTER) throw std::runtime_error("indexing a non-array/pointer");
    return Annotate(Builder.CreateGEP(type->element->Codegen(this), CreateLoad(address), index), type);
}
void CodeGen::DefineStruct(const std::string& name, const std::vector<StructField>& fields) {
    if (structs.count(name)) throw std::runtime_error("duplicate struct: " + name);
    structs[name] = fields;
    auto* type = llvm::StructType::create(Context, name);
    std::vector<llvm::Type*> members;
    for (auto& field : fields) members.push_back(field.type->Codegen(this));
    type->setBody(members);
}
llvm::Type* CodeGen::StructType(const std::string& name) {
    auto* type = llvm::StructType::getTypeByName(Context, name);
    if (!type) throw std::runtime_error("unknown struct: " + name);
    return type;
}
llvm::Value* CodeGen::Member(llvm::Value* address, const std::string& name) {
    auto source = TypeOf(address)->element;
    if (source->type != BaseType::TYPE::STRUCT) throw std::runtime_error("member of a non-struct");
    auto& fields = structs.at(source->tag);
    for (size_t i = 0; i < fields.size(); ++i)
        if (fields[i].name == name)
            return Annotate(Builder.CreateStructGEP(source->Codegen(this), address, i), BaseType::Pointer(fields[i].type));
    throw std::runtime_error("unknown member: " + name);
}
llvm::Value* CodeGen::SizeOf(TypeRef type) {
    return llvm::ConstantInt::get(GetInt64Type(), Module.getDataLayout().getTypeAllocSize(type->Codegen(this)));
}
void CodeGen::DeclareFunction(const std::string& name, TypeRef result, const std::vector<TypeRef>& params) {
    std::vector<llvm::Type*> types;
    for (auto& param : params) types.push_back(param->Codegen(this));
    auto* functionType = CreateFuncType(result->Codegen(this), types);
    auto* function = Module.getFunction(name);
    if (function && function->getFunctionType() != functionType) throw std::runtime_error("conflicting prototype: " + name);
    if (!function) function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name, Module);
    AddSymbol(name, VAR_TYPE::FUNC, {function});
    parameterTypes[function] = params;
    returnTypes[function] = result;
}
llvm::Value* CodeGen::Initializer(TypeRef type, const std::vector<llvm::Value*>& values) {
    std::vector<llvm::Constant*> elements;
    for (int i = 0; i < type->count; ++i) {
        auto* value = i < static_cast<int>(values.size()) ? Cast(values[i], type->element) : CreateZero(type->element->Codegen(this));
        auto* constant = llvm::dyn_cast<llvm::Constant>(value);
        if (!constant) throw std::runtime_error("nonconstant aggregate initializer");
        elements.push_back(constant);
    }
    if (values.size() > static_cast<size_t>(type->count)) throw std::runtime_error("too many initializers");
    return llvm::ConstantArray::get(llvm::cast<llvm::ArrayType>(type->Codegen(this)), elements);
}
