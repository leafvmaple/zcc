%require "3.0.4"
%skeleton "lalr1.cc"
%define api.parser.class {Parser}
%define api.token.constructor
%define api.value.type variant
%define api.prefix {yy}

%define parse.trace
%define parse.error verbose

%defines
%locations

%code requires {
  #include <memory>
  #include <string>
  #include "Scanner.h"
}

%code
{
  yy::Parser::symbol_type yylex(void* yyscanner, yy::location& loc);
}

%{

#include <iostream>
#include <memory>
#include <string>

int yylex();
void yyerror(std::unique_ptr<std::string> &ast, const char *s);

%}

%lex-param {void *scanner} {yy::location& loc}
%parse-param {void *scanner} {yy::location& loc} { class Scanner& ctx }

%token INT RETURN
%token <std::string> IDENT
%token <int> INT_CONST

%token END 0

%type <std::string> FuncDef FuncType Block Stmt Number

%%

CompUnit : FuncDef {
  ctx.ast = $1;
};

FuncDef : FuncType IDENT '(' ')' Block {
  $$ = $1 + " " + $2 + "() " + $5;
};

FuncType : INT {
  $$ = "int";
};

Block : '{' Stmt '}' {
  $$ = "{ " + $2 + " }";
};

Stmt : RETURN Number ';' {
  $$ = "return " + $2 + ";";
};

Number : INT_CONST {
  $$ = std::to_string($1);
};

%%

void yy::Parser::error(const yy::location& l, const std::string& m)
{
	std::cerr << l << ": " << m << std::endl;
}