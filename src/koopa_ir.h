#pragma once

#include <vector>
#include <string>

#include "../libkoopa/include/koopa.h"

class KoopaEnv {
public:
    int GetVarCount() {
        return varCount++;
    }

private:
    int varCount = 0;
};

koopa_raw_slice_t inline slice(koopa_raw_slice_item_kind_t kind) {
    return {nullptr, 0, kind};
}

template<typename T>
koopa_raw_slice_t inline slice(koopa_raw_slice_item_kind_t kind, const T& vec, KoopaEnv* env) {
    auto* buffer = new const void*[1];
    buffer[0] = vec->Parse(env);
    return {buffer, 1, kind};
}

template<typename T>
koopa_raw_slice_t inline slice(koopa_raw_slice_item_kind_t kind, const std::vector<T>& vec, KoopaEnv* env) {
    auto* buffer = new const void*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i) {
        buffer[i] = vec[i]->Parse(env);
    }
    return {buffer, static_cast<uint32_t>(vec.size()), kind};
}

koopa_raw_type_t inline type_kind(koopa_raw_type_tag_t tag) {
    return new koopa_raw_type_kind_t { tag };
}

const char* c_string(std::string name);
const char* c_string(int value);