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
     KERNEL = 258,
     GLOBAL = 259,
     VECN = 260,
     TYPE_OPAQUE = 261,
     TYPE_VOID = 262,
     TYPE_INT8 = 263,
     TYPE_INT16 = 264,
     TYPE_INT32 = 265,
     TYPE_INT64 = 266,
     TYPE_UINT8 = 267,
     TYPE_UINT16 = 268,
     TYPE_UINT32 = 269,
     TYPE_UINT64 = 270,
     TYPE_FLOAT = 271,
     TYPE_DOUBLE = 272,
     TYPE_IMAGE2D = 273,
     ICONST = 274,
     TYPE = 275,
     SYMBOL = 276,
     BODY = 277,
     OPEN_BODY = 278,
     CLOSE_BODY = 279,
     STRING = 280,
     VARG = 281,
     TYPEDEF_OPAQUE = 282,
     SKIP = 283
   };
#endif
/* Tokens.  */
#define KERNEL 258
#define GLOBAL 259
#define VECN 260
#define TYPE_OPAQUE 261
#define TYPE_VOID 262
#define TYPE_INT8 263
#define TYPE_INT16 264
#define TYPE_INT32 265
#define TYPE_INT64 266
#define TYPE_UINT8 267
#define TYPE_UINT16 268
#define TYPE_UINT32 269
#define TYPE_UINT64 270
#define TYPE_FLOAT 271
#define TYPE_DOUBLE 272
#define TYPE_IMAGE2D 273
#define ICONST 274
#define TYPE 275
#define SYMBOL 276
#define BODY 277
#define OPEN_BODY 278
#define CLOSE_BODY 279
#define STRING 280
#define VARG 281
#define TYPEDEF_OPAQUE 282
#define SKIP 283




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 71 "xclnm_gram.y"
{
	int ival;
	node_t* node;
}
/* Line 1529 of yacc.c.  */
#line 110 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE __xclnm_lval;

