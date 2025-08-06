#include "koopa_ir.h"

const int VEC_RESERVE_SIZE = 256;

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

bool _IsTerminator(const koopa_raw_value_t value) {
    return value->kind.tag == KOOPA_RVT_RETURN
        || value->kind.tag == KOOPA_RVT_BRANCH
        || value->kind.tag == KOOPA_RVT_JUMP;
}

KoopaEnv::KoopaEnv() {
    EnterScope();
    funcs.reserve(VEC_RESERVE_SIZE);
}

void KoopaEnv::EnterScope() {
    locals.push_back({});
    types.push_back({});
}

void KoopaEnv::ExitScope() {
    locals.pop_back();
    types.pop_back();
}

void KoopaEnv::Print() {
    if (_ParseProgram()) {
        koopa_dump_to_stdout(program);
    }
}

void KoopaEnv::Dump(const char* output) {
    if (_ParseProgram()) {
        koopa_dump_to_file(program, output);
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

void* KoopaEnv::CreateFunction(void* funcType, const std::string& name) {
    funcs.push_back({
        .name = "@" + name,
        .type = (koopa_raw_type_t)funcType
    });
    funcs.back().bbs.reserve(VEC_RESERVE_SIZE);
    return (void*)&funcs.back();
}

void* KoopaEnv::CreateBasicBlock(const std::string& name, void* func) {
    auto* function = (zcc_function_data_t*)func;
    function->bbs.push_back({
        .ptr = new koopa_raw_basic_block_data_t {
            .name = to_string("%" + name),
            .params = koopa_slice(KOOPA_RSIK_VALUE),
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .insts = koopa_slice(KOOPA_RSIK_VALUE)
        }
    });
    function->bbs.back().insts.reserve(VEC_RESERVE_SIZE);
    return (void*)&function->bbs.back();
}

void KoopaEnv::CreateCondBr(void* cond, void* trueBB, void* falseBB) {
    auto* true_block = (zcc_basic_block_data_t*)trueBB;
    auto* false_block = (zcc_basic_block_data_t*)falseBB;

    _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_UNIT),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BRANCH,
            .data.branch = {
                .cond = (koopa_raw_value_t)cond,
                .true_bb = true_block->ptr,
                .false_bb = false_block->ptr,
                .true_args = koopa_slice(KOOPA_RSIK_VALUE),
                .false_args = koopa_slice(KOOPA_RSIK_VALUE)
            }
        }
    });
}

void KoopaEnv::CreateBr(void* desc) {
    auto* bb = (zcc_basic_block_data_t*)desc;
    _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_UNIT),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_JUMP,
            .data.jump = {
                .target = bb->ptr,
                .args = koopa_slice(KOOPA_RSIK_VALUE)
            }
        }
    });
}

void KoopaEnv::CreateStore(void* value, void* dest) {
    _CreateInst(new koopa_raw_value_data_t {
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
    _CreateInst(new koopa_raw_value_data_t {
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
        .name = to_string("@" + name),
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

void KoopaEnv::SetInserPointer(void* ptr) {
    auto* bb = (zcc_basic_block_data_t*)ptr;
    insert_ptr = &bb->insts;
}

void* KoopaEnv::GetFunction() {
    return (void*)&funcs.back();
}

void* KoopaEnv::GetInt32Type() {
    return (void*)koopa_type(KOOPA_RTT_INT32);
}

void* KoopaEnv::GetInt32(int value) {
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

bool KoopaEnv::EndWithTerminator() {
    auto* basic_block = (zcc_value_vec_t*)insert_ptr;
    return !basic_block->empty() && _IsTerminator(basic_block->back());
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

void* KoopaEnv::_CreateInst(koopa_raw_value_t value) {
    insert_ptr->push_back(value);
    return (void*)value;
}

koopa_raw_basic_block_t KoopaEnv::_ParseBasicBlock(const zcc_basic_block_data_t& bbs) {
    std::vector<koopa_raw_value_t> insts;
    bool isTerminator = false;
    for (const auto& inst : bbs.insts) {
        if (!isTerminator) {
            insts.push_back(inst);
        }
        if (_IsTerminator(inst)) {
            isTerminator = true;
        }
    }
    bbs.ptr->insts = koopa_slice(KOOPA_RSIK_VALUE, insts);
    return bbs.ptr;
}

koopa_raw_function_t KoopaEnv::_ParseFunction(const zcc_function_data_t& funcs) {
    std::vector<koopa_raw_basic_block_t> bbs;
    for (const auto& bb : funcs.bbs) {
        bbs.push_back(_ParseBasicBlock(bb));
    }
    return new koopa_raw_function_data_t {
        .ty = funcs.type,
        .name = to_string(funcs.name),
        .params = koopa_slice(KOOPA_RSIK_VALUE),
        .bbs = koopa_slice(KOOPA_RSIK_BASIC_BLOCK, bbs)
    };
}

int KoopaEnv::_ParseProgram() {
    if (!funcs.empty()) {
        std::vector<koopa_raw_function_t> func_vec;
        for (const auto& func : funcs) {
            func_vec.push_back(_ParseFunction(func));
        }
        raw_program = new koopa_raw_program_t {
            .values = koopa_slice(KOOPA_RSIK_VALUE),
            .funcs = koopa_slice(KOOPA_RSIK_FUNCTION, func_vec)
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