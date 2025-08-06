#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <cassert>

#include "../libkoopa/include/koopa.h"

#include "ir.h"

enum class SYMBOL {
    CONST,
    VAR,
};

typedef std::vector<koopa_raw_value_t> zcc_value_vec_t;
struct zcc_basic_block_data_t {
    zcc_value_vec_t insts;
    koopa_raw_basic_block_data_t* ptr;
};
typedef std::vector<zcc_basic_block_data_t> zcc_basic_block_vec_t;
struct zcc_function_data_t {
    std::string name;
    koopa_raw_type_t type;
    zcc_basic_block_vec_t bbs;
};
typedef std::vector<zcc_function_data_t> zcc_function_vec_t;

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

koopa_raw_type_t inline koopa_type(koopa_raw_type_tag_t tag) {
    return new koopa_raw_type_kind_t { tag };
}

koopa_raw_type_t inline koopa_pointer(koopa_raw_type_tag_t tag) {
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

class KoopaEnv : public Env {
public:
    KoopaEnv();

    void EnterScope() override;
    void ExitScope() override;

    koopa_raw_basic_block_t _ParseBasicBlock(const zcc_basic_block_data_t& bbs);
    koopa_raw_function_t _ParseFunction(const zcc_function_data_t& funcs);
    int _ParseProgram();

    void Print() override;
    void Dump(const char* output) override;

    void* CreateFuncType(void* retType) override;
    void* CreateFunction(void* funcType, const std::string& name) override;
    void* CreateBasicBlock(const std::string& name, void* func) override;

    void CreateCondBr(void* cond, void* trueBB, void* falseBB) override;
    void CreateBr(void* desc) override;

    void CreateStore(void* value, void* dest) override;
    void* CreateLoad(void* src) override;

    void CreateRet(void* value) override;

    void* CreateAnd(void* lhs, void* rhs) override;
    void* CreateOr(void* lhs, void* rhs) override;
    void* CreateAdd(void* lhs, void* rhs) override;
    void* CreateSub(void* lhs, void* rhs) override;
    void* CreateMul(void* lhs, void* rhs) override;
    void* CreateDiv(void* lhs, void* rhs) override;
    void* CreateMod(void* lhs, void* rhs) override;

    void* CreateAlloca(void* type, const std::string& name) override;

    void* CreateICmpNE(void* lhs, void* rhs) override;
    void* CreateICmpEQ(void* lhs, void* rhs) override;
    void* CreateICmpLT(void* lhs, void* rhs) override;
    void* CreateICmpGT(void* lhs, void* rhs) override;
    void* CreateICmpLE(void* lhs, void* rhs) override;
    void* CreateICmpGE(void* lhs, void* rhs) override;

    void SetInserPointer(void* ptr) override;

    void* GetFunction() override;
    void* GetInt32Type() override;
    void* GetInt32(int value) override;

    bool EndWithTerminator() override;

    void AddSymbol(const std::string& name, VAR_TYPE type, void* value) override;
    void* GetSymbolValue(const std::string& name) override;
    VAR_TYPE GetSymbolType(void* value) override;

private:
    void* _CreateInst(koopa_raw_value_t value);

    zcc_function_vec_t funcs;

    std::vector<std::map<std::string, koopa_raw_value_t>> locals;
    std::vector<std::map<koopa_raw_value_t, VAR_TYPE>> types;

    koopa_raw_program_t* raw_program = nullptr;
    koopa_program_t program = nullptr;

    zcc_value_vec_t* insert_ptr = nullptr;
};
