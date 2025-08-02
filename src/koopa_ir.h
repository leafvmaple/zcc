#pragma once

#include <vector>
#include <string>
#include <map>

#include "../libkoopa/include/koopa.h"

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

    void enter_scope() {
        locals.push_back({});
        values.clear();
    }

    void* create_inst(koopa_raw_value_t value) {
        values.push_back(value);
        return (void*)value;
    }

    koopa_raw_basic_block_t exit_scope() {
        locals.pop_back();
        koopa_raw_value_t* buffer = new koopa_raw_value_t[values.size()];
        std::copy(values.begin(), values.end(), buffer);

        return new koopa_raw_basic_block_data_t {
            .name = "%entry",
            .params = koopa_slice(KOOPA_RSIK_VALUE),
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .insts = {
                .buffer = (const void**)buffer,
                .len = static_cast<uint32_t>(values.size()),
                .kind = KOOPA_RSIK_VALUE
            }
        };
    }

    void add_symbol(std::string name, koopa_raw_value_t value) {
        if (!locals.empty()) {
            locals.back()[name] = value;
        } else {
            globals[name] = value;  // Add to global scope if no local scope exists
        }
    }

    koopa_raw_value_t get_symbol(std::string name) {
        if (!locals.empty() && locals.back().count(name)) {
            return locals.back()[name];
        } else if (globals.count(name)) {
            return globals[name];
        }
        return nullptr;  // Symbol not found
    }

private:
    std::vector<koopa_raw_value_t> values;
    std::vector<std::map<std::string, koopa_raw_value_t>> locals;
    std::map<std::string, koopa_raw_value_t> globals;
};
