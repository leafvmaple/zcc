#include "Scanner.h"

#include "sysy.tab.hpp"
#include "sysy.lex.hpp"

Scanner::Scanner() {
    yylex_init(&lexer);
    parser = new yy::Parser(lexer, *loc, *this);
    loc = new yy::location();
}

Scanner::~Scanner() {
    yylex_destroy(lexer);
    delete parser;
    delete loc;
}

int Scanner::parse(FILE* input) {
    yyset_in(input, lexer);
    return parser->parse();
}