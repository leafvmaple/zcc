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

void KoopaEnv::Pass() {
    for (auto& func : funcs) {
        bool isEmpty = true;
        for (auto& bb : func.bbs) {
            std::vector<koopa_raw_value_t> insts;
            bool isTerminator = false;
            for (const auto& inst : bb->insts) {
                if (!isTerminator) {
                    insts.push_back(inst);
                    isEmpty = false;
                }
                if (_IsTerminator(inst)) {
                    isTerminator = true;
                }
            }
            std::swap(bb->insts, insts);
        }
        if (isEmpty) {
            func.bbs.back()->insts.push_back(new koopa_raw_value_data_t {
                .ty = koopa_type(KOOPA_RTT_UNIT),
                .name = nullptr,
                .used_by = koopa_slice(KOOPA_RSIK_VALUE),
                .kind = {
                    .tag = KOOPA_RVT_RETURN
                }
            });
        }
    }
}

koopa::Type* KoopaEnv::CreateFuncType(koopa::Type* retType, std::vector<koopa::Type*> params) {
    std::vector<koopa_raw_value_data_t*> byte;
    for (size_t i = 0; i < params.size(); ++i) {
        byte.push_back(new koopa_raw_value_data_t {
            .ty = (koopa_raw_type_t)params[i],
            .name = nullptr,
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_FUNC_ARG_REF,
                .data.func_arg_ref = {
                    .index = static_cast<uint32_t>(i)
                }
            }
        });
    }
    return new koopa_raw_type_kind_t {
        KOOPA_RTT_FUNCTION,
        .data.function = {
            .params = koopa_slice(KOOPA_RSIK_TYPE, params),
            .ret = (koopa_raw_type_t)retType
        }
    };
}

koopa::Function* KoopaEnv::CreateFunction(koopa::Type* funcType, const std::string& name, std::vector<std::string> params) {
    zcc_function_data_t func = {
        .name = name,
        .ptr = new koopa_raw_function_data_t {
            .ty = (koopa_raw_type_t)funcType,
            .name = to_string("@" + name),
            .params = koopa_slice(KOOPA_RSIK_VALUE),
            .bbs = koopa_slice(KOOPA_RSIK_BASIC_BLOCK)
        }
    };
    for (int i = 0; i < funcType->data.function.params.len; ++i) {
        func.params.push_back(new koopa_raw_value_data_t {
            .ty = (koopa_raw_type_t)funcType->data.function.params.buffer[i],
            .name = to_string("%" + params[i]),
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_FUNC_ARG_REF,
                .data.func_arg_ref = {
                    .index = static_cast<uint32_t>(i)
                }
            }
        });
    }
    AddSymbol(name, VAR_TYPE::FUNC, func.ptr);
    funcs.push_back(func);
    return &funcs.back();
}

koopa::BasicBlock* KoopaEnv::CreateBasicBlock(const std::string& name, koopa::Function* func) {
    func->bbs.push_back(new zcc_basic_block_data_t {
        .ptr = new koopa_raw_basic_block_data_t {
            .name = to_string("%" + name),
            .params = koopa_slice(KOOPA_RSIK_VALUE),
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .insts = koopa_slice(KOOPA_RSIK_VALUE)
        }
    });
    return func->bbs.back();
}

void KoopaEnv::CreateCondBr(koopa::Value* cond, koopa::BasicBlock* trueBB, koopa::BasicBlock* falseBB) {
    _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_UNIT),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BRANCH,
            .data.branch = {
                .cond = (koopa_raw_value_t)cond,
                .true_bb = trueBB->ptr,
                .false_bb = falseBB->ptr,
                .true_args = koopa_slice(KOOPA_RSIK_VALUE),
                .false_args = koopa_slice(KOOPA_RSIK_VALUE)
            }
        }
    });
}

void KoopaEnv::CreateBr(koopa::BasicBlock* desc) {
    _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_UNIT),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_JUMP,
            .data.jump = {
                .target = desc->ptr,
                .args = koopa_slice(KOOPA_RSIK_VALUE)
            }
        }
    });
}

void KoopaEnv::CreateStore(koopa::Value* value, koopa::Value* dest) {
    _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_STORE,
            .data.store = {
                .value = value,
                .dest = dest,
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateLoad(koopa::Value* src) {
    return (koopa::Value*)_CreateInst(new koopa::Value {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_LOAD,
            .data.load = {
                .src = src
            }
        }
    });
}

void KoopaEnv::CreateRet(koopa::Value* value) {
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

koopa::Value* KoopaEnv::CreateCall(void* func, std::vector<void*> args) {
    auto* function = (koopa_raw_function_t)func;
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = function->ty,
        .name = nullptr,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_CALL,
            .data.call = {
                .callee = function,
                .args = koopa_slice(KOOPA_RSIK_VALUE, args)
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateAnd(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateOr(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateAdd(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateSub(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateMul(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateDiv(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateMod(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateAlloca(koopa::Type* type, const std::string& name) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .name = to_string("@" + name),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_ALLOC,
        }
    });
}

koopa::Value* KoopaEnv::CreateICmpNE(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateICmpEQ(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateICmpLT(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateICmpGT(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateICmpLE(koopa::Value* lhs, koopa::Value* rhs) {
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

koopa::Value* KoopaEnv::CreateICmpGE(koopa::Value* lhs, koopa::Value* rhs) {
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

void KoopaEnv::SetInserPointer(koopa::BasicBlock* bb) {
    insert_ptr = bb;
}

koopa::Function* KoopaEnv::GetFunction() {
    return &funcs.back();
}

koopa::Value* KoopaEnv::GetFunctionArg(int index) {
    auto& func = funcs.back();
    return func.params[index];
}

koopa::Type* KoopaEnv::GetInt32Type() {
    return koopa_type(KOOPA_RTT_INT32);
}

koopa::Type* KoopaEnv::GetVoidType() {
    return koopa_type(KOOPA_RTT_UNIT);
}

koopa::Value* KoopaEnv::GetInt32(int value) {
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
    auto* basic_block = (koopa_inst_vec_t*)insert_ptr;
    return !basic_block->empty() && _IsTerminator(basic_block->back());
}

void KoopaEnv::AddSymbol(const std::string& name, VAR_TYPE type, void* value) {
    locals.back()[name] = value;
    types.back()[value] = type;
}

void* KoopaEnv::GetSymbolValue(const std::string& name) {
    for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return (void*)found->second;  // funnction or value
        }
    }
    return nullptr;
}

VAR_TYPE KoopaEnv::GetSymbolType(void* value) {
    for (auto it = types.rbegin(); it != types.rend(); ++it) {
        auto found = it->find(value);
        if (found != it->end()) {
            return found->second;
        }
    }
    return VAR_TYPE::VAR;
}

koopa::Value* KoopaEnv::_CreateInst(koopa::Value* value) {
    insert_ptr->insts.push_back(value);
    return value;
}

koopa_raw_basic_block_t KoopaEnv::_ParseBasicBlock(const zcc_basic_block_data_t& bbs) {
    bbs.ptr->insts = koopa_slice(KOOPA_RSIK_VALUE, bbs.insts);
    return bbs.ptr;
}

koopa_raw_function_t KoopaEnv::_ParseFunction(const zcc_function_data_t& funcs) {
    std::vector<koopa_raw_basic_block_t> bbs;
    for (const auto& bb : funcs.bbs) {
        bbs.push_back(_ParseBasicBlock(*bb));
    }
    funcs.ptr->bbs = koopa_slice(KOOPA_RSIK_BASIC_BLOCK, bbs);
    funcs.ptr->params = koopa_slice(KOOPA_RSIK_VALUE, funcs.params);
    return funcs.ptr;
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