#include "scanner.h"
#include "ir/codegen.h"

#include "sysy.tab.hpp"
#include "sysy.lex.hpp"

Scanner::Scanner() {
    yylex_init(&lexer);
    loc = std::make_unique<yy::location>();
    parser = std::make_unique<yy::Parser>(lexer, *loc, *this);
}

Scanner::~Scanner() {
    yylex_destroy(lexer);
}

void Scanner::Parse(FILE* input, CodeGen* cg) {
    yyset_in(input, lexer);
    int ret = parser->parse();
    if (ret == 0) {
        ast.Codegen(cg);
    } else {
        throw std::runtime_error("source parsing failed");
    }
}
