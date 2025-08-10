#include "scanner.h"

#include "../libkoopa/include/function.h"
#include "llvm/IR/Function.h"

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

template<typename Type, typename Value, typename BasicBlock, typename Function>
void Scanner::Parse(FILE* input, Env<Type, Value, BasicBlock, Function>* env) {
    yyset_in(input, lexer);
    int ret = parser->parse();
    if (ret == 0) {
        ast.Codegen(env);
    } else {
        fprintf(stderr, "Parse error at %s:%d:%d\n",
                loc->begin.filename ? loc->begin.filename->c_str() : "unknown",
                loc->begin.line, loc->begin.column);
    }
}

template void Scanner::Parse(FILE* input, Env<llvm::Type, llvm::Value, llvm::BasicBlock, llvm::Function>* env);
template void Scanner::Parse(FILE* input, Env<koopa::Type, koopa::Value, koopa::BasicBlock, koopa::Function>* env);