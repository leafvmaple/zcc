#pragma once

#include <fstream>
#include <string>
#include <memory>

#include "ast/ast.h"
#include "ast/type.h"

namespace yy {
    class Parser;
    class location;
}

class Scanner {
public:
    Scanner();
    ~Scanner();

    template<typename Type, typename Value, typename BasicBlock, typename Function>
    void Parse(FILE* input, Env<Type, Value, BasicBlock, Function>* env);
    
    CompUnitAST ast;

private:
    void* lexer;
    std::unique_ptr<yy::Parser> parser;
    std::unique_ptr<yy::location> loc;
};
