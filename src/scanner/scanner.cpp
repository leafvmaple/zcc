#include "scanner.h"

#include "sysy.tab.hpp"
#include "sysy.lex.hpp"

Scanner::Scanner() {
    yylex_init(&lexer);
    loc = std::make_unique<yy::location>();

    parser = std::make_unique<yy::Parser>(lexer, *loc, *this);
    // parser->set_debug_level(1);
}

Scanner::~Scanner() {
    yylex_destroy(lexer);
}

int Scanner::parse(FILE* input, std::unique_ptr<CompUnitAST>&& ast) {
    yyset_in(input, lexer);
    this->ast = std::move(ast);
    return parser->parse();
}