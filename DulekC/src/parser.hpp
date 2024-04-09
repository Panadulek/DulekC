
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
#line 43 "E:/LocalRepo/Compilers/AssemblerCompiler/DulekC/DulekC/src/parser.ypp"


    #include "Type.h"
    #include "Variable.h"
    #include "DuObject.h"
    #include <memory>
    #include <string>
    #include <cstdint>
    #include <vector>
    #include "Statement.h"
    #include "Expression.h"
    // Inne wymagane nag³ówki



/* Line 1676 of yacc.c  */
#line 56 "E:/LocalRepo/Compilers/AssemblerCompiler/DulekC/DulekC/src/parser.hpp"

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
     ASSIGMENT = 266,
     PLUS = 267,
     MINUS = 268,
     MULTIPLICATION = 269,
     DIV = 270,
     COMMENT = 271,
     FUNCTION_KEYWORD = 272,
     RETURN_KEYWORD = 273,
     IF_KEYWORD = 274,
     ELSE_KEYWORD = 275,
     WHILE_KEYWORD = 276,
     PTR = 277,
     NEW = 278,
     DELETE = 279,
     LT = 280,
     GT = 281,
     EQ = 282,
     SYS_DISPLAY = 283,
     ALLOCATOR = 284,
     DEALLOCATOR = 285,
     REALLOCATOR = 286,
     I8 = 287,
     U8 = 288,
     I16 = 289,
     U16 = 290,
     I32 = 291,
     U32 = 292,
     I64 = 293,
     U64 = 294,
     IDENTIFIER = 295,
     NUMBER = 296
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 56 "E:/LocalRepo/Compilers/AssemblerCompiler/DulekC/DulekC/src/parser.ypp"

    char* str;  
    uint64_t num; 
    Variable* pvariable;
    Type* ptype;
    NumericValue* pnumvalue;
    ObjectInByte bytetype;
    Identifier* pidentifier;
    Statement* pstatement;
    Value* pval;
    Expression* pexpr;
    SystemFunctions::SysFunctionID sysfunid;



/* Line 1676 of yacc.c  */
#line 130 "E:/LocalRepo/Compilers/AssemblerCompiler/DulekC/DulekC/src/parser.hpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


