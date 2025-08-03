#pragma once

#include <vector>
#include <string>
#include <map>

#include "../libkoopa/include/koopa.h"

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

koopa_raw_value_t inline int2logic(koopa_raw_value_t value) {
    return new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_NOT_EQ,
                .lhs = value,
                .rhs = koopa_int(0)
            }
        }
    };
}

const char* to_string(std::string name);
const char* to_string(int value);

class KoopaEnv {
public:
    KoopaEnv() {
        locals.push_back({});  // global scope
        types.push_back({});
    }

    void enter_scope() {
        locals.push_back({});
        types.push_back({});
        insts.clear();
    }

    void* create_inst(koopa_raw_value_t value) {
        insts.push_back(value);
        return (void*)value;
    }

    koopa_raw_basic_block_t exit_scope() {
        locals.pop_back();
        types.pop_back();
        uint32_t size = static_cast<uint32_t>(insts.size());
        koopa_raw_value_t* buffer = new koopa_raw_value_t[size];
        std::copy(insts.begin(), insts.end(), buffer);

        insts.clear();
        return new koopa_raw_basic_block_data_t {
            .name = "%entry",
            .params = koopa_slice(KOOPA_RSIK_VALUE),
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .insts = {
                .buffer = (const void**)buffer,
                .len = size,
                .kind = KOOPA_RSIK_VALUE
            }
        };
    }

    void add_symbol(std::string name, SYMBOL type, koopa_raw_value_t value) {
        locals.back()[name] = value;
        types.back()[value] = type;
    }

    koopa_raw_value_t get_symbol_value(std::string name) {
        for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;  // Return the value if found in the current scope
            }
        }

        return nullptr;  // Symbol not found
    }

    SYMBOL get_symbol_type(koopa_raw_value_t value) {
        for (auto it = types.rbegin(); it != types.rend(); ++it) {
            auto found = it->find(value);
            if (found != it->end()) {
                return found->second;
            }
        }

        return SYMBOL::VAR;
    }

private:
    std::vector<koopa_raw_value_t> insts;
    std::vector<std::map<std::string, koopa_raw_value_t>> locals;
    std::vector<std::map<koopa_raw_value_t, SYMBOL>> types;
};
