%{
/* xclnm_gram.y
 *
 * Copyright (c) 2008-2012 Brown Deer Technology, LLC.  All Rights Reserved.
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
%token	ADDRSPACE_GLOBAL ADDRSPACE_LOCAL

%token	VECN

/* types */
%token 	TYPE_OPAQUE

%token 	TYPE_VOID
%token 	TYPE_INT8
%token 	TYPE_INT16
%token 	TYPE_INT32
%token 	TYPE_INT64
%token 	TYPE_UINT8
%token 	TYPE_UINT16
%token 	TYPE_UINT32
%token 	TYPE_UINT64
%token 	TYPE_FLOAT 
%token	TYPE_DOUBLE

%token 	TYPE_INT8_VEC2
%token 	TYPE_INT16_VEC2
%token 	TYPE_INT32_VEC2
%token 	TYPE_INT64_VEC2
%token 	TYPE_UINT8_VEC2
%token 	TYPE_UINT16_VEC2
%token 	TYPE_UINT32_VEC2
%token 	TYPE_UINT64_VEC2
%token 	TYPE_FLOAT_VEC2
%token	TYPE_DOUBLE_VEC2

%token 	TYPE_INT8_VEC4
%token 	TYPE_INT16_VEC4
%token 	TYPE_INT32_VEC4
%token 	TYPE_INT64_VEC4
%token 	TYPE_UINT8_VEC4
%token 	TYPE_UINT16_VEC4
%token 	TYPE_UINT32_VEC4
%token 	TYPE_UINT64_VEC4
%token 	TYPE_FLOAT_VEC4
%token	TYPE_DOUBLE_VEC4

%token 	TYPE_INT8_VEC8
%token 	TYPE_INT16_VEC8
%token 	TYPE_INT32_VEC8
%token 	TYPE_INT64_VEC8
%token 	TYPE_UINT8_VEC8
%token 	TYPE_UINT16_VEC8
%token 	TYPE_UINT32_VEC8
%token 	TYPE_UINT64_VEC8
%token 	TYPE_FLOAT_VEC8
%token	TYPE_DOUBLE_VEC8

%token 	TYPE_INT8_VEC16
%token 	TYPE_INT16_VEC16
%token 	TYPE_INT32_VEC16
%token 	TYPE_INT64_VEC16
%token 	TYPE_UINT8_VEC16
%token 	TYPE_UINT16_VEC16
%token 	TYPE_UINT32_VEC16
%token 	TYPE_UINT64_VEC16
%token 	TYPE_FLOAT_VEC16
%token	TYPE_DOUBLE_VEC16

/* qualifiers */

/* generic tokens */
%token	ICONST
%token	TYPE
%token	SYMBOL
%token	BODY OPEN_BODY CLOSE_BODY
%token	STRING
%token	VARG
%token	TYPEDEF
%token	STRUCT
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
%type <ival> ADDRSPACE_GLOBAL ADDRSPACE_LOCAL
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

%type <ival> TYPE_INT8_VEC2
%type <ival> TYPE_INT16_VEC2
%type <ival> TYPE_INT32_VEC2
%type <ival> TYPE_INT64_VEC2
%type <ival> TYPE_UINT8_VEC2
%type <ival> TYPE_UINT16_VEC2
%type <ival> TYPE_UINT32_VEC2
%type <ival> TYPE_UINT64_VEC2
%type <ival> TYPE_FLOAT_VEC2
%type <ival> TYPE_DOUBLE_VEC2

%type <ival> TYPE_INT8_VEC4
%type <ival> TYPE_INT16_VEC4
%type <ival> TYPE_INT32_VEC4
%type <ival> TYPE_INT64_VEC4
%type <ival> TYPE_UINT8_VEC4
%type <ival> TYPE_UINT16_VEC4
%type <ival> TYPE_UINT32_VEC4
%type <ival> TYPE_UINT64_VEC4
%type <ival> TYPE_FLOAT_VEC4
%type <ival> TYPE_DOUBLE_VEC4

%type <ival> TYPE_INT8_VEC8
%type <ival> TYPE_INT16_VEC8
%type <ival> TYPE_INT32_VEC8
%type <ival> TYPE_INT64_VEC8
%type <ival> TYPE_UINT8_VEC8
%type <ival> TYPE_UINT16_VEC8
%type <ival> TYPE_UINT32_VEC8
%type <ival> TYPE_UINT64_VEC8
%type <ival> TYPE_FLOAT_VEC8
%type <ival> TYPE_DOUBLE_VEC8

%type <ival> TYPE_INT8_VEC16
%type <ival> TYPE_INT16_VEC16
%type <ival> TYPE_INT32_VEC16
%type <ival> TYPE_INT64_VEC16
%type <ival> TYPE_UINT8_VEC16
%type <ival> TYPE_UINT16_VEC16
%type <ival> TYPE_UINT32_VEC16
%type <ival> TYPE_UINT64_VEC16
%type <ival> TYPE_FLOAT_VEC16
%type <ival> TYPE_DOUBLE_VEC16

%type <ival> ICONST
%type <ival> TYPE
%type <ival> SYMBOL
%type <ival> BODY OPEN_BODY CLOSE_BODY
%type <ival> STRING
%type <node> func_dec func_def
%type <node> args arg type
%type <ival> ptrc
%type <ival> array
%type <ival> body
%type <ival> typedef
%type <ival> struct

%type <ival> input line
%type <ival> VARG
%type <ival> TYPEDEF
%type <ival> STRUCT
%type <ival> SKIP



%% /* grammar rules */

/* general constructions */

input:	/* empty */ { $$=0; }
			| input line ;

line: 	'\n' { $$=0; }
			| func_dec { __rlb(); cur_nptr = node_insert(cur_nptr,$1); }
			| func_def { __rlb(); cur_nptr = node_insert(cur_nptr,$1); }
			| typedef { __rlb(); }
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

arg:	type SYMBOL array { $$ = node_create_arg(node_set_arrn($1,$3),$2); }
		| type SYMBOL { $$ = node_create_arg($1,$2); }
		| type { $$ = node_create_arg($1,0); }
		| VARG { $$ = node_create_arg($1,0); };

type:	TYPE_VOID { $$ = node_create_type(1,1,TYPEID_VOID,0,0); } 
		| TYPE_INT8 { $$ = node_create_type(1,1,TYPEID_CHAR,0,0); } 
		| TYPE_INT16 { $$ = node_create_type(1,1,TYPEID_SHORT,0,0); } 
		| TYPE_INT32 { $$ = node_create_type(1,1,TYPEID_INT,0,0); } 
		| TYPE_INT64 { $$ = node_create_type(1,1,TYPEID_LONG,0,0); } 
		| TYPE_UINT8 { $$ = node_create_type(1,1,TYPEID_UCHAR,0,0); } 
		| TYPE_UINT16 { $$ = node_create_type(1,1,TYPEID_USHORT,0,0); } 
		| TYPE_UINT32 { $$ = node_create_type(1,1,TYPEID_UINT,0,0); } 
		| TYPE_UINT64 { $$ = node_create_type(1,1,TYPEID_ULONG,0,0); } 
		| TYPE_FLOAT { $$ = node_create_type(1,1,TYPEID_FLOAT,0,0); } 
		| TYPE_DOUBLE { $$ = node_create_type(1,1,TYPEID_DOUBLE,0,0); } 
		| TYPE_OPAQUE { $$ = node_create_type(1,1,TYPEID_OPAQUE,0,1); } 
		| TYPE_INT8_VEC2 { $$ = node_create_type(1,2,TYPEID_CHAR,0,0); } 
		| TYPE_INT16_VEC2 { $$ = node_create_type(1,2,TYPEID_SHORT,0,0); } 
		| TYPE_INT32_VEC2 { $$ = node_create_type(1,2,TYPEID_INT,0,0); } 
		| TYPE_INT64_VEC2 { $$ = node_create_type(1,2,TYPEID_LONG,0,0); } 
		| TYPE_UINT8_VEC2 { $$ = node_create_type(1,2,TYPEID_UCHAR,0,0); } 
		| TYPE_UINT16_VEC2 { $$ = node_create_type(1,2,TYPEID_USHORT,0,0); } 
		| TYPE_UINT32_VEC2 { $$ = node_create_type(1,2,TYPEID_UINT,0,0); } 
		| TYPE_UINT64_VEC2 { $$ = node_create_type(1,2,TYPEID_ULONG,0,0); } 
		| TYPE_FLOAT_VEC2 { $$ = node_create_type(1,2,TYPEID_FLOAT,0,0); } 
		| TYPE_DOUBLE_VEC2 { $$ = node_create_type(1,2,TYPEID_DOUBLE,0,0); } 
		| TYPE_INT8_VEC4 { $$ = node_create_type(1,4,TYPEID_CHAR,0,0); } 
		| TYPE_INT16_VEC4 { $$ = node_create_type(1,4,TYPEID_SHORT,0,0); } 
		| TYPE_INT32_VEC4 { $$ = node_create_type(1,4,TYPEID_INT,0,0); } 
		| TYPE_INT64_VEC4 { $$ = node_create_type(1,4,TYPEID_LONG,0,0); } 
		| TYPE_UINT8_VEC4 { $$ = node_create_type(1,4,TYPEID_UCHAR,0,0); } 
		| TYPE_UINT16_VEC4 { $$ = node_create_type(1,4,TYPEID_USHORT,0,0); } 
		| TYPE_UINT32_VEC4 { $$ = node_create_type(1,4,TYPEID_UINT,0,0); } 
		| TYPE_UINT64_VEC4 { $$ = node_create_type(1,4,TYPEID_ULONG,0,0); } 
		| TYPE_FLOAT_VEC4 { $$ = node_create_type(1,4,TYPEID_FLOAT,0,0); } 
		| TYPE_DOUBLE_VEC4 { $$ = node_create_type(1,4,TYPEID_DOUBLE,0,0); } 
		| TYPE_INT8_VEC8 { $$ = node_create_type(1,8,TYPEID_CHAR,0,0); } 
		| TYPE_INT16_VEC8 { $$ = node_create_type(1,8,TYPEID_SHORT,0,0); } 
		| TYPE_INT32_VEC8 { $$ = node_create_type(1,8,TYPEID_INT,0,0); } 
		| TYPE_INT64_VEC8 { $$ = node_create_type(1,8,TYPEID_LONG,0,0); } 
		| TYPE_UINT8_VEC8 { $$ = node_create_type(1,8,TYPEID_UCHAR,0,0); } 
		| TYPE_UINT16_VEC8 { $$ = node_create_type(1,8,TYPEID_USHORT,0,0); } 
		| TYPE_UINT32_VEC8 { $$ = node_create_type(1,8,TYPEID_UINT,0,0); } 
		| TYPE_UINT64_VEC8 { $$ = node_create_type(1,8,TYPEID_ULONG,0,0); } 
		| TYPE_FLOAT_VEC8 { $$ = node_create_type(1,8,TYPEID_FLOAT,0,0); } 
		| TYPE_DOUBLE_VEC8 { $$ = node_create_type(1,8,TYPEID_DOUBLE,0,0); } 
		| TYPE_INT8_VEC16 { $$ = node_create_type(1,16,TYPEID_CHAR,0,0); } 
		| TYPE_INT16_VEC16 { $$ = node_create_type(1,16,TYPEID_SHORT,0,0); } 
		| TYPE_INT32_VEC16 { $$ = node_create_type(1,16,TYPEID_INT,0,0); } 
		| TYPE_INT64_VEC16 { $$ = node_create_type(1,16,TYPEID_LONG,0,0); } 
		| TYPE_UINT8_VEC16 { $$ = node_create_type(1,16,TYPEID_UCHAR,0,0); } 
		| TYPE_UINT16_VEC16 { $$ = node_create_type(1,16,TYPEID_USHORT,0,0); } 
		| TYPE_UINT32_VEC16 { $$ = node_create_type(1,16,TYPEID_UINT,0,0); } 
		| TYPE_UINT64_VEC16 { $$ = node_create_type(1,16,TYPEID_ULONG,0,0); } 
		| TYPE_FLOAT_VEC16 { $$ = node_create_type(1,16,TYPEID_FLOAT,0,0); } 
		| TYPE_DOUBLE_VEC16 { $$ = node_create_type(1,16,TYPEID_DOUBLE,0,0); } 
		| ADDRSPACE_GLOBAL type { $$=node_set_addrspace($2,1); } 
		| ADDRSPACE_LOCAL type { $$=node_set_addrspace($2,3); } 
		| type ptrc { $$=node_set_ptrc($1,$2); } ;

ptrc:		ptrc '*' { $$ = $1+1; }
			| '*' { $$ = 1; };

array:	array '[' ICONST ']' { $$ = $1+1; };
			| '[' ICONST ']' { $$ = 2; };

struct:	STRUCT BODY 
				{ $$ = $1; };

typedef:	TYPEDEF struct SYMBOL ';' 
				{ add_typedef(symbuf+$3); };

body:		BODY { $$=$1; };


%%

void
yyerror( const char* s)
{
	printf("ERROR: %s\n",s);
}


