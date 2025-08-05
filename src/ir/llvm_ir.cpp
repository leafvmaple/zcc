#include "ir.h"
#include "llvm_ir.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/ADCE.h"

LLVMEnv::LLVMEnv(std::string moduleName)
    : TheModule(std::forward<std::string>(moduleName), TheContext), Builder(TheContext) {
    EnterScope();
}

void LLVMEnv::CleanUp() {
#if LLVM_VERSION_MAJOR >= 18
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

void* LLVMEnv::CreateFuncType(void* retType) {
    return llvm::FunctionType::get((llvm::Type*)retType, false);
}

void* LLVMEnv::CreateFunction(void* funcType, const std::string& name) {
    return llvm::Function::Create((llvm::FunctionType*)funcType, llvm::Function::ExternalLinkage, name, &TheModule);
}

void* LLVMEnv::CreateBasicBlock(const std::string& name, void* func) {
    return llvm::BasicBlock::Create(TheContext, name, (llvm::Function*)func);
}

void LLVMEnv::CreateCondBr(void* cond, void* trueBB, void* falseBB) {
    auto* logic_cond = CreateICmpNE((llvm::Value*)cond, GetInt32(0));
    Builder.CreateCondBr((llvm::Value*)logic_cond, (llvm::BasicBlock*)trueBB, (llvm::BasicBlock*)falseBB);
}

void LLVMEnv::CreateBr(void* desc) {
    Builder.CreateBr((llvm::BasicBlock*)desc);
}

void LLVMEnv::CreateStore(void* value, void* dest) {
    Builder.CreateStore((llvm::Value*)value, (llvm::Value*)dest);
}

void* LLVMEnv::CreateLoad(void* src) {
    // TODO
    return Builder.CreateLoad(llvm::Type::getInt32Ty(TheContext), (llvm::Value*)src);
}

void* LLVMEnv::CreateAlloca(void* type, const std::string& name) {
    return Builder.CreateAlloca((llvm::Type*)type, nullptr, name);
}

void* LLVMEnv::CreateAnd(void* lhs, void* rhs) {
    return Builder.CreateAnd((llvm::Value*)lhs, (llvm::Value*)rhs);
}

void* LLVMEnv::CreateOr(void* lhs, void* rhs) {
    return Builder.CreateOr((llvm::Value*)lhs, (llvm::Value*)rhs);
}

void* LLVMEnv::CreateAdd(void* lhs, void* rhs) {
    return Builder.CreateAdd((llvm::Value*)lhs, (llvm::Value*)rhs);
}

void* LLVMEnv::CreateSub(void* lhs, void* rhs) {
    return Builder.CreateSub((llvm::Value*)lhs, (llvm::Value*)rhs);
}

void* LLVMEnv::CreateMul(void* lhs, void* rhs) {
    return Builder.CreateMul((llvm::Value*)lhs, (llvm::Value*)rhs);
}

void* LLVMEnv::CreateDiv(void* lhs, void* rhs) {
    return Builder.CreateSDiv((llvm::Value*)lhs, (llvm::Value*)rhs);
}

void* LLVMEnv::CreateMod(void* lhs, void* rhs) {
    return Builder.CreateSRem((llvm::Value*)lhs, (llvm::Value*)rhs);
}

void* LLVMEnv::CreateICmpNE(void* lhs, void* rhs) {
    auto* res = Builder.CreateICmpNE((llvm::Value*)lhs, (llvm::Value*)rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

void* LLVMEnv::CreateICmpEQ(void* lhs, void* rhs) {
    auto* res = Builder.CreateICmpEQ((llvm::Value*)lhs, (llvm::Value*)rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

void* LLVMEnv::CreateICmpLT(void* lhs, void* rhs) {
    auto* res = Builder.CreateICmpSLT((llvm::Value*)lhs, (llvm::Value*)rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

void* LLVMEnv::CreateICmpGT(void* lhs, void* rhs) {
    auto* res = Builder.CreateICmpSGT((llvm::Value*)lhs, (llvm::Value*)rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

void* LLVMEnv::CreateICmpLE(void* lhs, void* rhs) {
    auto* res = Builder.CreateICmpSLE((llvm::Value*)lhs, (llvm::Value*)rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

void* LLVMEnv::CreateICmpGE(void* lhs, void* rhs) {
    auto* res = Builder.CreateICmpSGE((llvm::Value*)lhs, (llvm::Value*)rhs);
    return Builder.CreateZExt(res, llvm::Type::getInt32Ty(TheContext));
}

void LLVMEnv::SetInserPointer(void* ptr) {
    Builder.SetInsertPoint((llvm::BasicBlock*)ptr);
}

void* LLVMEnv::GetFunction() {
    return Builder.GetInsertBlock()->getParent();
}

void* LLVMEnv::GetInt32Type() {
    return llvm::Type::getInt32Ty(TheContext);
}

void* LLVMEnv::GetInt32(int value) {
    return llvm::ConstantInt::get((llvm::IntegerType*)GetInt32Type(), value);
}

bool LLVMEnv::EndWithTerminator() {
    auto* basic_block = (llvm::BasicBlock*)Builder.GetInsertBlock();
    return !basic_block->empty() && 
           (basic_block->back().isTerminator());
}

void LLVMEnv::CreateRet(void* value) {
    Builder.CreateRet((llvm::Value*)value);
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