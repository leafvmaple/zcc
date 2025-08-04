#pragma once

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include <map>

class SymbolTable {
public:
    void EnterScope() {
        locals.push_back({});
    }

    llvm::Value* GetSymbolValue(const std::string& name) {
        for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;  // Return the value if found in the current scope
            }
        }
        auto globalFound = globals.find(name);
        return globalFound != globals.end() ? globalFound->second : nullptr;  // Return nullptr if not found
    }

    llvm::Type* GetSymbolType(llvm::Value* value) {
        return typeMap[value];
    }

    void AddSymbol(const std::string& name, llvm::Type* type, llvm::Value* value) {
        if (!locals.empty()) {
            locals.back()[name] = value;
        } else {
            globals[name] = value;  // Add to global scope if no local scope exists
        }
        typeMap[value] = type;  // Map the value to its type
    }

    void ExitScope() {
        if (!locals.empty()) {
            locals.pop_back();
        }
    }

private:
    std::vector<std::map<std::string, llvm::Value*>> locals;
    std::map<std::string, llvm::Value*> globals;
    std::map<llvm::Value*, llvm::Type*> typeMap;
    // std::vector<std::unordered_map<std::string, llvm::Value*>> locals;
    // std::unordered_map<std::string, llvm::Value*> globals;  // Global symbols
    // std::unordered_map<llvm::Value*, llvm::Type*> typeMap;  // Maps values to their types
};

class LLVMEnv {
public:
    LLVMEnv(std::string moduleName);
    
    void EnterScope();
    void ExitScope();

    void* CreateFuncType(void* retType);
    void CreateFunction(void* funcType, const std::string& name);

    void CreateStore(void* value, void* dest);
    void* CreateLoad(void* src);

    void CreateRet(void* value);
    void CreateBasicBlock(const std::string& name);

    void* CreateAnd(void* lhs, void* rhs);
    void* CreateOr(void* lhs, void* rhs);
    void* CreateAdd(void* lhs, void* rhs);
    void* CreateSub(void* lhs, void* rhs);
    void* CreateMul(void* lhs, void* rhs);
    void* CreateDiv(void* lhs, void* rhs);
    void* CreateMod(void* lhs, void* rhs);

    void* CreateAlloca(void* type, const std::string& name);

    void* CreateICmpNE(void* lhs, void* rhs);
    void* CreateICmpEQ(void* lhs, void* rhs);
    void* CreateICmpLT(void* lhs, void* rhs);
    void* CreateICmpGT(void* lhs, void* rhs);
    void* CreateICmpLE(void* lhs, void* rhs);
    void* CreateICmpGE(void* lhs, void* rhs);

    void AddSymbol(const std::string& name, VAR_TYPE type, void* value);
    void* GetSymbolValue(const std::string& name);
    VAR_TYPE GetSymbolType(void* value);

    llvm::LLVMContext TheContext;
    llvm::Module TheModule;
    llvm::IRBuilder<llvm::NoFolder, llvm::IRBuilderDefaultInserter> Builder;

    SymbolTable symtab;

private:
    std::vector<std::map<std::string, llvm::Value*>> locals;
    std::vector<std::map<llvm::Value*, VAR_TYPE>> types;
};