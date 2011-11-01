%{
/* xclnm_gram.y
 *
 * Copyright (c) 2008-2010 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 (GPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* DAR */

%}

/* 
 * This is the yacc grammar file for a simply MIPS-like assembler parser
 */

%{
/* DAR */
%}

%token	KERNEL
%token	GLOBAL

%token	VECN

/* types */
%token 	TYPE_OPAQUE
%token 	TYPE_VOID
%token 	TYPE_INT8 TYPE_INT16 TYPE_INT32 TYPE_INT64
%token 	TYPE_UINT8 TYPE_UINT16 TYPE_UINT32 TYPE_UINT64
%token 	TYPE_FLOAT
%token 	TYPE_DOUBLE
%token	TYPE_IMAGE2D

/* qualifiers */

/* generic tokens */
%token	ICONST
%token	TYPE
%token	SYMBOL
%token	BODY OPEN_BODY CLOSE_BODY
%token	STRING
%token	VARG
%token	TYPEDEF_OPAQUE
%token	SKIP

%{

#include "xclnm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void yyerror(const char*);

%}

%union {
	int ival;
	node_t* node;
}

%type <ival> KERNEL
%type <ival> GLOBAL
%type <ival> VECN
%type <ival> TYPE_OPAQUE
%type <ival> TYPE_VOID
%type <ival> TYPE_INT8
%type <ival> TYPE_INT16
%type <ival> TYPE_INT32
%type <ival> TYPE_INT64
%type <ival> TYPE_UINT8
%type <ival> TYPE_UINT16
%type <ival> TYPE_UINT32
%type <ival> TYPE_UINT64
%type <ival> TYPE_FLOAT
%type <ival> TYPE_DOUBLE
%type <ival> TYPE_IMAGE2D
%type <ival> ICONST
%type <ival> TYPE
%type <ival> SYMBOL
%type <ival> BODY OPEN_BODY CLOSE_BODY
%type <ival> STRING
%type <node> func_dec func_def
%type <node> args arg type
%type <ival> datatype
%type <ival> ptrc
%type <ival> vecn
%type <ival> body

%type <ival> input line
%type <ival> VARG
%type <ival> TYPEDEF_OPAQUE
%type <ival> SKIP



%% /* grammar rules */

/* general constructions */

input:	/* empty */ { $$=0; }
			| input line ;

line: 	'\n' { $$=0; }
			| func_dec { __rlb(); cur_nptr = node_insert(cur_nptr,$1); }
			| func_def { __rlb(); cur_nptr = node_insert(cur_nptr,$1); }
			| SKIP {__rlb(); };



func_dec:	type SYMBOL '(' ')' ';'
					{$$=node_create_func_dec($1,$2,0);};
				| type SYMBOL '(' args ')' ';'
					{$$=node_create_func_dec($1,$2,$4);};

func_def: 	type SYMBOL '(' ')' body
					{ $$=node_create_func_def($1,$2,0);};
				| type SYMBOL '(' args ')' body
					{ $$=node_create_func_def($1,$2,$4);};
				| KERNEL type SYMBOL '(' ')' body
					{ $$=node_create_func_def($2,$3,0);};
				| KERNEL type SYMBOL '(' args ')' body
					{ $$=node_create_func_def($2,$3,$5);};

args:		args ',' arg { $$ = node_insert_tail($1,$3); } 
			| arg ;

arg:	type SYMBOL { $$ = node_create_arg($1,$2); }
		| type { $$ = node_create_arg($1,0); }
		| VARG { $$ = node_create_arg($1,0); }

type:	datatype { $$ = node_create_type(1,1,$1,0,0); } ;
		| type ptrc { $$=node_set_ptrc($1,$2); }
		| '<' vecn datatype '>' { $$=node_create_type(1,$2,$3,0,0); } 
		| '[' vecn type ']' { $$=node_set_arrn($3,$2); } ;

datatype:	TYPE_VOID 
				| TYPE_INT8 | TYPE_INT16 | TYPE_INT32 | TYPE_INT64
				| TYPE_UINT8 | TYPE_UINT16 | TYPE_UINT32 | TYPE_UINT64
				| TYPE_FLOAT | TYPE_DOUBLE 
				| TYPE_OPAQUE ;

ptrc:		ptrc '*' { $$ = $1+1; }
			| '*' { $$ = 1; }

body:		BODY { $$=$1; };

vecn:		VECN { $$=$1; };

%%

void
yyerror( const char* s)
{
	printf("ERROR: %s\n",s);
}


