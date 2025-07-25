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

%type <std::unique_ptr<BaseAST>> FuncDef Block Stmt Number Expr UnaryExpr PrimaryExpr MulExpr AddExpr
%type <std::unique_ptr<BaseType>> FuncType
%type <std::string> UnaryOp MulOp AddOp

%%

CompUnit : FuncDef {
  ctx.ast->AddFuncDef(std::move($1));
};

FuncDef : FuncType IDENT '(' ')' Block {
  $$ = std::make_unique<FuncDefAST>(std::move($1), $2, std::move($5));
};

FuncType : INT {
  $$ = std::make_unique<IntType>();
};

Block : '{' Stmt '}' {
  $$ = std::make_unique<BlockAST>(std::move($2));
};

Stmt : RETURN Expr ';' {
  $$ = std::make_unique<StmtAST>(std::move($2));
};

Expr : AddExpr {
  $$ = std::make_unique<ExprAST>(std::move($1));
};

PrimaryExpr : '(' Expr ')' {
  $$ = std::make_unique<PrimaryExprAST>(PrimaryExprAST::Type::Expr, std::move($2));
} | Number {
  $$ = std::make_unique<PrimaryExprAST>(PrimaryExprAST::Type::Number, std::move($1));
};

Number : INT_CONST {
  $$ = std::make_unique<NumberAST>(std::move($1));
};

UnaryExpr : PrimaryExpr {
  $$ = std::make_unique<UnaryExprAST>(std::move($1));
} | UnaryOp UnaryExpr {
  $$ = std::make_unique<UnaryExprAST>($1, std::move($2));
};

MulExpr : UnaryExpr {
  $$ = std::make_unique<MulExprAST>(std::move($1));
} | MulExpr MulOp UnaryExpr {
  $$ = std::make_unique<MulExprAST>(std::move($1), $2, std::move($3));
};

AddExpr : MulExpr {
  $$ = std::make_unique<AddExprAST>(std::move($1));
} | AddExpr AddOp MulExpr {
  $$ = std::make_unique<AddExprAST>(std::move($1), $2, std::move($3));
};

UnaryOp : '+'  {
  $$ = "+";
} | '-' {
  $$ = "-";
} | '!' {
  $$ = "!";
};

MulOp : '*' {
  $$ = "*";
} | '/' {
  $$ = "/";
} | '%' {
  $$ = "%";
};

AddOp : '+' {
  $$ = "+";
} | '-' {
  $$ = "-";
};

%%

void yy::Parser::error(const yy::location& l, const std::string& m)
{
	std::cerr << l << ": " << m << std::endl;
}