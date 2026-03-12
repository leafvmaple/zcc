%option noyywrap
%option nounput
%option noinput
%option reentrant

%{

#include <cstdlib>
#include <string>

#include "sysy.tab.hpp"

/* yylex signature for reentrant scanner + bison C++ skeleton */
#define YY_DECL yy::Parser::symbol_type yylex(yyscan_t yyscanner, yy::location& loc)

/* Advance location columns for every matched token */
#define YY_USER_ACTION loc.columns(yyleng);

/* Map EOF to bison's END token */
#define yyterminate() return yy::Parser::make_END(loc)

/* ---------- helper functions ---------- */

static int parse_char_escape(char c) {
    switch (c) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '\\': return '\\';
        case '\'': return '\'';
        case '\"': return '\"';
        case '0':  return '\0';
        case 'a':  return '\a';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'v':  return '\v';
        default:   return c;
    }
}

static std::string parse_string_literal(const char* text, int len) {
    std::string result;
    for (int i = 1; i < len - 1; ++i) {
        if (text[i] == '\\' && i + 1 < len - 1) {
            result += static_cast<char>(parse_char_escape(text[i + 1]));
            ++i;
        } else {
            result += text[i];
        }
    }
    return result;
}

%}

/* ---------- named patterns ---------- */

WS            [ \t\r]+
LineComment   "//".*

Identifier    [a-zA-Z_][a-zA-Z0-9_]*

Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

CharLiteral   '([^\\']|\\[nrt\\\'\"0abfv])'
StringLiteral \"([^\\"]|\\.)*\"

%x COMMENT

%%
 /* ---------- block comments ---------- */

"/*"              { BEGIN(COMMENT); }
<COMMENT>"*/"     { BEGIN(INITIAL); }
<COMMENT>\n       { loc.lines(1); loc.step(); }
<COMMENT>.        { /* eat comment body */ }

 /* ---------- whitespace & line comments ---------- */

\n                { loc.lines(1); loc.step(); }
{WS}             { loc.step(); }
{LineComment}    { /* skip to end-of-line */ }

 /* ---------- keywords ---------- */

"int"             { return yy::Parser::make_INT(loc); }
"char"            { return yy::Parser::make_CHAR(loc); }
"void"            { return yy::Parser::make_VOID(loc); }
"const"           { return yy::Parser::make_CONST(loc); }
"return"          { return yy::Parser::make_RETURN(loc); }
"if"              { return yy::Parser::make_IF(loc); }
"else"            { return yy::Parser::make_ELSE(loc); }
"while"           { return yy::Parser::make_WHILE(loc); }
"for"             { return yy::Parser::make_FOR(loc); }
"break"           { return yy::Parser::make_BREAK(loc); }
"continue"        { return yy::Parser::make_CONTINUE(loc); }

 /* ---------- multi-char operators ---------- */

"&&"              { return yy::Parser::make_AND(loc); }
"||"              { return yy::Parser::make_OR(loc); }
"=="              { return yy::Parser::make_EQ(loc); }
"!="              { return yy::Parser::make_NE(loc); }
"<="              { return yy::Parser::make_LE(loc); }
">="              { return yy::Parser::make_GE(loc); }

 /* ---------- identifiers ---------- */

{Identifier}      { return yy::Parser::make_IDENT(std::string(yytext), loc); }

 /* ---------- literals ---------- */

{CharLiteral}     {
    int val = (yytext[1] == '\\') ? parse_char_escape(yytext[2])
                                  : static_cast<unsigned char>(yytext[1]);
    return yy::Parser::make_INT_CONST(val, loc);
}

{StringLiteral}   {
    return yy::Parser::make_STR_CONST(parse_string_literal(yytext, yyleng), loc);
}

{Decimal}         { return yy::Parser::make_INT_CONST(strtol(yytext, nullptr, 0), loc); }
{Octal}           { return yy::Parser::make_INT_CONST(strtol(yytext, nullptr, 0), loc); }
{Hexadecimal}     { return yy::Parser::make_INT_CONST(strtol(yytext, nullptr, 0), loc); }

 /* ---------- single-char tokens (fallback) ---------- */

.                 { return yy::Parser::symbol_type(yy::Parser::token_type(yytext[0]), loc); }

%%