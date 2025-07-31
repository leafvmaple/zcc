#pragma once

#include <vector>

#include "koopa.h"

koopa_raw_slice_t inline slice(koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN) {
    return {nullptr, 0, kind};
}

template<typename T>
koopa_raw_slice_t inline slice(koopa_raw_slice_item_kind_t kind, const T& vec) {
    auto* buffer = new const void*[1];
    buffer[0] = vec->Parse();
    return {buffer, 1, kind};
}

template<typename T>
koopa_raw_slice_t inline slice(koopa_raw_slice_item_kind_t kind, const std::vector<T>& vec) {
    auto* buffer = new const void*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i) {
        buffer[i] = vec[i]->Parse();
    }
    return {buffer, static_cast<uint32_t>(vec.size()), kind};
}

koopa_raw_type_t inline type_kind(koopa_raw_type_tag_t tag) {
    return new koopa_raw_type_kind_t { tag };
}