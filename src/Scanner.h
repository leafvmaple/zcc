#pragma once

#include <fstream>
#include <string>

namespace yy {
    class Parser;
    class location;
}

class Scanner {
public:
    Scanner();
    ~Scanner();

    int parse(FILE* input);

    void* lexer;
    yy::Parser* parser;
    yy::location* loc;

    std::string ast;
};