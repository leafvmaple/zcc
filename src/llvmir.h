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
    SymbolTable() = default;
    ~SymbolTable() = default;

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

class LLVMParams {
public:
    LLVMParams(std::string moduleName)
        : TheModule(std::forward<std::string>(moduleName), TheContext), Builder(TheContext){};
    ~LLVMParams() = default;

    llvm::LLVMContext TheContext;
    llvm::Module TheModule;
    llvm::IRBuilder<llvm::NoFolder, llvm::IRBuilderDefaultInserter> Builder;

    SymbolTable symtab;
};