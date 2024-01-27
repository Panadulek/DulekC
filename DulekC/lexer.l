%option noyywrap

%{
#include "parser.hpp" // Bison wygeneruje ten plik
#include <iostream>
#include "Type.h"
#pragma warning(disable : 4996)

#define YYDEBUGYY 0
#if YYDEBUGYY == 1
#define DISPLAY(X) std::cout << X << std::endl
#else
#define DISPLAY(X) ;
#endif

#include<string>
%}



%%
"fnc"					{return FUNCTION_KEYWORD;}
"if"                    {return IF_KEYWORD;}
"return"				{ return RETURN_KEYWORD;}
"i8"					{ yylval.bytetype = ObjectInByte::BYTE; return I8; }
"u8"					{ yylval.bytetype = ObjectInByte::BYTE; return U8; }
"i16"					{ yylval.bytetype = ObjectInByte::WORD; return I16; }
"u16"					{ yylval.bytetype = ObjectInByte::WORD; return U16; }
"i32"					{ yylval.bytetype = ObjectInByte::DWORD; return I32; }
"u32"					{ yylval.bytetype = ObjectInByte::DWORD; return U32; }
"i64"					{ yylval.bytetype = ObjectInByte::QWORD; return I64; }
"u64"					{ yylval.bytetype = ObjectInByte::QWORD; return U64; }
"$display"				{return SYS_DISPLAY;}

-?[0-9]+ {
    yylval.num = std::stoull(yytext); 
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
"="						{return ASSIGMENT;}
"+"						{return PLUS;}
"-"						{return MINUS;}
"*"						{return MULTIPLICATION;}
"/"						{return DIV;}
.                       { /* Ignoruj inne znaki */ }



%%
