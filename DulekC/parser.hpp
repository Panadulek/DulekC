
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* "%code requires" blocks.  */

/* Line 1676 of yacc.c  */
#line 24 "parser.ypp"


    #include "Type.h"
    #include "Variable.h"
    #include "DuObject.h"
    #include <memory>
    #include <string>
    #include <cstdint>
    #include <vector>
    #include "Statement.h"
    // Inne wymagane nag³ówki



/* Line 1676 of yacc.c  */
#line 55 "parser.hpp"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ARROW = 258,
     LBRACE = 259,
     RBRACE = 260,
     COMMA = 261,
     SEMICOLON = 262,
     LBUCKLE = 263,
     RBUCKLE = 264,
     INIT_TYPE = 265,
     FUNCTION_KEYWORD = 266,
     ASSIGMENT = 267,
     I8 = 268,
     U8 = 269,
     I16 = 270,
     U16 = 271,
     I32 = 272,
     U32 = 273,
     I64 = 274,
     U64 = 275,
     IDENTIFIER = 276,
     NUMBER = 277
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 36 "parser.ypp"

    char* str;  
    uint64_t num; 
    Variable* pvariable;
    Type* ptype;
    NumericValue* pnumvalue;
    ObjectInByte bytetype;
    Identifier* pidentifier;
    Statement* pstatement;



/* Line 1676 of yacc.c  */
#line 107 "parser.hpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


