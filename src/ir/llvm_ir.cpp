#include "ir.h"
#include "llvm_ir.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/ADCE.h"

#include "llvm/Support/raw_os_ostream.h"
#if LLVM_VERSION_MAJOR >= 18
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/MC/TargetRegistry.h"
#endif

#include <fstream>

LLVMEnv::LLVMEnv(std::string moduleName)
    : TheModule(std::forward<std::string>(moduleName), TheContext), Builder(TheContext) {
    
    // _SetMachineTarget("i386-unknown-linux-gnu");

    EnterScope();
}

void LLVMEnv::Optimize() {
// #if LLVM_VERSION_MAJOR >= 18
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

    mpm.run(TheModule, mam);
#endif
}

void LLVMEnv::Print() {
    TheModule.print(llvm::outs(), nullptr);
}

void LLVMEnv::Dump(const char* output) {
    std::ofstream outFile(output);
    llvm::raw_os_ostream rawOutFile(outFile);
    TheModule.print(rawOutFile, nullptr);
}

llvm::Type* LLVMEnv::CreateFuncType(llvm::Type* retType, std::vector<llvm::Type*> params) {
    auto paramTypes = std::vector<llvm::Type*>();
    for (auto& param : params) {
        paramTypes.push_back(param);
    }
    return llvm::FunctionType::get(retType, paramTypes, false);
}

llvm::BasicBlock* LLVMEnv::CreateBasicBlock(const std::string& name, llvm::Function* func) {
    return llvm::BasicBlock::Create(TheContext, name, func);
}

llvm::Function* LLVMEnv::CreateFunction(llvm::Type* funcType, const std::string& name, std::vector<std::string> names) {
    auto func = llvm::Function::Create((llvm::FunctionType*)funcType, llvm::Function::ExternalLinkage, name, &TheModule);
    auto args = func->arg_begin();
    for (size_t i = 0; i < names.size(); ++i) {
        args->setName(names[i]);
        ++args;
    }
    AddSymbol(name, VAR_TYPE::FUNC, { func });
    return func;
}

llvm::Value* LLVMEnv::CreateArray(llvm::Type* type, std::vector<llvm::Value*> values) {
    std::vector<llvm::Constant*> constants;
    for (auto* val : values) {
        if (auto* c = llvm::dyn_cast<llvm::Constant>(val)) {
            constants.push_back(c);
        }
    }
    return llvm::ConstantArray::get((llvm::ArrayType*)type, constants);
}

void LLVMEnv::CreateBuiltin(const std::string& name, llvm::Type* retType, std::vector<llvm::Type*> params) {
    auto* funcType = llvm::FunctionType::get(retType, params, false);
    auto* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, TheModule);
    AddSymbol(name, VAR_TYPE::FUNC, { func });
}

void LLVMEnv::CreateCondBr(llvm::Value* cond, llvm::BasicBlock* trueBB, llvm::BasicBlock* falseBB) {
    auto* logic_cond = CreateICmpNE(cond, GetInt32(0));
    Builder.CreateCondBr(logic_cond, trueBB, falseBB);
}

void LLVMEnv::CreateBr(llvm::BasicBlock* desc) {
    Builder.CreateBr(desc);
}

void LLVMEnv::CreateStore(llvm::Value* value, llvm::Value* dest) {
    Builder.CreateStore(value, dest);
}

llvm::Value* LLVMEnv::CreateLoad(llvm::Value* src) {
    // TODO
    return Builder.CreateLoad(llvm::Type::getInt32Ty(TheContext), src);
}

llvm::Value* LLVMEnv::CreateAlloca(llvm::Type* type, const std::string& name) {
    return Builder.CreateAlloca(type, nullptr, name);
}

llvm::Value* LLVMEnv::CreateGlobal(llvm::Type* type, const std::string& name, llvm::Value* init) {
    llvm::Constant* initVal = init ? llvm::dyn_cast<llvm::Constant>(init) : llvm::Constant::getNullValue(type);
    return new llvm::GlobalVariable(TheModule, type, false, llvm::GlobalValue::ExternalLinkage, initVal, name);
}

llvm::Value* LLVMEnv::CreateZero(llvm::Type* type) {
    return llvm::Constant::getNullValue(type);
}

llvm::Value* LLVMEnv::CreateAnd(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateAnd(lhs, rhs);
}

llvm::Value* LLVMEnv::CreateOr(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateOr(lhs, rhs);
}

llvm::Value* LLVMEnv::CreateAdd(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateAdd(lhs, rhs);
}

llvm::Value* LLVMEnv::CreateSub(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateSub(lhs, rhs);
}

llvm::Value* LLVMEnv::CreateMul(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateMul(lhs, rhs);
}

llvm::Value* LLVMEnv::CreateDiv(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateSDiv(lhs, rhs);
}

llvm::Value* LLVMEnv::CreateMod(llvm::Value* lhs, llvm::Value* rhs) {
    return Builder.CreateSRem(lhs, rhs);
}

llvm::Value* LLVMEnv::CreateICmpNE(llvm::Value* lhs, llvm::Value* rhs) {
    auto* res = Builder.CreateICmpNE(lhs, rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

llvm::Value* LLVMEnv::CreateICmpEQ(llvm::Value* lhs, llvm::Value* rhs) {
    auto* res = Builder.CreateICmpEQ(lhs, rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

llvm::Value* LLVMEnv::CreateICmpLT(llvm::Value* lhs, llvm::Value* rhs) {
    auto* res = Builder.CreateICmpSLT(lhs, rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

llvm::Value* LLVMEnv::CreateICmpGT(llvm::Value* lhs, llvm::Value* rhs) {
    auto* res = Builder.CreateICmpSGT(lhs, rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

llvm::Value* LLVMEnv::CreateICmpLE(llvm::Value* lhs, llvm::Value* rhs) {
    auto* res = Builder.CreateICmpSLE(lhs, rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

llvm::Value* LLVMEnv::CreateICmpGE(llvm::Value* lhs, llvm::Value* rhs) {
    auto* res = Builder.CreateICmpSGE(lhs, rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

void LLVMEnv::SetInserPointer(llvm::BasicBlock* ptr) {
    Builder.SetInsertPoint(ptr);
}

llvm::Function* LLVMEnv::GetFunction() {
    return Builder.GetInsertBlock()->getParent();
}

llvm::Value* LLVMEnv::GetFunctionArg(int index) {
    auto* func = GetFunction();
    auto argIt = func->arg_begin();
    std::advance(argIt, index);

    return &(*argIt);
}

llvm::Type* LLVMEnv::GetInt32Type() {
    return llvm::Type::getInt32Ty(TheContext);
}

llvm::Type* LLVMEnv::GetVoidType() {
    return llvm::Type::getVoidTy(TheContext);
}

llvm::Type* LLVMEnv::GetArrayType(llvm::Type* type, int size) {
    return llvm::ArrayType::get(type, size);
}

llvm::Type* LLVMEnv::GetPointerType(llvm::Type* type) {
    return llvm::PointerType::get(type, 0); // 0 for address space
}

llvm::Type* LLVMEnv::GetValueType(llvm::Value* value) {
    return value->getType();
}

llvm::Value* LLVMEnv::GetInt32(int value) {
    return llvm::ConstantInt::get(GetInt32Type(), value);
}

llvm::Value* LLVMEnv::CreateGEP(llvm::Type* type, llvm::Value* array, vector<llvm::Value*> index) {
    return Builder.CreateGEP(type, array, index);
}

llvm::Value* LLVMEnv::CaculateBinaryOp(const std::function<int(int, int)>& func, llvm::Value* lhs, llvm::Value* rhs) {
    auto* lhsConst = llvm::dyn_cast<llvm::ConstantInt>(lhs);
    auto* rhsConst = llvm::dyn_cast<llvm::ConstantInt>(rhs);

    int result = func(lhsConst->getSExtValue(), rhsConst->getSExtValue());
    return GetInt32(result);
}

int LLVMEnv::GetValueInt(llvm::Value* value) {
    auto* constInt = llvm::dyn_cast<llvm::ConstantInt>(value);
    if (constInt) {
        return constInt->getSExtValue();
    }
    return 0;
}

llvm::Value* LLVMEnv::GetArrayElement(llvm::Value* array, int index) {
    auto* arrayType = llvm::dyn_cast<llvm::ArrayType>(array->getType());
    auto* indexValue = GetInt32(index);
    return Builder.CreateGEP(arrayType->getElementType(), array, {indexValue});
}

bool LLVMEnv::IsArrayType(llvm::Type* value) {
    return value->isArrayTy();
}

bool LLVMEnv::IsPointerType(llvm::Type* value) {
    return value->isPointerTy();
}

bool LLVMEnv::EndWithTerminator() {
    auto* basic_block = Builder.GetInsertBlock();
    return !basic_block->empty() && basic_block->back().isTerminator();
}

void LLVMEnv::CreateRet(llvm::Value* value) {
    Builder.CreateRet(value);
}

llvm::Value* LLVMEnv::CreateCall(llvm::Function* func, std::vector<llvm::Value*> args) {
    return Builder.CreateCall(func, args);
}

#if LLVM_VERSION_MAJOR >= 18
void LLVMEnv::_SetMachineTarget(const char* target) {
    llvm::TargetOptions Options;
    std::string Error;

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();

    auto Target = llvm::TargetRegistry::lookupTarget(target, Error);
    auto TM = Target->createTargetMachine(target, "generic", "", Options, std::nullopt);

    TheModule.setDataLayout(TM->createDataLayout());
}
#endif