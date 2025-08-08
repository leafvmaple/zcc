#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <cassert>

#include "../libkoopa/include/koopa.h"
#include "../libkoopa/include/function.h"

#include "ir.h"

enum class SYMBOL {
    CONST,
    VAR,
};

class KoopaEnv;

koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind) {
    return {nullptr, 0, kind};
}

template<typename T>
koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind, const T& vec, KoopaEnv* env) {
    auto* buffer = new const void*[1];
    buffer[0] = vec->ToKoopa(env);
    return {buffer, 1, kind};
}

template<typename T>
koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind, const std::vector<T>& vec, KoopaEnv* env) {
    auto* buffer = new const void*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i) {
        buffer[i] = vec[i]->ToKoopa(env);
    }
    return {buffer, static_cast<uint32_t>(vec.size()), kind};
}

template<typename T>
koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind, const std::vector<T>& vec) {
    auto* buffer = new const void*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i) {
        buffer[i] = vec[i];
    }
    return {buffer, static_cast<uint32_t>(vec.size()), kind};
}

inline koopa::Type* koopa_type(koopa_raw_type_tag_t tag) {
    return new koopa_raw_type_kind_t { tag };
}

inline koopa::Type* koopa_pointer(koopa_raw_type_tag_t tag) {
    return new koopa_raw_type_kind_t {
        .tag = KOOPA_RTT_POINTER,
        .data.pointer = {
            .base = koopa_type(tag)
        }
    };
}

koopa_raw_value_t inline koopa_int(int value) {
    return new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_INTEGER,
            .data.integer = {
                .value = value
            }
        }
    };
}

const char* to_string(std::string name);
const char* to_string(int value);

class KoopaEnv : public Env<koopa_raw_type_kind_t, koopa_raw_value_data, zcc_basic_block_data_t, zcc_function_data_t> {
public:
    using T = koopa_raw_type_kind_t;
    using V = koopa_raw_value_data_t;
    using B = zcc_basic_block_data_t;
    using F = zcc_function_data_t;

    KoopaEnv();
    KoopaEnv(std::string input) : KoopaEnv() {}

    void Pass() override;
    
    void EnterWhile(koopa::BasicBlock* entry, koopa::BasicBlock* end) override {
        whiles.push_back({entry, end});
    }

    void ExitWhile() override {
        whiles.pop_back();
    }

    koopa::BasicBlock* GetWhileEntry() override {
        return whiles.back().entry;
    }

    koopa::BasicBlock* GetWhileEnd() override {
        return whiles.back().end;
    }

    void Print() override;
    void Dump(const char* output) override;

    koopa::Type* CreateFuncType(koopa::Type* retType, std::vector<koopa::Type*> params) override;
    koopa::Function* CreateFunction(koopa::Type* funcType, const std::string& name, std::vector<std::string> params) override;
    koopa::BasicBlock* CreateBasicBlock(const std::string& name, F* func) override;

    void CreateCondBr(koopa::Value* cond, koopa::BasicBlock* trueBB, koopa::BasicBlock* falseBB) override;
    void CreateBr(koopa::BasicBlock* desc) override;

    void CreateStore(koopa::Value* value, koopa::Value* dest) override;
    koopa::Value* CreateLoad(koopa::Value* src) override;
    void CreateRet(koopa::Value* value) override;
    koopa::Value* CreateCall(void* func, std::vector<koopa::Value*> args) override;

    koopa::Value* CreateAnd(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateOr(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateAdd(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateSub(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateMul(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateDiv(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateMod(koopa::Value* lhs, koopa::Value* rhs) override;

    koopa::Value* CreateAlloca(koopa::Type* type, const std::string& name) override;

    koopa::Value* CreateICmpNE(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateICmpEQ(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateICmpLT(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateICmpGT(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateICmpLE(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateICmpGE(koopa::Value* lhs, koopa::Value* rhs) override;

    void SetInserPointer(koopa::BasicBlock* ptr) override;

    koopa::Function* GetFunction() override;
    koopa::Value* GetFunctionArg(int index) override;

    koopa::Type* GetInt32Type() override;
    koopa::Type* GetVoidType() override;

    koopa::Value* GetInt32(int value) override;

    bool EndWithTerminator() override;

private:
    struct zcc_while_data_t {
        koopa::BasicBlock* entry;
        koopa::BasicBlock* end;
    };

    union koopa_raw_symobol_data_t {
        koopa_raw_value_t value;
        koopa_raw_function_t function;
    };

    zcc_function_vec_t funcs;

    std::vector<std::map<std::string, void*>> locals{};
    std::vector<std::map<void*, VAR_TYPE>> types{};
    std::vector<zcc_while_data_t> whiles{};

    koopa_raw_program_t* raw_program{};
    koopa_program_t program{};
    zcc_basic_block_data_t* insert_ptr{};

    koopa::Value* _CreateInst(koopa::Value* value);
    koopa_raw_basic_block_t _ParseBasicBlock(const zcc_basic_block_data_t& bbs);
    koopa_raw_function_t _ParseFunction(const zcc_function_data_t& funcs);
    int _ParseProgram();
};
