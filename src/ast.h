#pragma once

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/Function.h"

#include "type.h"

#include "../libkoopa/include/koopa.h"
#include "../libkoopa/include/utils.h"

using std::unique_ptr;
using std::vector;
using std::string;

class LLVMParams;

struct BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual string ToString() const { return ""; };
    virtual llvm::Value* Codegen(LLVMParams* params) = 0;
    virtual void* Parse() {
        // Default implementation does nothing
        return nullptr;
    }
};

class DefineAST {
public:
    DefineAST(string ident) : ident(std::move(ident)) {}
    DefineAST(string ident, unique_ptr<BaseAST>&& initVal)
        : ident(std::move(ident)), initVal(std::move(initVal)) {}

    string ident;
    unique_ptr<BaseAST> initVal;
};

struct CompUnitAST : public BaseAST {
public:
    void AddFuncDef(unique_ptr<BaseAST>&& funcDef);

    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return new koopa_raw_program_t {
            .values = slice(KOOPA_RSIK_VALUE),
            .funcs = slice(KOOPA_RSIK_FUNCTION, funcDef)
        };
    }
protected:
    vector<unique_ptr<BaseAST>> funcDef;
};

class FuncDefAST : public BaseAST {
public:
    FuncDefAST(unique_ptr<BaseType>&& funcType, string ident, unique_ptr<BaseAST>&& block)
        : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}
        
    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return new koopa_raw_function_data_t {
            .ty = new koopa_raw_type_kind {
                KOOPA_RTT_FUNCTION,
                .data.function = {
                    .params = slice(KOOPA_RSIK_TYPE),
                    .ret = (koopa_raw_type_t)(funcType->Parse())
                }
            },
            .name = "@main",
            .params = slice(KOOPA_RSIK_VALUE),
            .bbs = slice(KOOPA_RSIK_BASIC_BLOCK, block)
        };
    }
protected:
    unique_ptr<BaseType> funcType;
    string ident;
    unique_ptr<BaseAST> block;
};

class BlockAST : public BaseAST {
public:
    BlockAST(vector<unique_ptr<BaseAST>>&& blocks)
        : blocks(std::move(blocks)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return new koopa_raw_basic_block_data_t {
            .name = "%entry",  // No name for the block
            .params = slice(KOOPA_RSIK_VALUE),
            .used_by = slice(KOOPA_RSIK_VALUE),
            .insts = slice(KOOPA_RSIK_VALUE, blocks),
        };
    }
protected:
    vector<unique_ptr<BaseAST>> blocks;
};

class StmtAST : public BaseAST {
public:
    enum class Type {
        Assign,
        Expr,
        Block,
        If,
        Ret,
    };
    StmtAST(Type type)
        : type(type) {};
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1)
        : type(type), expr1(std::move(expr1)) {};
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1, unique_ptr<BaseAST>&& expr2)
        : type(type), expr1(std::move(expr1)), expr2(std::move(expr2)) {}
    StmtAST(Type type, unique_ptr<BaseAST>&& expr1, unique_ptr<BaseAST>&& expr2, unique_ptr<BaseAST>&& expr3)
        : type(type), expr1(std::move(expr1)), expr2(std::move(expr2)), expr3(std::move(expr3)) {}

    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return new koopa_raw_value_data_t {
            .ty = type_kind(KOOPA_RTT_UNIT),
            .name = nullptr,
            .used_by = slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_RETURN,
                .data.ret = {
                    .value = (koopa_raw_value_t)expr1->Parse()
                }
            }
        };
    }
protected:
    Type type;
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
    unique_ptr<BaseAST> expr3;
};

class ExprAST : public BaseAST {
public:
    ExprAST(unique_ptr<BaseAST>&& expr)
        : expr(std::move(expr)) {};
    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return expr->Parse();
    }
private:
    unique_ptr<BaseAST> expr;
};

class PrimaryExprAST : public BaseAST {
public:
    enum class Type {
        Expr,
        LVal,
        Number
    };

    PrimaryExprAST(Type type, unique_ptr<BaseAST>&& ast) {
        this->type = type;
        this->ast = std::move(ast);
    }
    string ToString() const override { return "PrimaryExprAST {" + ast->ToString() + "}"; }
    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return ast->Parse();
    }
private:
    Type type;
    unique_ptr<BaseAST> ast;
};

class NumberAST : public BaseAST {
public:
    NumberAST(int value) : value(value) {}
    string ToString() const override { return "NumberAST {" + std::to_string(value); + "}"; }
    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return new koopa_raw_value_data_t {
            .ty = type_kind(KOOPA_RTT_INT32),
            .name = nullptr,
            .used_by = slice(KOOPA_RSIK_VALUE),
            .kind = {
                .tag = KOOPA_RVT_INTEGER,
                .data.integer = {
                    .value = value
                }
            }
        };
    }
protected:
    int value;
};

class UnaryExprAST : public BaseAST {
public:
    UnaryExprAST(unique_ptr<BaseAST>&& expr) {
        this->primaryExpr = std::move(expr);
    }
    UnaryExprAST(string op, unique_ptr<BaseAST>&& expr) {
        this->op = std::move(op);
        this->unaryExpr = std::move(expr);
    }
    string ToString() const override {
        auto content = primaryExpr ? primaryExpr->ToString() : (op + ", " + unaryExpr->ToString());
        return "UnaryExprAST { " + content + " }";
    }
    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return primaryExpr->Parse();
    }
private:
    unique_ptr<BaseAST> primaryExpr;
    string op;
    unique_ptr<BaseAST> unaryExpr;
};

class MulExprAST : public BaseAST {
public:
    MulExprAST(unique_ptr<BaseAST>&& expr) : unaryExpr(std::move(expr)) {}
    MulExprAST(unique_ptr<BaseAST>&& left, string op, unique_ptr<BaseAST>&& right)
        : unaryExpr(std::move(right)), mulExpr(std::move(left)), op(std::move(op)) {}
    string ToString() const override {
        if (mulExpr) {
            return "MulExprAST { " + mulExpr->ToString() + ", " + op + ", " + unaryExpr->ToString() + " }";
        } else {
            return "MulExprAST { " + unaryExpr->ToString() + " }";
        }
    }
    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return unaryExpr->Parse();
    }
private:
    unique_ptr<BaseAST> unaryExpr;
    unique_ptr<BaseAST> mulExpr;
    string op;
};

class AddExprAST : public BaseAST {
public:
    AddExprAST(unique_ptr<BaseAST>&& expr) : mulExpr(std::move(expr)) {}
    AddExprAST(unique_ptr<BaseAST>&& left, string op, unique_ptr<BaseAST>&& right)
        : mulExpr(std::move(right)), addExpr(std::move(left)), op(std::move(op))  {}
    string ToString() const override {
        if (addExpr) {
            return "AddExprAST { " + addExpr->ToString() + ", " + op + ", " + mulExpr->ToString() + " }";
        } else {
            return "AddExprAST { " + mulExpr->ToString() + " }";
        }
    }
    void* Parse() override {
        return mulExpr->Parse();
    }
    llvm::Value* Codegen(LLVMParams* params) override; 
private:
    unique_ptr<BaseAST> mulExpr;
    unique_ptr<BaseAST> addExpr;
    string op;
};

class RelExprAST : public BaseAST {
public:
    enum class Op {
        LT,
        GT,
        LE,
        GE,
    };
    RelExprAST(unique_ptr<BaseAST>&& expr) : expr1(std::move(expr)) {}
    RelExprAST(unique_ptr<BaseAST>&& left, Op op, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)), op(std::move(op)) {}
    string ToString() const override {
        if (expr2) {
            return "RelExprAST { " + expr1->ToString() + ", " + std::to_string(static_cast<int>(op)) + ", " + expr2->ToString() + " }";
        } else {
            return "RelExprAST { " + expr1->ToString() + " }";
        }
    }
    llvm::Value* Codegen(LLVMParams* params) override;
    void* Parse() override {
        return expr1->Parse();
    }
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
    Op op;
};

class EqExprAST : public BaseAST {
public:
    enum class Op {
        EQ,
        NE,
    };
    EqExprAST(unique_ptr<BaseAST>&& expr) : expr1(std::move(expr)) {}
    EqExprAST(unique_ptr<BaseAST>&& left, Op op, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)), op(std::move(op)) {}
    string ToString() const override {
        if (expr2) {
            return "EqExprAST { " + expr1->ToString() + ", " + std::to_string(static_cast<int>(op)) + ", " + expr2->ToString() + " }";
        } else {
            return "EqExprAST { " + expr1->ToString() + " }";
        }
    }
    void* Parse() override {
        return expr1->Parse();
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
    Op op;
};

class LAndExprAST : public BaseAST {
public:
    LAndExprAST(unique_ptr<BaseAST>&& expr) : expr1(std::move(expr)) {}
    LAndExprAST(unique_ptr<BaseAST>&& left, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)) {}
    string ToString() const override {
        if (expr2) {
            return "LAndExprAST { " + expr1->ToString() + ", &&, " + expr2->ToString() + " }";
        } else {
            return "LAndExprAST { " + expr1->ToString() + " }";
        }
    }
    void* Parse() override {
        return expr1->Parse();
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class LOrExprAST : public BaseAST {
public:
    LOrExprAST(unique_ptr<BaseAST>&& expr) : expr1(std::move(expr)) {}
    LOrExprAST(unique_ptr<BaseAST>&& left, unique_ptr<BaseAST>&& right)
        : expr1(std::move(left)), expr2(std::move(right)) {}
    string ToString() const override {
        if (expr2) {
            return "LOrExprAST { " + expr1->ToString() + ", ||, " + expr2->ToString() + " }";
        } else {
            return "LOrExprAST { " + expr1->ToString() + " }";
        }
    }
    void* Parse() override {
        return expr1->Parse();
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr1;
    unique_ptr<BaseAST> expr2;
};

class DeclAST : public BaseAST {
public:
    DeclAST(unique_ptr<BaseAST>&& constDecl) : constDecl(std::move(constDecl)) {}
    string ToString() const override { return "DeclAST { " + constDecl->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> constDecl;
};

class ConstDeclAST : public BaseAST {
public:
    ConstDeclAST(unique_ptr<BaseType>&& btype, unique_ptr<DefineAST>&& constDef)
        : btype(std::move(btype)), constDef(std::move(constDef)) {}
    string ToString() const override {
        return "ConstDeclAST { " + btype->ToString() + " }";
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseType> btype;
    unique_ptr<DefineAST> constDef;
};

class VarDeclAST : public BaseAST {
public:
    VarDeclAST(unique_ptr<BaseType>&& btype, unique_ptr<DefineAST>&& localDef)
        : btype(std::move(btype)), localDef(std::move(localDef)) {}
    string ToString() const override {
        return "VarDeclAST { " + btype->ToString() + ", " + localDef->ident + " }";
    };
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseType> btype;
    unique_ptr<DefineAST> localDef;
};

class ConstInitValAST : public BaseAST {
public:
    ConstInitValAST(unique_ptr<BaseAST>&& expr) : expr(std::move(expr)) {}
    string ToString() const override { return "ConstInitValAST { " + expr->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:    
    unique_ptr<BaseAST> expr;
};

class InitValAST : public BaseAST {
public:
    InitValAST(unique_ptr<BaseAST>&& expr) : expr(std::move(expr)) {}
    string ToString() const override { return "InitValAST { " + expr->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr;
};

class BlockItemAST : public BaseAST {
public:
    BlockItemAST(unique_ptr<BaseAST>&& ast) : ast(std::move(ast)) {}
    string ToString() const override {
        return "BlockItemAST { " + ast->ToString() + " }";
    }
    void *Parse() override {
        return ast->Parse();
    }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> ast;
};

class LValAST : public BaseAST {
public:
    LValAST(string ident) : ident(std::move(ident)) {}
    string ToString() const override { return "LValAST { " + ident + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    string ident;
};

class ConstExprAST : public BaseAST {
public:
    ConstExprAST(unique_ptr<BaseAST>&& expr) : expr(std::move(expr)) {}
    string ToString() const override { return "ConstExprAST { " + expr->ToString() + " }"; }
    llvm::Value* Codegen(LLVMParams* params) override;
private:
    unique_ptr<BaseAST> expr;
};