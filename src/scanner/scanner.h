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

class Env;

class Scanner {
public:
    Scanner();
    ~Scanner();

    void Parse(FILE* input, Env* env);
    
    CompUnitAST ast;

private:
    void* lexer;
    std::unique_ptr<yy::Parser> parser;
    std::unique_ptr<yy::location> loc;
};