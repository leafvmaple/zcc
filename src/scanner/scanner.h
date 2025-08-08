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

    template<typename T, typename V, typename B, typename F>
    void Parse(FILE* input, Env<T, V, B, F>* env);
    
    CompUnitAST ast;

private:
    void* lexer;
    std::unique_ptr<yy::Parser> parser;
    std::unique_ptr<yy::location> loc;
};
