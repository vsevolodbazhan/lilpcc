%option nounput
%option noinput

%{
#define YYSTYPE char*
#define SAVE_TOKEN yylval = strdup(yytext)

#include "string.h"

#include "parser.tab.h"
%}

%%
[ \t\n]+ { /* Ignore whitespaces. */}

"{" { return OP_BRACE; }
"}" { return CL_BRACE; }
"(" { return OP_PAREN; }
")" { return CL_PAREN; }
"~" { return XOR; }
"=" { return ASSIGN; }
";" { return SEMICOLON; }
"," { return COMMA; }

"+"  { return PLUS; }
"-"  { return MINUS; }
"*"  { return MULT; }
"/"  { return DIV; }
"<"  { return LESS; }
">"  { return GREATER; }
"<=" { return LESS_OR_EQUAL; }
">=" { return GREATER_OR_EQUAL; }
"==" { return EQUAL; }

[0-9]+   { SAVE_TOKEN; return NUMBER; }
"if"     { return IF; }
"while"  { return WHILE; }
"return" { return RETURN; }
"int"    { return INT; }

[a-z_][a-zA-Z0-9_]* { SAVE_TOKEN; return ID; }
%%
