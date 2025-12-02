%option noyywrap nounput batch
%x COMMENT

%{
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yyerror(const char *s);
%}

%%
"meow meow"     { return T_INT; }
"meow meow meow" { return T_FLOAT; }
"meowmeow"      { return T_BOOL; }
"meowww"        { return T_STRING; }
"meow"          { return T_DECLARACION; }

"meoow"         { return T_IF; }
"meoow meow"    { return T_ELSE; }
"meowl"         { return T_WHILE; }
"meowrr"        { return T_FOR; }
"mew"           { return T_FUNCTION; }
"mewmew"        { return T_RETURN; }

"meowt"         { yylval.type = TYPE_BOOL; return T_TRUE; }
"meowf"         { yylval.type = TYPE_BOOL; return T_FALSE; }

"miau_pixel"        { return T_MIAU_PIXEL; }
"miau_key"          { return T_MIAU_KEY; }
"miau_input"        { return T_MIAU_INPUT; }
"miau_print"        { return T_MIAU_PRINT; }

[0-9]+\.[0-9]+([eE][-+]?[0-9]+)? { yylval.type = TYPE_FLOAT; yylval.fval = atof(yytext); return T_LITERAL_FLOAT; }
[0-9]+      { yylval.type = TYPE_INT; yylval.ival = atoi(yytext); return T_LITERAL_INT; }
\"(\\.|[^\"])*\" { yylval.type = TYPE_STRING; yylval.sval = strdup(yytext); return T_LITERAL_STRING; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.sval = strdup(yytext); return T_ID; }

"/*"            { BEGIN(COMMENT); }
<COMMENT>[^*\n]*    { ; }
<COMMENT>"*"+[^*/\n]* { ; }
<COMMENT>"*"+"/"        { BEGIN(INITIAL); }
[ \t\n]         ;
"//"[^\n]*      { ; }

"="             { return T_ASSIGN; }
";"             { return T_SEMICOLON; }
"+"             { return T_PLUS; }
"-"             { return T_MINUS; }
"*"             { return T_MULT; }
"/"             { return T_DIV; }
"("             { return T_LPAREN; }
")"             { return T_RPAREN; }
"["             { return T_LBRACKET; }
"]"             { return T_RBRACKET; }
"."             { return T_DOT; }
","             { return T_COMMA; }
.               { fprintf(stderr, "Error léxico: Carácter no reconocido: %s\n", yytext); }

%%