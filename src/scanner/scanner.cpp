#include "scanner.h"
#include "generator.h"

#include "../libkoopa/include/koopa.h"
#include "../libkoopa/include/function.h"

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

template<typename T, typename V, typename B, typename F>
void Scanner::Parse(FILE* input, Env<T, V, B, F>* env) {
    yyset_in(input, lexer);
    int ret = parser->parse();
    if (ret == 0) {
        Generator<T, V, B, F> generator(env);
        generator.Generate(ast);
    } else {
        fprintf(stderr, "Parse error at %s:%d:%d\n",
                loc->begin.filename ? loc->begin.filename->c_str() : "unknown",
                loc->begin.line, loc->begin.column);
    }
}

template void Scanner::Parse(FILE* input, Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>* env);
template void Scanner::Parse(FILE* input, Env<koopa_raw_type_kind_t, koopa_raw_value_data, zcc_basic_block_data_t, zcc_function_data_t>* env);