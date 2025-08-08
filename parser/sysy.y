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
  #include "scanner/scanner.h"
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

%token INT VOID
%token RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token AND OR EQ NE LE GE
%token <std::string> IDENT
%token <int> INT_CONST

%token END 0

%type <std::vector<std::unique_ptr<ExprAST>>> FuncRParams
%type <std::vector<std::unique_ptr<ConstDefAST>>> ConstDefs
%type <std::vector<std::unique_ptr<VarDefAST>>> VarDefs
%type <std::vector<std::unique_ptr<FuncFParamAST>>> FuncFParams

%type <std::unique_ptr<ExprAST>> OptExpr

%type <std::unique_ptr<FuncDefAST>> FuncDef
%type <std::unique_ptr<FuncFParamAST>> FuncFParam

%type <std::unique_ptr<BlockAST>> Block
%type <std::vector<std::unique_ptr<BlockItemAST>>> BlockItems
%type <std::unique_ptr<BlockItemAST>> BlockItem
%type <std::unique_ptr<DeclAST>> Decl
%type <std::unique_ptr<StmtAST>> Stmt MatchedStmt UnmatchedStmt
%type <std::unique_ptr<NumberAST>> Number
%type <std::unique_ptr<LValAST>> LVal
%type <std::unique_ptr<ExprAST>> Expr

%type <std::unique_ptr<UnaryExprAST>> UnaryExpr
%type <std::unique_ptr<PrimaryExprAST>> PrimaryExpr
%type <std::unique_ptr<MulExprAST>> MulExpr
%type <std::unique_ptr<AddExprAST>> AddExpr
%type <std::unique_ptr<RelExprAST>> RelExpr
%type <std::unique_ptr<EqExprAST>> EqExpr
%type <std::unique_ptr<LAndExprAST>> LAndExpr
%type <std::unique_ptr<LOrExprAST>> LOrExpr
%type <std::unique_ptr<ConstExprAST>> ConstExpr

%type <std::unique_ptr<ConstDeclAST>> ConstDecl
%type <std::unique_ptr<VarDeclAST>> VarDecl
%type <std::unique_ptr<ConstInitValAST>> ConstInitVal
%type <std::unique_ptr<InitValAST>> InitVal

%type <std::unique_ptr<BaseType>> BasicType
%type <std::unique_ptr<ConstDefAST>> ConstDef
%type <std::unique_ptr<VarDefAST>> VarDef
%type <std::string> UnaryOp MulOp AddOp

%%

CompUnit : | CompUnit FuncDef {
  ctx.ast.AddFuncDef(std::move($2));
} | CompUnit Decl {
  ctx.ast.AddDecl(std::move($2));
};

FuncDef : BasicType IDENT '(' FuncFParams ')' Block {
  $$ = std::make_unique<FuncDefAST>(std::move($1), $2, std::move($4), std::move($6));
} | BasicType IDENT '(' ')' Block {
  $$ = std::make_unique<FuncDefAST>(std::move($1), $2, std::move($5));
};

BasicType : INT {
  $$ = std::make_unique<BaseType>(BaseType::TYPE::INT);
} | VOID {
  $$ = std::make_unique<BaseType>(BaseType::TYPE::VOID);
};

FuncFParams : FuncFParam {
  $$ = std::vector<std::unique_ptr<FuncFParamAST>>();
  $$.emplace_back(std::move($1));
} | FuncFParams ',' FuncFParam {
  $1.emplace_back(std::move($3));
  $$ = std::move($1);
};

FuncFParam : BasicType IDENT {
  $$ = std::make_unique<FuncFParamAST>(std::move($1), $2);
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
  $$ = std::vector<std::unique_ptr<BlockItemAST>>();
} | BlockItems BlockItem {
  $1.emplace_back(std::move($2));
  $$ = std::move($1);
}

Stmt : MatchedStmt {
  $$ = std::move($1);
} | UnmatchedStmt {
  $$ = std::move($1);
};

MatchedStmt : LVal '=' Expr ';'{
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Assign, std::move($1), std::move($3));
} | OptExpr ';' {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Expr, std::move($1));
} | Block {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Block, std::move($1));
} | IF '(' Expr ')' MatchedStmt ELSE MatchedStmt {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::If, std::move($3), std::move($5), std::move($7));
} | RETURN Expr ';' {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Ret, std::move($2));
} | WHILE '(' Expr ')' MatchedStmt {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::While, std::move($3), std::move($5));
} | BREAK ';' {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Break);
} | CONTINUE ';' {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::Continue);
};

UnmatchedStmt: IF '(' Expr ')' Stmt {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::If, std::move($3), std::move($5));
} | IF '(' Expr ')' MatchedStmt ELSE UnmatchedStmt {
  $$ = std::make_unique<StmtAST>(StmtAST::Type::If, std::move($3), std::move($5), std::move($7));
};

OptExpr : Expr {
  $$ = std::move($1);
} | {
  $$ = std::unique_ptr<ExprAST>();
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
} | IDENT '(' ')' {
  $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::Type::Call, $1);
} | IDENT '(' FuncRParams ')' {
  $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::Type::Call, $1, std::move($3));
}

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

ConstDecl : CONST BasicType ConstDefs ';' {
  $$ = std::make_unique<ConstDeclAST>(std::move($2), std::move($3));
}

VarDecl : BasicType VarDefs ';' {
  $$ = std::make_unique<VarDeclAST>(std::move($1), std::move($2));
}

ConstDefs : ConstDef {
  $$ = std::vector<std::unique_ptr<ConstDefAST>>();
  $$.emplace_back(std::move($1));
} | ConstDefs ',' ConstDef {
  $1.emplace_back(std::move($3));
  $$ = std::move($1);
}

ConstDef : IDENT '=' ConstInitVal {
  $$ = std::make_unique<ConstDefAST>($1, std::move($3));
}

VarDefs : VarDef {
  $$ = std::vector<std::unique_ptr<VarDefAST>>();
  $$.emplace_back(std::move($1));
} | VarDefs ',' VarDef {
  $1.emplace_back(std::move($3));
  $$ = std::move($1);
}

VarDef : IDENT {
  $$ = std::make_unique<VarDefAST>($1);
} | IDENT '=' InitVal {
  $$ = std::make_unique<VarDefAST>($1, std::move($3));
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

FuncRParams : Expr {
  $$ = std::vector<std::unique_ptr<ExprAST>>();
  $$.emplace_back(std::move($1));
} | FuncRParams Expr {
  $1.emplace_back(std::move($2));
  $$ = std::move($1);
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