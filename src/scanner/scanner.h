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

    int parse(FILE* input, std::unique_ptr<CompUnitAST>&& ast);

    void* lexer;
    std::unique_ptr<yy::Parser> parser;
    std::unique_ptr<yy::location> loc;

    std::unique_ptr<CompUnitAST> ast;
};