#include "Scanner.h"

#include "sysy.tab.hpp"
#include "sysy.lex.hpp"

Scanner::Scanner() {
    yylex_init(&lexer);
    parser = std::make_unique<yy::Parser>(lexer, *loc, *this);
    loc = std::make_unique<yy::location>();
}

Scanner::~Scanner() {
    yylex_destroy(lexer);
}

int Scanner::parse(FILE* input) {
    yyset_in(input, lexer);
    return parser->parse();
}