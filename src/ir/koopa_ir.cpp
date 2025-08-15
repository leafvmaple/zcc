#include "koopa_ir.h"

const int VEC_RESERVE_SIZE = 256;

koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind) {
    return {nullptr, 0, kind};
}

template<typename Type>
koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind, const Type& vec, KoopaEnv* env) {
    auto* buffer = new const void*[1];
    buffer[0] = vec->Codegen(env);
    return {buffer, 1, kind};
}

template<typename Type>
koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind, const std::vector<Type>& vec, KoopaEnv* env) {
    auto* buffer = new const void*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i) {
        buffer[i] = vec[i]->Codegen(env);
    }
    return {buffer, static_cast<uint32_t>(vec.size()), kind};
}

template<typename Type>
koopa_raw_slice_t inline koopa_slice(koopa_raw_slice_item_kind_t kind, const std::vector<Type>& vec) {
    auto* buffer = new const void*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i) {
        buffer[i] = vec[i];
    }
    return {buffer, static_cast<uint32_t>(vec.size()), kind};
}

inline koopa::Type* koopa_type(koopa_raw_type_tag_t tag) {
    return new koopa_raw_type_kind_t { tag };
}

inline koopa::Type* koopa_pointer(koopa::Type* type) {
    return new koopa_raw_type_kind_t {
        .tag = KOOPA_RTT_POINTER,
        .data.pointer = {
            .base = type
        }
    };
}

inline koopa::Type* koopa_element_type(const koopa::Type* type) {
    assert(type->tag == KOOPA_RTT_POINTER || type->tag == KOOPA_RTT_ARRAY);
    return const_cast<koopa::Type*>(type->data.pointer.base);
}

koopa_raw_value_t inline koopa_int(int value) {
    return new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_INTEGER,
            .data.integer = {
                .value = value
            }
        }
    };
}

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

void KoopaEnv::Optimize() {
    for (auto& func : funcs) {
        for (auto& bb : func.bbs) {
            std::vector<koopa_raw_value_t> insts;
            bool isTerminator = false;
            for (const auto& inst : bb->insts) {
                if (!isTerminator) {
                    insts.push_back(inst);
                }
                if (_IsTerminator(inst)) {
                    isTerminator = true;
                }
            }
            std::swap(bb->insts, insts);
        }
        if (!func.bbs.empty() && (func.bbs.back()->insts.empty() || !_IsTerminator(func.bbs.back()->insts.back()))) {
            func.bbs.back()->insts.push_back(new koopa_raw_value_data_t {
                .ty = koopa_type(KOOPA_RTT_UNIT),
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
            .ty = params[i],
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
            .ret = retType
        }
    };
}

koopa::Function* KoopaEnv::CreateFunction(koopa::Type* funcType, const std::string& name, std::vector<std::string> params) {
    koopa::Function func = {
        .name = name,
        .ptr = new koopa_raw_function_data_t {
            .ty = funcType,
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
    funcs.emplace_back(func);
    AddSymbol(name, VAR_TYPE::FUNC, { .function = &funcs.back() });
    return &funcs.back();
}

koopa::Value* KoopaEnv::CreateArray(koopa::Type* type, std::vector<koopa::Value*> values) {
    return new koopa_raw_value_data_t {
        .ty = type,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_AGGREGATE,
            .data.aggregate = {
                .elems = koopa_slice(KOOPA_RSIK_VALUE, values)
            }
        }
    };
}

void KoopaEnv::CreateBuiltin(const std::string& name, koopa::Type* retType, std::vector<koopa::Type*> params) {
    koopa::Function func = {
        .name = name,
        .ptr = new koopa_raw_function_data_t {
            .ty = CreateFuncType(retType, params),
            .name = to_string("@" + name),
            .params = koopa_slice(KOOPA_RSIK_VALUE),
            .bbs = koopa_slice(KOOPA_RSIK_BASIC_BLOCK)
        }
    };
    funcs.emplace_back(func);
    AddSymbol(name, VAR_TYPE::FUNC, { .function = &funcs.back() });
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
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BRANCH,
            .data.branch = {
                .cond = cond,
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
        .ty = GetValueType(value),
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
    koopa::Type* type = GetValueType(src);
    type = koopa_element_type(type);
    return (koopa::Value*)_CreateInst(new koopa::Value {
        .ty = type,
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
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_RETURN,
            .data.ret = {
                .value = value
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateCall(koopa::Function* func, std::vector<koopa::Value*> args) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = func->ptr->ty,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_CALL,
            .data.call = {
                .callee = func->ptr,
                .args = koopa_slice(KOOPA_RSIK_VALUE, args)
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateAnd(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_AND,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateOr(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_OR,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateAdd(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_ADD,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateSub(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_SUB,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateMul(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_MUL,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateDiv(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_DIV,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateMod(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_MOD,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateAlloca(koopa::Type* type, const std::string& name) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_pointer(type),
        .name = to_string("@" + name),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_ALLOC,
        }
    });
}

koopa::Value* KoopaEnv::CreateGlobal(koopa::Type* type, const std::string& name, koopa::Value* init) {
    auto* value = new koopa_raw_value_data_t {
        .ty = type,
        .name = to_string("@" + name),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_GLOBAL_ALLOC,
            .data.global_alloc = {
                .init = init
            }
        }
    };
    values.push_back(value);
    return value;
}

koopa::Value* KoopaEnv::CreateZero(koopa::Type* type) {
    return new koopa_raw_value_data_t {
        .ty = type,
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_ZERO_INIT
        }
    };
}

koopa::Value* KoopaEnv::CreateICmpNE(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_NOT_EQ,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateICmpEQ(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_EQ,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateICmpLT(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_LT,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateICmpGT(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_GT,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateICmpLE(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_LE,
                .lhs = lhs,
                .rhs = rhs
            }
        }
    });
}

koopa::Value* KoopaEnv::CreateICmpGE(koopa::Value* lhs, koopa::Value* rhs) {
    return _CreateInst(new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_BINARY,
            .data.binary = {
                .op = KOOPA_RBO_GE,
                .lhs = lhs,
                .rhs = rhs
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

koopa::Type* KoopaEnv::GetArrayType(koopa::Type* type, int size) {
    return new koopa_raw_type_kind_t {
        .tag = KOOPA_RTT_ARRAY,
        .data.array = {
            .base = type,
            .len = (size_t)size
        }
    };
}

koopa::Type* KoopaEnv::GetPointerType(koopa::Type* type) {
    return koopa_pointer(type);
}

koopa::Type* KoopaEnv::GetValueType(koopa::Value* value) {
    return const_cast<koopa::Type*>(value->ty);
}

koopa::Type* KoopaEnv::GetElementType(koopa::Type* type) {
    return koopa_element_type(type);
}

koopa::Value* KoopaEnv::GetInt32(int value) {
    return new koopa_raw_value_data_t {
        .ty = koopa_type(KOOPA_RTT_INT32),
        .used_by = koopa_slice(KOOPA_RSIK_VALUE),
        .kind = {
            .tag = KOOPA_RVT_INTEGER,
            .data.integer = {
                .value = value
            }
        }
    };
}

koopa::Value* KoopaEnv::CreateGEP(koopa::Type* type, koopa::Value* array, vector<koopa::Value*> indies, bool isPointer) {
    koopa::Value* res = array;
    for (const auto& index : indies) {
        type = GetValueType(res);
        assert(type->tag == KOOPA_RTT_POINTER);
        auto elem_type = koopa_element_type(type);
        auto tag = KOOPA_RVT_GET_ELEM_PTR;
        if (isPointer) {
            tag = KOOPA_RVT_GET_PTR;
            isPointer = false;
        } else {
            type = koopa_pointer(koopa_element_type(elem_type));
        }

        res = _CreateInst(new koopa_raw_value_data_t{
            .ty = type,
            .used_by = koopa_slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = tag,
                .data.get_elem_ptr = {
                    .src = res,
                    .index = index
                }
            }
        });
    }
    return res;
    // return CreateLoad(res);
}

koopa::Value* KoopaEnv::CaculateBinaryOp(const std::function<int(int, int)>& func, koopa::Value* lhs, koopa::Value* rhs) {
    int result = func(lhs->kind.data.integer.value, rhs->kind.data.integer.value);
    return GetInt32(result);
}

int KoopaEnv::GetValueInt(koopa::Value* value) {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        return value->kind.data.integer.value;
    }
    std::cerr << "Error: Value is not an integer." << std::endl;
    return 0; // or throw an exception
}

koopa::Value* KoopaEnv::GetArrayElement(koopa::Value* array, int index) {
    // TODO assert
    return (koopa::Value*)array->kind.data.aggregate.elems.buffer[index];
}

bool KoopaEnv::IsArrayType(koopa::Type* type) {
    return type->tag == KOOPA_RTT_ARRAY;
}

bool KoopaEnv::EndWithTerminator() {
    auto insts = insert_ptr->insts;
    return !insts.empty() && _IsTerminator(insts.back());
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
            .values = koopa_slice(KOOPA_RSIK_VALUE, values),
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