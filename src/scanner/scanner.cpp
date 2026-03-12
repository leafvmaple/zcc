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
        fprintf(stderr, "Parse error at %s:%d:%d\n",
                loc->begin.filename ? loc->begin.filename->c_str() : "unknown",
                loc->begin.line, loc->begin.column);
    }
}
