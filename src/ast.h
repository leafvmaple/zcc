#pragma once

#include <memory>
#include <string>
#include <vector>

using std::unique_ptr;
using std::string;

class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual std::string ToString() const = 0;
};

class CompUnitAST : public BaseAST {
public:
    void AddFuncDef(unique_ptr<BaseAST>&& funcDef);
    std::string ToString() const override;
private:
    unique_ptr<BaseAST> funcDef;
};

class FuncDefAST : public BaseAST {
public:
    FuncDefAST(unique_ptr<BaseAST>&& funcType, std::string ident, unique_ptr<BaseAST>&& block)
    : funcType(std::move(funcType)), ident(std::move(ident)), block(std::move(block)) {}
    std::string ToString() const override;
private:
    unique_ptr<BaseAST> funcType;
    std::string ident;
    unique_ptr<BaseAST> block;
};

class FuncTypeAST : public BaseAST {
public:
    FuncTypeAST(std::string type) : type(std::move(type)) {}
    std::string ToString() const override;
private:
    std::string type;
};

class BlockAST : public BaseAST {
public:
    BlockAST(unique_ptr<BaseAST>&& stmts) : stmts(std::move(stmts)) {}
    std::string ToString() const override;
private:
    unique_ptr<BaseAST> stmts;
};

class StmtAST : public BaseAST {
public:
    StmtAST(unique_ptr<BaseAST>&& num) : num(std::move(num)) {};
    std::string ToString() const override;
private:
    unique_ptr<BaseAST> num;
};

class NumAST : public BaseAST {
public:
    NumAST(int value) : value(value) {}
    std::string ToString() const override;
private:
    int value;
};