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

template<typename Type>
koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind, const Type& vec, KoopaEnv* env) {
    auto* buffer = new const void*[1];
    buffer[0] = vec->Codegen(env);
    return {buffer, 1, kind};
}

template<typename Type>
koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind, const std::vector<Type>& vec, KoopaEnv* env) {
    auto* buffer = new const void*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i) {
        buffer[i] = vec[i]->Codegen(env);
    }
    return {buffer, static_cast<uint32_t>(vec.size()), kind};
}

template<typename Type>
koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind, const std::vector<Type>& vec) {
    auto* buffer = new const void*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i) {
        buffer[i] = vec[i];
    }
    return {buffer, static_cast<uint32_t>(vec.size()), kind};
}

inline koopa::Type* koopa_type(koopa_raw_type_tag_t tag) {
    return new koopa_raw_type_kind_t { tag };
}

inline koopa::Type* koopa_pointer(koopa::Type* type) {
    return new koopa_raw_type_kind_t {
        .tag = KOOPA_RTT_POINTER,
        .data.pointer = {
            .base = type
        }
    };
}

koopa_raw_value_t inline koopa_int(int value) {
    return new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
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

class KoopaEnv : public Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function> {
public:
    using T = koopa_raw_type_kind_t;
    using V = koopa_raw_value_data_t;
    using B = zcc_basic_block_data_t;
    using F = zcc_function_data_t;

    KoopaEnv();
    KoopaEnv(std::string input) : KoopaEnv() {}

    void Optimize() override;
    void Print() override;
    void Dump(const char* output) override;

    koopa::Type* CreateFuncType(koopa::Type* retType, std::vector<koopa::Type*> params) override;
    koopa::BasicBlock* CreateBasicBlock(const std::string& name, F* func) override;
    koopa::Function* CreateFunction(koopa::Type* funcType, const std::string& name, std::vector<std::string> params) override;
    koopa::Value* CreateArray(koopa::Type* type, std::vector<koopa::Value*> values) override;
    void CreateBuiltin(const std::string& name, koopa::Type* retType, std::vector<koopa::Type*> params) override;

    void CreateCondBr(koopa::Value* cond, koopa::BasicBlock* trueBB, koopa::BasicBlock* falseBB) override;
    void CreateBr(koopa::BasicBlock* desc) override;

    void CreateStore(koopa::Value* value, koopa::Value* dest) override;
    void CreateRet(koopa::Value* value) override;

    koopa::Value* CreateLoad(koopa::Value* src) override;
    koopa::Value* CreateCall(koopa::Function* func, std::vector<koopa::Value*> args) override;

    koopa::Value* CreateAnd(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateOr(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateAdd(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateSub(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateMul(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateDiv(koopa::Value* lhs, koopa::Value* rhs) override;
    koopa::Value* CreateMod(koopa::Value* lhs, koopa::Value* rhs) override;

    koopa::Value* CreateAlloca(koopa::Type* type, const std::string& name) override;
    koopa::Value* CreateGlobal(koopa::Type* type, const std::string& name, koopa::Value* init) override;
    koopa::Value* CreateZero(koopa::Type* type) override;

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
    koopa::Type* GetArrayType(koopa::Type* type, int size) override;
    koopa::Type* GetPointerType(koopa::Type* type) override;

    koopa::Value* GetInt32(int value) override;
    koopa::Value* CreateGEP(koopa::Type* type, koopa::Value* array, koopa::Value* index) override;

    koopa::Value* CaculateBinaryOp(const std::function<int(int, int)>& func, koopa::Value* lhs, koopa::Value* rhs) override;

    int GetValueInt(koopa::Value* value) override;

    bool EndWithTerminator() override;

private:
    zcc_function_vec_t funcs;
    std::vector<koopa_raw_value_data_t*> values;

    koopa_raw_program_t* raw_program{};
    koopa_program_t program{};
    zcc_basic_block_data_t* insert_ptr{};

    koopa::Value* _CreateInst(koopa::Value* value);
    koopa_raw_basic_block_t _ParseBasicBlock(const zcc_basic_block_data_t& bbs);
    koopa_raw_function_t _ParseFunction(const zcc_function_data_t& funcs);
    int _ParseProgram();
};
