#include "koopa_ir.h"

const char* to_string(std::string name) {
    auto* str = new char[name.size() + 1];
    std::copy(name.begin(), name.end(), str);
    str[name.size()] = '\0';
    return str;
}

const char* to_string(int value) {
    auto* str = new char[12];  // Enough for int32_t
    snprintf(str, 12, "%%%d", value);
    return str;
}

KoopaEnv::KoopaEnv() {
    locals.push_back({});  // 初始化全局作用域
    types.push_back({});
}

void KoopaEnv::_save_basic_block() {
    if (!insts.empty()) {
        bbs.emplace_back(new koopa_raw_basic_block_data_t {
            .name = bb_name != "" ? to_string(bb_name) : nullptr,
            .params = koopa_slice(KOOPA_RSIK_VALUE),
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .insts = koopa_slice(KOOPA_RSIK_VALUE, insts)
        });
        insts.clear();
    }
}

void KoopaEnv::_save_function() {
    _save_basic_block();
    if (!bbs.empty()) {
        funcs.emplace_back(new koopa_raw_function_data_t {
            .ty = func_type,
            .name = to_string(func_name),
            .params = koopa_slice(KOOPA_RSIK_VALUE),
            .bbs = koopa_slice(KOOPA_RSIK_BASIC_BLOCK, bbs)
        });
        bbs.clear();
    }
}

int KoopaEnv::_save_program() {
    _save_function();
    if (!funcs.empty()) {
        raw_program = new koopa_raw_program_t {
            .values = koopa_slice(KOOPA_RSIK_VALUE),
            .funcs = koopa_slice(KOOPA_RSIK_FUNCTION, funcs)
        };
        funcs.clear();
    }
    if (!program) {
        auto res = koopa_generate_raw_to_koopa(raw_program, &program);
        if (res != KOOPA_EC_SUCCESS) {
            std::cerr << "Error generating Koopa IR: " << res << std::endl;
            return 0;
        }
    }
    return 1;
}

void KoopaEnv::Print() {
    if (_save_program()) {
        koopa_dump_to_stdout(program);
    }
}

void KoopaEnv::Dump(const std::string& outfile) {
    if (_save_program()) {
        koopa_dump_to_file(program, outfile.c_str());
    }
}

void* KoopaEnv::CreateFuncType(void* retType) {
    return new koopa_raw_type_kind_t {
        KOOPA_RTT_FUNCTION,
        .data.function = {
            .params = koopa_slice(KOOPA_RSIK_TYPE),
            .ret = (koopa_raw_type_t)retType
        }
    };
}

void KoopaEnv::CreateFunction(void* funcType, const std::string& name) {
    _save_function();

    func_name = name;
    func_type = (koopa_raw_type_t)funcType;
}

void KoopaEnv::CreateStore(void* value, void* dest) {
    insts.push_back(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_STORE,
            .data.store = {
                .value = (koopa_raw_value_t)value,
                .dest = (koopa_raw_value_t)dest,
            }
        }
    });
}

void* KoopaEnv::CreateLoad(void* src) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_LOAD,
            .data.load = {
                .src = (koopa_raw_value_t)src
            }
        }
    });
}

void KoopaEnv::CreateRet(void* value) {
    insts.push_back(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_RETURN,
            .data.ret = {
                .value = (koopa_raw_value_t)value
            }
        }
    });
}

void KoopaEnv::CreateBasicBlock(const std::string& name) {
    _save_basic_block();
    bb_name = name;
}

void* KoopaEnv::CreateAnd(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_AND,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateOr(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_OR,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateAdd(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_ADD,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateSub(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_SUB,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateMul(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_MUL,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateDiv(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_DIV,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateMod(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_MOD,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateAlloca(void* type, const std::string& name) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = to_string(name),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_ALLOC,
        }
    });
}

void* KoopaEnv::CreateICmpNE(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_NOT_EQ,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateICmpEQ(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_EQ,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateICmpLT(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_LT,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateICmpGT(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_GT,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateICmpLE(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_LE,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void* KoopaEnv::CreateICmpGE(void* lhs, void* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_GE,
                .lhs = (koopa_raw_value_t)lhs,
                .rhs = (koopa_raw_value_t)rhs
            }
        }
    });
}

void KoopaEnv::EnterScope() {
    locals.push_back({});
    types.push_back({});
}

void* KoopaEnv::_CreateInst(koopa_raw_value_t value) {
    insts.push_back(value);
    return (void*)value;
}

void KoopaEnv::ExitScope() {
    locals.pop_back();
    types.pop_back();
}

void KoopaEnv::AddSymbol(const std::string& name, VAR_TYPE type, void* value) {
    auto value_t = (koopa_raw_value_t)value;
    locals.back()[name] = value_t;
    types.back()[value_t] = type;
}

void* KoopaEnv::GetSymbolValue(const std::string& name) {
    for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return (void*)found->second;
        }
    }
    return nullptr;
}

VAR_TYPE KoopaEnv::GetSymbolType(void* value) {
    for (auto it = types.rbegin(); it != types.rend(); ++it) {
        auto found = it->find((koopa_raw_value_t)value);
        if (found != it->end()) {
            return found->second;
        }
    }
    return VAR_TYPE::VAR;
}
