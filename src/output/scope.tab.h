/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_SRC_OUTPUT_SCOPE_TAB_H_INCLUDED
# define YY_YY_SRC_OUTPUT_SCOPE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 1 "src/scope.y" /* yacc.c:1921  */

	#include "../bytecode.h"

#line 52 "src/output/scope.tab.h" /* yacc.c:1921  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    EOL = 258,
    IDENTIFIER = 259,
    S_EXTERN = 260,
    S_IF = 261,
    S_ELSE = 262,
    S_WHILE = 263,
    S_RETURN = 264,
    S_BREAK = 265,
    S_FUNC = 266,
    S_SWAP = 267,
    S_FOR = 268,
    E_NEW = 269,
    T_AUTO = 270,
    T_VOID = 271,
    T_STR = 272,
    T_INT = 273,
    T_BOOL = 274,
    T_FLOAT = 275,
    T_FUNC = 276,
    L_NUMBER = 277,
    L_BOOL = 278,
    L_STRING = 279,
    L_FLOAT = 280,
    T_EQ = 281,
    T_NE = 282,
    T_AND = 283,
    T_OR = 284,
    T_GTE = 285,
    T_LTE = 286,
    O_UMINUS = 287,
    O_CAST = 288
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 29 "src/scope.y" /* yacc.c:1921  */

	char* v_str;
	int v_int;
	float v_float;

#line 104 "src/output/scope.tab.h" /* yacc.c:1921  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_OUTPUT_SCOPE_TAB_H_INCLUDED  */
