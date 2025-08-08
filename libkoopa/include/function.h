#pragma once

#include "koopa.h"

#include <vector>
#include <string>

typedef std::vector<koopa_raw_value_t> koopa_inst_vec_t;
struct zcc_basic_block_data_t {
    koopa_inst_vec_t insts;
    koopa_raw_basic_block_data_t* ptr;
};
typedef std::vector<zcc_basic_block_data_t*> zcc_basic_block_vec_t;
struct zcc_function_data_t {
    std::string name;
    std::vector<koopa_raw_value_data_t*> params;
    zcc_basic_block_vec_t bbs;
    koopa_raw_function_data_t* ptr;
};
typedef std::vector<zcc_function_data_t> zcc_function_vec_t;


namespace koopa {
    using Type = koopa_raw_type_kind_t;
    using Value = koopa_raw_value_data_t;
    using BasicBlock = zcc_basic_block_data_t;
    using Function = zcc_function_data_t;
}