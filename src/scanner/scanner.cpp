#include "scanner.h"
#include "generator.h"

#include "../libkoopa/include/koopa.h"

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

template<typename V>
void Scanner::Parse(FILE* input, Env<V>* env) {
    yyset_in(input, lexer);
    int ret = parser->parse();
    if (ret == 0) {
        Generator<V> generator(env);
        generator.Generate(ast);
    } else {
        fprintf(stderr, "Parse error at %s:%d:%d\n",
                loc->begin.filename ? loc->begin.filename->c_str() : "unknown",
                loc->begin.line, loc->begin.column);
    }
}

template void Scanner::Parse<llvm::Value>(FILE* input, Env<llvm::Value>* env);
template void Scanner::Parse<koopa_raw_value_data>(FILE* input, Env<koopa_raw_value_data>* env);