/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TARGET = 258,
     DECLARE = 259,
     DEFINE = 260,
     VECN = 261,
     TYPE_OPAQUE = 262,
     TYPE_VOID = 263,
     TYPE_INT8 = 264,
     TYPE_INT16 = 265,
     TYPE_INT32 = 266,
     TYPE_INT64 = 267,
     TYPE_FLOAT = 268,
     TYPE_DOUBLE = 269,
     ADDRSPACE = 270,
     ICONST = 271,
     TYPE = 272,
     GLOBAL_SYMBOL = 273,
     GLOBAL_SYMBOL_DOT = 274,
     LOCAL_SYMBOL = 275,
     GENERIC_SYMBOL = 276,
     BODY = 277,
     OPEN_BODY = 278,
     CLOSE_BODY = 279,
     COMMENT = 280,
     STRING = 281,
     VARG = 282,
     TYPEDEF_OPAQUE = 283,
     SKIP = 284
   };
#endif
/* Tokens.  */
#define TARGET 258
#define DECLARE 259
#define DEFINE 260
#define VECN 261
#define TYPE_OPAQUE 262
#define TYPE_VOID 263
#define TYPE_INT8 264
#define TYPE_INT16 265
#define TYPE_INT32 266
#define TYPE_INT64 267
#define TYPE_FLOAT 268
#define TYPE_DOUBLE 269
#define ADDRSPACE 270
#define ICONST 271
#define TYPE 272
#define GLOBAL_SYMBOL 273
#define GLOBAL_SYMBOL_DOT 274
#define LOCAL_SYMBOL 275
#define GENERIC_SYMBOL 276
#define BODY 277
#define OPEN_BODY 278
#define CLOSE_BODY 279
#define COMMENT 280
#define STRING 281
#define VARG 282
#define TYPEDEF_OPAQUE 283
#define SKIP 284




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 73 "xclnm_gram.y"
{
	int ival;
	node_t* node;
}
/* Line 1529 of yacc.c.  */
#line 112 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE __xclnm_lval;

