#include "ast.h"
#include "ir.h"
#include "llvm_ir.h"

LLVMEnv::LLVMEnv(std::string moduleName)
    : TheModule(std::forward<std::string>(moduleName), TheContext), Builder(TheContext) {
    symtab.EnterScope();
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

void LLVMEnv::CreateFunction(void* funcType, const std::string& name) {
    auto* func = llvm::Function::Create((llvm::FunctionType*)funcType, llvm::Function::ExternalLinkage, name, &TheModule);
    Builder.SetInsertPoint(llvm::BasicBlock::Create(TheContext, "entry", func));
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