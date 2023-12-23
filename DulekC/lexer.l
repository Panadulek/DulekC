%option noyywrap

%{
#include "parser.hpp" // Bison wygeneruje ten plik
#include <iostream>
#pragma warning(disable : 4996)
#define DISPLAY(X) std::cout << X << std::endl
#define MY_DEBUG 1
#include<string>
%}



%%

"i8"					{ DISPLAY(yytext); yylval.str = strdup(yytext); return I8; }
"u8"					{ DISPLAY(yytext); yylval.str = strdup(yytext); return U8; }
"i16"					{ DISPLAY(yytext); yylval.str = strdup(yytext); return I16; }
"u16"					{ DISPLAY(yytext); yylval.str = strdup(yytext); return U16; }
"i32"					{ DISPLAY(yytext); yylval.str = strdup(yytext); return I32; }
"u32"					{ DISPLAY(yytext); yylval.str = strdup(yytext); return U32; }
"i64"					{ DISPLAY(yytext); yylval.str = strdup(yytext); return I64; }
"u64"					{ DISPLAY(yytext); yylval.str = strdup(yytext); return U64; }


[0-9]+                  { 
yylval.num = std::stoull(yytext) ; 
DISPLAY("NUMBER"); 
return NUMBER; 

}
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.str = strdup(yytext); DISPLAY("IDENTIFIER"); return IDENTIFIER; }
"->"                    { DISPLAY("ARROW");return ARROW; }
"{"                     { return LBUCKLE; }
"}"                     { return RBUCKLE; }
","                     { return COMMA; }
";"                     { DISPLAY(";");return SEMICOLON; }
":"						{return INIT_TYPE;}
[ \t\n]+                { /* Ignoruj bia³e znaki */ }
"("						{return LBRACE;}
")"						{return RBRACE;}
.                       { /* Ignoruj inne znaki */ }



%%
