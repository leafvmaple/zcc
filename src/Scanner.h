#pragma once

#include <fstream>
#include <string>
#include <memory>

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
    std::unique_ptr<yy::Parser> parser;
    std::unique_ptr<yy::location> loc;

    std::string ast;
};