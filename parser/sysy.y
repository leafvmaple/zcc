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

%token INT RETURN CONST IF ELSE
%token AND OR EQ NE LE GE
%token <std::string> IDENT
%token <int> INT_CONST

%token END 0

%type <std::vector<std::unique_ptr<BaseAST>>> BlockItems
%type <std::unique_ptr<BaseAST>> OptExpr

%type <std::unique_ptr<BaseAST>> FuncDef Block BlockItem Stmt Number LVal
%type <std::unique_ptr<BaseAST>> Expr UnaryExpr PrimaryExpr MulExpr AddExpr RelExpr EqExpr LAndExpr LOrExpr ConstExpr
%type <std::unique_ptr<BaseAST>> Decl ConstDecl VarDecl
%type <std::unique_ptr<BaseAST>> ConstInitVal InitVal
%type <std::unique_ptr<BaseType>> BType
%type <std::unique_ptr<DefineAST>> ConstDef VarDef
%type <std::string> UnaryOp MulOp AddOp

%%

CompUnit : FuncDef {
  ctx.ast->AddFuncDef(std::move($1));
};

FuncDef : BType IDENT '(' ')' Block {
  $$ = std::make_unique<FuncDefAST>(std::move($1), $2, std::move($5));
};

BType : INT {
  $$ = std::make_unique<IntType>();
};

Block : '{' BlockItems '}' {
  $$ = std::make_unique<BlockAST>(std::move($2));
};

BlockItem : Decl {
  $$ = std::make_unique<BlockItemAST>(std::move($1));
} | Stmt {
  $$ = std::make_unique<BlockItemAST>(std::move($1));
}

BlockItems : {
  $$ = std::vector<std::unique_ptr<BaseAST>>();
} | BlockItems BlockItem {
  $1.emplace_back(std::move($2));
  $$ = std::move($1);
}

Stmt : LVal '=' Expr ';' {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Assign, std::move($1), std::move($3));
} | OptExpr ';' {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Expr, std::move($1));
} | Block {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Block, std::move($1));
} | IF '(' Expr ')' Stmt ELSE Stmt {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::If, std::move($3), std::move($5), std::move($7));
} | IF '(' Expr ')' Stmt {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::If, std::move($3), std::move($5));
} | RETURN Expr ';' {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Ret, std::move($2));
};

OptExpr : Expr {
  $$ = std::unique_ptr<BaseAST>(std::move($1));
} | {
  $$ = std::unique_ptr<BaseAST>();
};

Expr : LOrExpr {
  $$ = std::make_unique<ExprAST>(std::move($1));
};

PrimaryExpr : '(' Expr ')' {
  $$ = std::make_unique<PrimaryExprAST>(PrimaryExprAST::Type::Expr, std::move($2));
} | LVal {
  $$ = std::make_unique<PrimaryExprAST>(PrimaryExprAST::Type::LVal, std::move($1));
} | Number {
  $$ = std::make_unique<PrimaryExprAST>(PrimaryExprAST::Type::Number, std::move($1));
};

Number : INT_CONST {
  $$ = std::make_unique<NumberAST>(std::move($1));
};

UnaryExpr : PrimaryExpr {
  $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::Type::Primary, std::move($1));
} | UnaryOp UnaryExpr {
  $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::Type::Unary, $1, std::move($2));
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

RelExpr : AddExpr {
  $$ = std::make_unique<RelExprAST>(std::move($1));
} | RelExpr '<' AddExpr {
  $$ = std::make_unique<RelExprAST>(std::move($1), RelExprAST::Op::LT, std::move($3));
} | RelExpr '>' AddExpr {
  $$ = std::make_unique<RelExprAST>(std::move($1), RelExprAST::Op::GT, std::move($3));
} | RelExpr LE AddExpr {
  $$ = std::make_unique<RelExprAST>(std::move($1), RelExprAST::Op::LE, std::move($3));
} | RelExpr GE AddExpr {
  $$ = std::make_unique<RelExprAST>(std::move($1), RelExprAST::Op::GE, std::move($3));
};

EqExpr : RelExpr {
  $$ = std::make_unique<EqExprAST>(std::move($1));
} | EqExpr EQ RelExpr {
  $$ = std::make_unique<EqExprAST>(std::move($1), EqExprAST::Op::EQ, std::move($3));
} | EqExpr NE RelExpr {
  $$ = std::make_unique<EqExprAST>(std::move($1), EqExprAST::Op::NE, std::move($3));
};

LAndExpr : EqExpr {
  $$ = std::make_unique<LAndExprAST>(std::move($1));
} | LAndExpr AND EqExpr {
  $$ = std::make_unique<LAndExprAST>(std::move($1), std::move($3));
};

LOrExpr : LAndExpr {
  $$ = std::make_unique<LOrExprAST>(std::move($1));
} | LOrExpr OR LAndExpr {
  $$ = std::make_unique<LOrExprAST>(std::move($1), std::move($3));
};

Decl : ConstDecl {
  $$ = std::make_unique<DeclAST>(std::move($1));
} | VarDecl {
  $$ = std::make_unique<DeclAST>(std::move($1));
}

ConstDecl : CONST BType ConstDef ';' {
  $$ = std::make_unique<ConstDeclAST>(std::move($2), std::move($3));
}

VarDecl : BType VarDef ';' {
  $$ = std::make_unique<VarDeclAST>(std::move($1), std::move($2));
}

ConstDef : IDENT '=' ConstInitVal {
  $$ = std::make_unique<DefineAST>($1, std::move($3));
}

VarDef : IDENT {
  $$ = std::make_unique<DefineAST>($1);
} | IDENT '=' InitVal {
  $$ = std::make_unique<DefineAST>($1, std::move($3));
}

ConstInitVal : ConstExpr {
  $$ = std::make_unique<ConstInitValAST>(std::move($1));
}

InitVal : ConstExpr {
  $$ = std::make_unique<InitValAST>(std::move($1));
}

LVal : IDENT {
  $$ = std::make_unique<LValAST>($1);
}

ConstExpr : Expr {
  $$ = std::make_unique<ConstExprAST>(std::move($1));
}

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