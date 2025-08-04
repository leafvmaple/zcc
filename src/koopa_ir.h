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

    void _save_basic_block();
    void _save_function();
    int _save_program();

    void Print();
    void Dump(const std::string& outfile);

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

private:
    void* _CreateInst(koopa_raw_value_t value);

    std::vector<koopa_raw_value_t> insts;
    std::vector<std::map<std::string, koopa_raw_value_t>> locals;
    std::vector<std::map<koopa_raw_value_t, VAR_TYPE>> types;

    koopa_raw_program_t* raw_program = nullptr;
    koopa_program_t program = nullptr;

    std::string func_name;
    koopa_raw_type_t func_type;
    std::vector<koopa_raw_function_data_t*> funcs;

    std::string bb_name;
    std::vector<koopa_raw_basic_block_data_t*> bbs;
};
