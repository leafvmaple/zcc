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
    KoopaEnv() {
        locals.push_back({});  // global scope
        types.push_back({});
    }

    void _save_basic_block() {
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

    void _save_function() {
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

    int _save_program() {
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

    void Print() {
        if (_save_program()) {
            koopa_dump_to_stdout(program);
        }
    }

    void Dump(const std::string& outfile) {
        if (_save_program()) {
            koopa_dump_to_file(program, outfile.c_str());
        }
    }

    void* CreateFuncType(void* retType) {
        return new koopa_raw_type_kind_t {
            KOOPA_RTT_FUNCTION,
            .data.function = {
                .params = koopa_slice(KOOPA_RSIK_TYPE),
                .ret = (koopa_raw_type_t)retType
            }
        };
    }

    void CreateFunction(void* funcType, const std::string& name) {
        _save_function();

        func_name = name;
        func_type = (koopa_raw_type_t)funcType;
    }

    void CreateStore(void* value, void* dest) {
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

    void* CreateLoad(void* src) {
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

    void CreateRet(void* value) {
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
    void CreateBasicBlock(const std::string& name) {
        _save_basic_block();

        bb_name = name;
    }

    void* CreateAnd(void* lhs, void* rhs) {
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

    void* CreateOr(void* lhs, void* rhs) {
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

    void* CreateAdd(void* lhs, void* rhs) {
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

    void* CreateSub(void* lhs, void* rhs) {
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

    void* CreateMul(void* lhs, void* rhs) {
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

    void* CreateDiv(void* lhs, void* rhs) {
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

    void* CreateMod(void* lhs, void* rhs) {
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

    void* CreateAlloca(void* type, const std::string& name) {
        return _CreateInst(new koopa_raw_value_data_t {
            .ty = koopa_type(KOOPA_RTT_INT32),
            .name = to_string(name),
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_ALLOC,
            }
        });
    }

    void* CreateICmpNE(void* lhs, void* rhs) {
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

    void* CreateICmpEQ(void* lhs, void* rhs) {
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

    void* CreateICmpLT(void* lhs, void* rhs) {
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

    void* CreateICmpGT(void* lhs, void* rhs) {
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

    void* CreateICmpLE(void* lhs, void* rhs) {
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

    void* CreateICmpGE(void* lhs, void* rhs) {
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

    void EnterScope() {
        locals.push_back({});
        types.push_back({});
    }

    void* _CreateInst(koopa_raw_value_t value) {
        insts.push_back(value);
        return (void*)value;
    }

    void ExitScope() {
        locals.pop_back();
        types.pop_back();
    }

    void AddSymbol(const std::string& name, VAR_TYPE type, void* value) {
        auto value_t = (koopa_raw_value_t)value;
        locals.back()[name] = value_t;
        types.back()[value_t] = type;
    }

    void* GetSymbolValue(const std::string& name) {
        for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return (void*)found->second;  // Return the value if found in the current scope
            }
        }

        return nullptr;  // Symbol not found
    }

    VAR_TYPE GetSymbolType(void* value) {
        for (auto it = types.rbegin(); it != types.rend(); ++it) {
            auto found = it->find((koopa_raw_value_t)value);
            if (found != it->end()) {
                return found->second;
            }
        }

        return VAR_TYPE::VAR;
    }

private:
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
