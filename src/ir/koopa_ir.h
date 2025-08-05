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

    void EnterScope();
    void ExitScope();

    koopa_raw_basic_block_t _ParseBasicBlock(const zcc_basic_block_data_t& bbs);
    koopa_raw_function_t _ParseFunction(const zcc_function_data_t& funcs);
    int _ParseProgram();

    void Print();
    void Dump(const std::string& outfile);

    void* CreateFuncType(void* retType);
    void* CreateFunction(void* funcType, const std::string& name);
    void* CreateBasicBlock(const std::string& name, void* func);

    void CreateCondBr(void* cond, void* trueBB, void* falseBB);
    void CreateBr(void* desc);

    void CreateStore(void* value, void* dest);
    void* CreateLoad(void* src);

    void CreateRet(void* value);

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

    void SetInserPointer(void* ptr);

    void* GetFunction();

    void* GetInt32Type();

    void* GetInt32(int value);

    bool EndWithTerminator();

    void AddSymbol(const std::string& name, VAR_TYPE type, void* value);
    void* GetSymbolValue(const std::string& name);
    VAR_TYPE GetSymbolType(void* value);

private:
    void* _CreateInst(koopa_raw_value_t value);

    zcc_function_vec_t funcs;

    std::vector<std::map<std::string, koopa_raw_value_t>> locals;
    std::vector<std::map<koopa_raw_value_t, VAR_TYPE>> types;

    koopa_raw_program_t* raw_program = nullptr;
    koopa_program_t program = nullptr;

    zcc_value_vec_t* insert_ptr = nullptr;
};
