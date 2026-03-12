/* ===================================================================
 * Bison configuration
 * =================================================================== */

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

/* ---------- early includes (available in header) ---------- */

%code requires {
    #include <memory>
    #include <string>
    #include "scanner/scanner.h"
}

/* ---------- yylex forward declaration ---------- */

%code {
    yy::Parser::symbol_type yylex(void* yyscanner, yy::location& loc);
}

/* ---------- scanner / parser parameters ---------- */

%lex-param   {void* scanner} {yy::location& loc}
%parse-param {void* scanner} {yy::location& loc} {class Scanner& ctx}

/* ===================================================================
 * Token declarations
 * =================================================================== */

/* keywords */
%token INT CHAR VOID
%token CONST RETURN IF ELSE WHILE FOR BREAK CONTINUE

/* multi-char operators (with precedence, low → high) */
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'

/* valued tokens */
%token <std::string> IDENT
%token <int>         INT_CONST
%token <std::string> STR_CONST

/* end-of-input */
%token END 0

/* ===================================================================
 * Non-terminal types
 * =================================================================== */

/* --- top-level & functions --- */
%type <std::unique_ptr<FuncDefAST>>    FuncDef
%type <std::unique_ptr<FuncFParamAST>> FuncFParam
%type <std::vector<std::unique_ptr<FuncFParamAST>>> FuncFParams
%type <std::vector<std::unique_ptr<ExprAST>>>       FuncRParams

/* --- blocks & statements --- */
%type <std::unique_ptr<BlockAST>>     Block
%type <std::unique_ptr<BlockItemAST>> BlockItem
%type <std::vector<std::unique_ptr<BlockItemAST>>> BlockItems
%type <std::unique_ptr<StmtAST>>      Stmt MatchedStmt UnmatchedStmt

/* --- declarations --- */
%type <std::unique_ptr<DeclAST>>      Decl
%type <std::unique_ptr<ConstDeclAST>> ConstDecl
%type <std::unique_ptr<VarDeclAST>>   VarDecl
%type <std::unique_ptr<ConstDefAST>>  ConstDef
%type <std::unique_ptr<VarDefAST>>    VarDef
%type <std::vector<std::unique_ptr<ConstDefAST>>> ConstDefs
%type <std::vector<std::unique_ptr<VarDefAST>>>   VarDefs

/* --- initializers --- */
%type <std::unique_ptr<ConstInitValAST>> ConstInitVal
%type <std::unique_ptr<InitValAST>>      InitVal
%type <std::vector<std::unique_ptr<ConstInitValAST>>> ConstInitVals
%type <std::vector<std::unique_ptr<InitValAST>>>      InitVals

/* --- expressions --- */
%type <std::unique_ptr<ExprAST>>        Expr
%type <std::unique_ptr<ConstExprAST>>   ConstExpr
%type <std::unique_ptr<PrimaryExprAST>> PrimaryExpr
%type <std::unique_ptr<UnaryExprAST>>   UnaryExpr
%type <std::unique_ptr<BinaryExprAST>>  BinaryExpr
%type <std::unique_ptr<LAndExprAST>>    LAndExpr
%type <std::unique_ptr<LOrExprAST>>     LOrExpr
%type <std::unique_ptr<NumberAST>>      Number
%type <std::unique_ptr<LValAST>>        LVal
%type <std::unique_ptr<BaseType>>       BasicType

/* --- indexing & dimensions --- */
%type <std::vector<std::unique_ptr<ConstExprAST>>> ArrayDims
%type <std::vector<std::unique_ptr<ExprAST>>>      Indies

/* --- for-statement helpers --- */
%type <std::unique_ptr<ExprAST>>      OptExpr
%type <std::unique_ptr<BlockItemAST>> ForInitClause
%type <std::unique_ptr<StmtAST>>      ForStepClause


/* ===================================================================
 * Grammar rules
 * =================================================================== */
%%

/* ---------- translation unit ---------- */

CompUnit
    : %empty
    | CompUnit FuncDef          { ctx.ast.AddFuncDef(std::move($2)); }
    | CompUnit Decl             { ctx.ast.AddDecl(std::move($2)); }
    ;

/* ---------- function definition ---------- */

FuncDef
    : BasicType IDENT '(' FuncFParams ')' Block {
        $$ = std::make_unique<FuncDefAST>(std::move($1), $2, std::move($4), std::move($6));
      }
    | BasicType IDENT '(' ')' Block {
        $$ = std::make_unique<FuncDefAST>(std::move($1), $2, std::move($5));
      }
    ;

BasicType
    : INT   { $$ = std::make_unique<BaseType>(BaseType::TYPE::INT);  }
    | CHAR  { $$ = std::make_unique<BaseType>(BaseType::TYPE::CHAR); }
    | VOID  { $$ = std::make_unique<BaseType>(BaseType::TYPE::VOID); }
    ;

FuncFParams
    : FuncFParam {
        $$ = std::vector<std::unique_ptr<FuncFParamAST>>();
        $$.emplace_back(std::move($1));
      }
    | FuncFParams ',' FuncFParam {
        $1.emplace_back(std::move($3));
        $$ = std::move($1);
      }
    ;

FuncFParam
    : BasicType IDENT '[' ']' ArrayDims {
        $$ = std::make_unique<FuncFParamAST>(std::move($1), $2, std::move($5));
      }
    | BasicType IDENT '[' ']' {
        $$ = std::make_unique<FuncFParamAST>(std::move($1), $2, true);
      }
    | BasicType IDENT {
        $$ = std::make_unique<FuncFParamAST>(std::move($1), $2);
      }
    ;

/* ---------- blocks ---------- */

Block
    : '{' BlockItems '}' {
        $$ = std::make_unique<BlockAST>(std::move($2));
      }
    ;

BlockItems
    : %empty {
        $$ = std::vector<std::unique_ptr<BlockItemAST>>();
      }
    | BlockItems BlockItem {
        $1.emplace_back(std::move($2));
        $$ = std::move($1);
      }
    ;

BlockItem
    : Decl { $$ = std::make_unique<BlockItemAST>(std::move($1)); }
    | Stmt { $$ = std::make_unique<BlockItemAST>(std::move($1)); }
    ;

/* ---------- statements (dangling-else resolution) ---------- */

Stmt
    : MatchedStmt   { $$ = std::move($1); }
    | UnmatchedStmt { $$ = std::move($1); }
    ;

MatchedStmt
    : LVal '=' Expr ';' {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::Assign, std::move($1), std::move($3));
      }
    | OptExpr ';' {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::Expr, std::move($1));
      }
    | Block {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::Block, std::move($1));
      }
    | IF '(' Expr ')' MatchedStmt ELSE MatchedStmt {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::If, std::move($3), std::move($5), std::move($7));
      }
    | RETURN ';' {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::Ret);
      }
    | RETURN Expr ';' {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::Ret, std::move($2));
      }
    | WHILE '(' Expr ')' MatchedStmt {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::While, std::move($3), std::move($5));
      }
    | FOR '(' ForInitClause OptExpr ';' ForStepClause ')' MatchedStmt {
        auto s = std::make_unique<StmtAST>(StmtAST::TYPE::For);
        if ($3) {
            if ($3->decl)      s->forDecl     = std::move($3->decl);
            else if ($3->stmt) s->forInitStmt = std::move($3->stmt);
        }
        s->cond        = std::move($4);
        s->forStepStmt = std::move($6);
        s->thenStmt    = std::move($8);
        $$ = std::move(s);
      }
    | BREAK ';' {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::Break);
      }
    | CONTINUE ';' {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::Continue);
      }
    ;

UnmatchedStmt
    : IF '(' Expr ')' Stmt {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::If, std::move($3), std::move($5));
      }
    | IF '(' Expr ')' MatchedStmt ELSE UnmatchedStmt {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::If, std::move($3), std::move($5), std::move($7));
      }
    | FOR '(' ForInitClause OptExpr ';' ForStepClause ')' UnmatchedStmt {
        auto s = std::make_unique<StmtAST>(StmtAST::TYPE::For);
        if ($3) {
            if ($3->decl)      s->forDecl     = std::move($3->decl);
            else if ($3->stmt) s->forInitStmt = std::move($3->stmt);
        }
        s->cond        = std::move($4);
        s->forStepStmt = std::move($6);
        s->thenStmt    = std::move($8);
        $$ = std::move(s);
      }
    | WHILE '(' Expr ')' UnmatchedStmt {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::While, std::move($3), std::move($5));
      }
    ;

/* ---------- for-statement clauses ---------- */

OptExpr
    : Expr   { $$ = std::move($1); }
    | %empty { $$ = nullptr; }
    ;

ForInitClause
    : ';' {
        $$ = nullptr;
      }
    | BasicType VarDefs ';' {
        auto decl = std::make_unique<DeclAST>(
            std::make_unique<VarDeclAST>(std::move($1), std::move($2)));
        $$ = std::make_unique<BlockItemAST>(std::move(decl));
      }
    | LVal '=' Expr ';' {
        auto stmt = std::make_unique<StmtAST>(StmtAST::TYPE::Assign, std::move($1), std::move($3));
        $$ = std::make_unique<BlockItemAST>(std::move(stmt));
      }
    | Expr ';' {
        auto stmt = std::make_unique<StmtAST>(StmtAST::TYPE::Expr, std::move($1));
        $$ = std::make_unique<BlockItemAST>(std::move(stmt));
      }
    ;

ForStepClause
    : %empty {
        $$ = nullptr;
      }
    | LVal '=' Expr {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::Assign, std::move($1), std::move($3));
      }
    | Expr {
        $$ = std::make_unique<StmtAST>(StmtAST::TYPE::Expr, std::move($1));
      }
    ;

/* ---------- expressions (precedence low → high) ---------- */

Expr
    : LOrExpr {
        $$ = std::make_unique<ExprAST>(std::move($1));
      }
    ;

LOrExpr
    : LAndExpr {
        $$ = std::make_unique<LOrExprAST>(std::move($1));
      }
    | LOrExpr OR LAndExpr {
        $$ = std::make_unique<LOrExprAST>(std::move($1), std::move($3));
      }
    ;

LAndExpr
    : BinaryExpr {
        $$ = std::make_unique<LAndExprAST>(std::move($1));
      }
    | LAndExpr AND BinaryExpr {
        $$ = std::make_unique<LAndExprAST>(std::move($1), std::move($3));
      }
    ;

BinaryExpr
    : UnaryExpr {
        $$ = std::make_unique<BinaryExprAST>(std::move($1));
      }
    | BinaryExpr '+' BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::ADD, std::move($1), std::move($3));
      }
    | BinaryExpr '-' BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::SUB, std::move($1), std::move($3));
      }
    | BinaryExpr '*' BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::MUL, std::move($1), std::move($3));
      }
    | BinaryExpr '/' BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::DIV, std::move($1), std::move($3));
      }
    | BinaryExpr '%' BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::MOD, std::move($1), std::move($3));
      }
    | BinaryExpr '<' BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::LT, std::move($1), std::move($3));
      }
    | BinaryExpr '>' BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::GT, std::move($1), std::move($3));
      }
    | BinaryExpr LE BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::LE, std::move($1), std::move($3));
      }
    | BinaryExpr GE BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::GE, std::move($1), std::move($3));
      }
    | BinaryExpr EQ BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::EQ, std::move($1), std::move($3));
      }
    | BinaryExpr NE BinaryExpr {
        $$ = std::make_unique<BinaryExprAST>(BinaryExprAST::Op::NE, std::move($1), std::move($3));
      }
    ;

UnaryExpr
    : PrimaryExpr {
        $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::TYPE::Primary, std::move($1));
      }
    | '+' UnaryExpr {
        $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::TYPE::Unary, UnaryExprAST::OP::PLUS, std::move($2));
      }
    | '-' UnaryExpr {
        $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::TYPE::Unary, UnaryExprAST::OP::MINUS, std::move($2));
      }
    | '!' UnaryExpr {
        $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::TYPE::Unary, UnaryExprAST::OP::NOT, std::move($2));
      }
    | IDENT '(' ')' {
        $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::TYPE::Call, $1);
      }
    | IDENT '(' FuncRParams ')' {
        $$ = std::make_unique<UnaryExprAST>(UnaryExprAST::TYPE::Call, $1, std::move($3));
      }
    ;

PrimaryExpr
    : '(' Expr ')' {
        $$ = std::make_unique<PrimaryExprAST>(PrimaryExprAST::TYPE::Expr, std::move($2));
      }
    | LVal {
        $$ = std::make_unique<PrimaryExprAST>(PrimaryExprAST::TYPE::LVal, std::move($1));
      }
    | Number {
        $$ = std::make_unique<PrimaryExprAST>(PrimaryExprAST::TYPE::Number, std::move($1));
      }
    | STR_CONST {
        $$ = std::make_unique<PrimaryExprAST>($1);
      }
    ;

Number
    : INT_CONST {
        $$ = std::make_unique<NumberAST>(std::move($1));
      }
    ;

FuncRParams
    : Expr {
        $$ = std::vector<std::unique_ptr<ExprAST>>();
        $$.emplace_back(std::move($1));
      }
    | FuncRParams ',' Expr {
        $1.emplace_back(std::move($3));
        $$ = std::move($1);
      }
    ;

/* ---------- declarations ---------- */

Decl
    : ConstDecl { $$ = std::make_unique<DeclAST>(std::move($1)); }
    | VarDecl   { $$ = std::make_unique<DeclAST>(std::move($1)); }
    ;

ConstDecl
    : CONST BasicType ConstDefs ';' {
        $$ = std::make_unique<ConstDeclAST>(std::move($2), std::move($3));
      }
    ;

VarDecl
    : BasicType VarDefs ';' {
        $$ = std::make_unique<VarDeclAST>(std::move($1), std::move($2));
      }
    ;

ConstDefs
    : ConstDef {
        $$ = std::vector<std::unique_ptr<ConstDefAST>>();
        $$.emplace_back(std::move($1));
      }
    | ConstDefs ',' ConstDef {
        $1.emplace_back(std::move($3));
        $$ = std::move($1);
      }
    ;

VarDefs
    : VarDef {
        $$ = std::vector<std::unique_ptr<VarDefAST>>();
        $$.emplace_back(std::move($1));
      }
    | VarDefs ',' VarDef {
        $1.emplace_back(std::move($3));
        $$ = std::move($1);
      }
    ;

ConstDef
    : IDENT '=' ConstInitVal {
        $$ = std::make_unique<ConstDefAST>($1, std::move($3));
      }
    | IDENT ArrayDims '=' ConstInitVal {
        $$ = std::make_unique<ConstDefAST>($1, std::move($2), std::move($4));
      }
    ;

VarDef
    : IDENT {
        $$ = std::make_unique<VarDefAST>($1);
      }
    | IDENT ArrayDims {
        $$ = std::make_unique<VarDefAST>($1, std::move($2));
      }
    | IDENT '=' InitVal {
        $$ = std::make_unique<VarDefAST>($1, std::move($3));
      }
    | IDENT ArrayDims '=' InitVal {
        $$ = std::make_unique<VarDefAST>($1, std::move($2), std::move($4));
      }
    ;

/* ---------- initializers ---------- */

ConstInitVal
    : ConstExpr {
        $$ = std::make_unique<ConstInitValAST>(std::move($1));
      }
    | '{' '}' {
        $$ = std::make_unique<ConstInitValAST>();
      }
    | '{' ConstInitVals '}' {
        $$ = std::make_unique<ConstInitValAST>(std::move($2));
      }
    ;

ConstInitVals
    : ConstInitVal {
        $$ = std::vector<std::unique_ptr<ConstInitValAST>>();
        $$.emplace_back(std::move($1));
      }
    | ConstInitVals ',' ConstInitVal {
        $1.emplace_back(std::move($3));
        $$ = std::move($1);
      }
    ;

InitVal
    : Expr {
        $$ = std::make_unique<InitValAST>(std::move($1));
      }
    | '{' '}' {
        $$ = std::make_unique<InitValAST>();
      }
    | '{' InitVals '}' {
        $$ = std::make_unique<InitValAST>(std::move($2));
      }
    ;

InitVals
    : InitVal {
        $$ = std::vector<std::unique_ptr<InitValAST>>();
        $$.emplace_back(std::move($1));
      }
    | InitVals ',' InitVal {
        $1.emplace_back(std::move($3));
        $$ = std::move($1);
      }
    ;

/* ---------- l-values & indexing ---------- */

LVal
    : IDENT {
        $$ = std::make_unique<LValAST>($1);
      }
    | IDENT Indies {
        $$ = std::make_unique<LValAST>($1, std::move($2));
      }
    ;

ArrayDims
    : '[' ConstExpr ']' {
        $$ = std::vector<std::unique_ptr<ConstExprAST>>();
        $$.emplace_back(std::move($2));
      }
    | ArrayDims '[' ConstExpr ']' {
        $1.emplace_back(std::move($3));
        $$ = std::move($1);
      }
    ;

Indies
    : '[' Expr ']' {
        $$ = std::vector<std::unique_ptr<ExprAST>>();
        $$.emplace_back(std::move($2));
      }
    | Indies '[' Expr ']' {
        $1.emplace_back(std::move($3));
        $$ = std::move($1);
      }
    ;

ConstExpr
    : Expr {
        $$ = std::make_unique<ConstExprAST>(std::move($1));
      }
    ;

%%

/* ===================================================================
 * Error handler
 * =================================================================== */

void yy::Parser::error(const yy::location& l, const std::string& m)
{
    std::cerr << l << ": " << m << std::endl;
}