#pragma once

#include <string>
#include <vector>
#include <map>

enum class VAR_TYPE {
    CONST,
    VAR,
    FUNC,
};

template<typename Type, typename Value, typename BasicBlock, typename Function>
class Env {
public:
    union Symbol_Value {
        Value* value;
        Function* function;

        bool operator<(const Symbol_Value& other) const {
            return value < other.value;
        }
    };

    virtual ~Env() = default;

    virtual void Optimize() = 0;
    virtual void Print() = 0;
    virtual void Dump(const char* output) = 0;

    virtual Type* CreateFuncType(Type* retType, std::vector<Type*> params) = 0;
    virtual BasicBlock* CreateBasicBlock(const std::string& name, Function* func) = 0;
    virtual Function* CreateFunction(Type* funcType, const std::string& name, std::vector<std::string> names) = 0;
    virtual void CreateBuiltin(const std::string& name, Type* retType, std::vector<Type*> params) = 0;

    virtual void CreateCondBr(Value* cond, BasicBlock* trueBB, BasicBlock* falseBB) = 0;
    virtual void CreateBr(BasicBlock* desc) = 0;

    virtual void CreateStore(Value* value, Value* dest) = 0;
    virtual Value* CreateLoad(Value* src) = 0;
    virtual void CreateRet(Value* value) = 0;
    virtual Value* CreateCall(Function* func, std::vector<Value*> args) = 0;

    virtual Value* CreateAnd(Value* lhs, Value* rhs) = 0;
    virtual Value* CreateOr(Value* lhs, Value* rhs) = 0;

    virtual Value* CreateAdd(Value* lhs, Value* rhs) = 0;
    virtual Value* CreateSub(Value* lhs, Value* rhs) = 0;
    virtual Value* CreateMul(Value* lhs, Value* rhs) = 0;
    virtual Value* CreateDiv(Value* lhs, Value* rhs) = 0;
    virtual Value* CreateMod(Value* lhs, Value* rhs) = 0;

    virtual Value* CreateAlloca(Type* type, const std::string& name) = 0;
    virtual Value* CreateGlobal(Type* type, const std::string& name, Value* init) = 0;
    virtual Value* CreateZero(Type* type) = 0;

    virtual Value* CreateICmpNE(Value* lhs, Value* rhs) = 0;
    virtual Value* CreateICmpEQ(Value* lhs, Value* rhs) = 0;

    virtual Value* CreateICmpLT(Value* lhs, Value* rhs) = 0;
    virtual Value* CreateICmpGT(Value* lhs, Value* rhs) = 0;
    virtual Value* CreateICmpLE(Value* lhs, Value* rhs) = 0;
    virtual Value* CreateICmpGE(Value* lhs, Value* rhs) = 0;
    
    virtual void SetInserPointer(BasicBlock* ptr) = 0;

    virtual Function* GetFunction() = 0;
    virtual Value* GetFunctionArg(int index) = 0;

    virtual Type* GetInt32Type() = 0;
    virtual Type* GetVoidType() = 0;
    virtual Type* GetArrayType(Type* type) = 0;
    virtual Type* GetPointerType(Type* type) = 0;

    virtual Value* GetInt32(int value) = 0;

    virtual bool EndWithTerminator() = 0;

    void EnterScope() {
        locals.push_back({});
        types.push_back({});
    }

    void ExitScope() {
        locals.pop_back();
        types.pop_back();
    }

    bool IsGlobalScope() const {
        return locals.size() == 1;
    }

    void AddSymbol(const std::string& name, VAR_TYPE type, Symbol_Value value)  {
        locals.back()[name] = value;
        types.back()[value] = type;
    }

    Symbol_Value GetSymbolValue(const std::string& name) {
        for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        return { nullptr };
    }

    VAR_TYPE GetSymbolType(Symbol_Value value)  {
        for (auto it = types.rbegin(); it != types.rend(); ++it) {
            auto found = it->find(value);
            if (found != it->end()) {
                return found->second;
            }
        }
        return VAR_TYPE::VAR;
    }
    
    void EnterWhile(BasicBlock* entry, BasicBlock* end) {
        whiles.push_back({entry, end});
    }

    void ExitWhile() {
        whiles.pop_back();
    }

    BasicBlock* GetWhileEntry() {
        return whiles.back().entry;
    }

    BasicBlock* GetWhileEnd() { 
        return whiles.back().end;
    }

private:
    struct while_data_t {
        BasicBlock* entry;
        BasicBlock* end;
    };

    std::vector<std::map<std::string, Symbol_Value>> locals;
    std::vector<std::map<Symbol_Value, VAR_TYPE>> types;
    std::vector<while_data_t> whiles;
};
