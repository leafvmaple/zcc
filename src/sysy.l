%option noyywrap
%option nounput
%option noinput
%option reentrant

%{

#include <cstdlib>
#include <string>

#define YY_DECL yy::Parser::symbol_type yylex(yyscan_t yyscanner, yy::location& loc)

#define YY_USER_ACTION loc.columns(yyleng);
#define yyterminate() return yy::Parser::make_END(loc)

#include "sysy.tab.hpp"

using namespace std;

%}

WhiteSpace    [ \t\n\r]*
LineComment   "//".*

Identifier    [a-zA-Z_][a-zA-Z0-9_]*

Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

%%

{WhiteSpace}    { }
{LineComment}   { }

"int"           { return yy::Parser::make_INT(loc); }
"return"        { return yy::Parser::make_RETURN(loc); }

"&&"            { return yy::Parser::make_AND(loc); }
"||"            { return yy::Parser::make_OR(loc); }
"=="            { return yy::Parser::make_EQ(loc); }
"!="            { return yy::Parser::make_NE(loc); }
"<="            { return yy::Parser::make_LE(loc); }
">="            { return yy::Parser::make_GE(loc); }

{Identifier}    { return yy::Parser::make_IDENT(std::string(yytext), loc); }

{Decimal}       { return yy::Parser::make_INT_CONST(strtol(yytext, nullptr, 0), loc); }
{Octal}         { return yy::Parser::make_INT_CONST(strtol(yytext, nullptr, 0), loc); }
{Hexadecimal}   { return yy::Parser::make_INT_CONST(strtol(yytext, nullptr, 0), loc); }

.               { return yy::Parser::symbol_type(yy::Parser::token_type(yytext[0]), loc); }

%%