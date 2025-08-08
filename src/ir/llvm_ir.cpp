#include "ir.h"
#include "llvm_ir.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/ADCE.h"

#include "llvm/Support/raw_os_ostream.h"

#include <fstream>

LLVMEnv::LLVMEnv(std::string moduleName)
    : TheModule(std::forward<std::string>(moduleName), TheContext), Builder(TheContext) {
    EnterScope();
}

void LLVMEnv::Pass() {
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

void LLVMEnv::EnterScope() {
    locals.push_back({});
    types.push_back({});
}

void LLVMEnv::ExitScope() {
    locals.pop_back();
    types.pop_back();
}

void LLVMEnv::AddSymbol(const std::string& name, VAR_TYPE type, void* value) {
    auto value_t = (llvm::Value*)value;
    locals.back()[name] = value_t;
    types.back()[value_t] = type;
}

void* LLVMEnv::GetSymbolValue(const std::string& name) {
    for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return (void*)found->second;
        }
    }
    return nullptr;
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

llvm::Function* LLVMEnv::CreateFunction(llvm::Type* funcType, const std::string& name, std::vector<std::string> names) {
    auto func = llvm::Function::Create((llvm::FunctionType*)funcType, llvm::Function::ExternalLinkage, name, &TheModule);
    auto args = func->arg_begin();
    for (size_t i = 0; i < names.size(); ++i) {
        args->setName(names[i]);
        ++args;
    }
    AddSymbol(name, VAR_TYPE::FUNC, (void*)func);
    return func;
}

llvm::BasicBlock* LLVMEnv::CreateBasicBlock(const std::string& name, llvm::Function* func) {
    return llvm::BasicBlock::Create(TheContext, name, func);
}

void LLVMEnv::CreateCondBr(llvm::Value* cond, llvm::BasicBlock* trueBB, llvm::BasicBlock* falseBB) {
    auto* logic_cond = CreateICmpNE(cond, GetInt32(0));
    Builder.CreateCondBr(logic_cond, (llvm::BasicBlock*)trueBB, (llvm::BasicBlock*)falseBB);
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

llvm::Value* LLVMEnv::GetInt32(int value) {
    return llvm::ConstantInt::get((llvm::IntegerType*)GetInt32Type(), value);
}

bool LLVMEnv::EndWithTerminator() {
    auto* basic_block = (llvm::BasicBlock*)Builder.GetInsertBlock();
    return !basic_block->empty() && basic_block->back().isTerminator();
}

void LLVMEnv::CreateRet(llvm::Value* value) {
    Builder.CreateRet(value);
}

llvm::Value* LLVMEnv::CreateCall(void* func, std::vector<llvm::Value*> args) {
    auto* llvmFunc = (llvm::Function*)func;
    return Builder.CreateCall(llvmFunc, args);
}

VAR_TYPE LLVMEnv::GetSymbolType(void* value) {
    for (auto it = types.rbegin(); it != types.rend(); ++it) {
        auto found = it->find((llvm::Value*)value);
        if (found != it->end()) {
            return found->second;
        }
    }
    return VAR_TYPE::VAR;
}