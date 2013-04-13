%{
/* xclnm_scan.l
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
 * This is the flex input for a simply MIPS-like assembler parser
 */

%{
/* DAR */
%}

WS			[ \t\v\f\n]
DIG		[0-9]
SYM1		[a-zA-Z_]
SYM		[a-zA-Z_0-9]
SYMDOT1	[a-zA-Z_\.]
SYMDOT	[a-zA-Z_0-9\.]


%{

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "xclnm.h"
#include "xclnm_node.h"
#include "xclnm_gram.h"

#define ival_return(x) __xclnm_lval.ival = x; return(x);

FILE* __xclnm_yyin;

#define YY_INPUT(buf,result,max_sz) \
  { result = fread(buf,1,max_sz,__xclnm_yyin); }

%}


%% /* rules */

"typedef"			{ __plb(); ival_return(TYPEDEF); }
"struct"				{ __plb(); ival_return(STRUCT); }
"union"				{ __plb(); ival_return(STRUCT); }

"__kernel" 			{ __plb(); ival_return(KERNEL); }
"kernel" 			{ __plb(); ival_return(KERNEL); }
"__global" 			{ __plb(); ival_return(ADDRSPACE_GLOBAL); }
"global" 			{ __plb(); ival_return(ADDRSPACE_GLOBAL); }
"__local" 			{ __plb(); ival_return(ADDRSPACE_LOCAL); }
"local" 				{ __plb(); ival_return(ADDRSPACE_LOCAL); }
"__read_only" 		{ __plb(); }
"read_only" 		{ __plb(); }
"__write_only" 	{ __plb(); }
"write_only" 		{ __plb(); }
"const" 				{ __plb(); }
"restrict" 			{ __plb(); }
"__private" 		{ __plb(); }
"private" 			{ __plb(); }
"__constant" 		{ __plb(); }
"constant" 			{ __plb(); }
"inline" 			{ __plb(); }
"__inline" 			{ __plb(); }

"void" 							{ __plb(); ival_return(TYPE_VOID); }
"char" 							{ __plb(); ival_return(TYPE_INT8); }
"short" 							{ __plb(); ival_return(TYPE_INT16); }
"int" 							{ __plb(); ival_return(TYPE_INT32); }
"long" 							{ __plb(); ival_return(TYPE_INT64); }
"uchar" 							{ __plb(); ival_return(TYPE_UINT8); }
"unsigned"{WS}+"char" 		{ __plb(); ival_return(TYPE_UINT8); }
"ushort"							{ __plb(); ival_return(TYPE_UINT16); }
"unsigned"{WS}+"short"		{ __plb(); ival_return(TYPE_UINT16); }
"uint" 							{ __plb(); ival_return(TYPE_UINT32); }
"unsigned"{WS}+"int" 		{ __plb(); ival_return(TYPE_UINT32); }
"ulong" 							{ __plb(); ival_return(TYPE_UINT64); }
"unsigned"{WS}+"long" 		{ __plb(); ival_return(TYPE_UINT64); }
"float" 							{ __plb(); ival_return(TYPE_FLOAT); }
"double"							{ __plb(); ival_return(TYPE_DOUBLE); }

"image2d_t"	{ __plb(); ival_return(TYPE_OPAQUE); }

"char2" 							{ __plb(); ival_return(TYPE_INT8_VEC2); }
"short2" 						{ __plb(); ival_return(TYPE_INT16_VEC2); }
"int2" 							{ __plb(); ival_return(TYPE_INT32_VEC2); }
"long2" 							{ __plb(); ival_return(TYPE_INT64_VEC2); }
"uchar2"							{ __plb(); ival_return(TYPE_UINT8_VEC2); }
"unsigned"{WS}+"char2"		{ __plb(); ival_return(TYPE_UINT8_VEC2); }
"ushort2" 						{ __plb(); ival_return(TYPE_UINT16_VEC2); }
"unsigned"{WS}+"short2" 	{ __plb(); ival_return(TYPE_UINT16_VEC2); }
"uint2" 							{ __plb(); ival_return(TYPE_UINT32_VEC2); }
"unsigned"{WS}+"int2" 		{ __plb(); ival_return(TYPE_UINT32_VEC2); }
"ulong2"							{ __plb(); ival_return(TYPE_UINT64_VEC2); }
"unsigned"{WS}+"ulong2"		{ __plb(); ival_return(TYPE_UINT64_VEC2); }
"float2" 							{ __plb(); ival_return(TYPE_FLOAT_VEC2); }
"double2"							{ __plb(); ival_return(TYPE_DOUBLE_VEC2); }

"char4" 							{ __plb(); ival_return(TYPE_INT8_VEC4); }
"short4" 						{ __plb(); ival_return(TYPE_INT16_VEC4); }
"int4" 							{ __plb(); ival_return(TYPE_INT32_VEC4); }
"long4" 							{ __plb(); ival_return(TYPE_INT32_VEC4); }
"uchar4" 						{ __plb(); ival_return(TYPE_UINT8_VEC4); }
"unsigned"{WS}+"char4" 		{ __plb(); ival_return(TYPE_UINT8_VEC4); }
"ushort4" 						{ __plb(); ival_return(TYPE_UINT16_VEC4); }
"unsigned"{WS}+"short4" 	{ __plb(); ival_return(TYPE_UINT16_VEC4); }
"uint4" 							{ __plb(); ival_return(TYPE_UINT32_VEC4); }
"unsigned"{WS}+"int4" 		{ __plb(); ival_return(TYPE_UINT32_VEC4); }
"ulong4" 						{ __plb(); ival_return(TYPE_UINT64_VEC4); }
"unsigned"{WS}+"long4" 		{ __plb(); ival_return(TYPE_UINT64_VEC4); }
"float4" 						{ __plb(); ival_return(TYPE_FLOAT_VEC4); }
"double4"						{ __plb(); ival_return(TYPE_DOUBLE_VEC4); }

"char8"                                 { __plb(); ival_return(TYPE_INT8_VEC8); }
"short8"                           { __plb(); ival_return(TYPE_INT16_VEC8); }
"int8"                                  { __plb(); ival_return(TYPE_INT32_VEC8); }
"long8"                                 { __plb(); ival_return(TYPE_INT32_VEC8); }
"uchar8"                           { __plb(); ival_return(TYPE_UINT8_VEC8); }
"unsigned"{WS}+"char8"        { __plb(); ival_return(TYPE_UINT8_VEC8); }
"ushort8"                               { __plb(); ival_return(TYPE_UINT16_VEC8); }
"unsigned"{WS}+"short8"  { __plb(); ival_return(TYPE_UINT16_VEC8); }
"uint8"                                 { __plb(); ival_return(TYPE_UINT32_VEC8); }
"unsigned"{WS}+"int8"         { __plb(); ival_return(TYPE_UINT32_VEC8); }
"ulong8"                           { __plb(); ival_return(TYPE_UINT64_VEC8); }
"unsigned"{WS}+"long8"        { __plb(); ival_return(TYPE_UINT64_VEC8); }
"float8"                           { __plb(); ival_return(TYPE_FLOAT_VEC8); }
"double8"                          { __plb(); ival_return(TYPE_DOUBLE_VEC8); }

"char16"                                 { __plb(); ival_return(TYPE_INT8_VEC16); }
"short16"                           { __plb(); ival_return(TYPE_INT16_VEC16); }
"int16"                                  { __plb(); ival_return(TYPE_INT32_VEC16); }
"long16"                                 { __plb(); ival_return(TYPE_INT32_VEC16); }
"uchar16"                           { __plb(); ival_return(TYPE_UINT8_VEC16); }
"unsigned"{WS}+"char16"        { __plb(); ival_return(TYPE_UINT8_VEC16); }
"ushort16"                               { __plb(); ival_return(TYPE_UINT16_VEC16); }
"unsigned"{WS}+"short16"  { __plb(); ival_return(TYPE_UINT16_VEC16); }
"uint16"                                 { __plb(); ival_return(TYPE_UINT32_VEC16); }
"unsigned"{WS}+"int16"         { __plb(); ival_return(TYPE_UINT32_VEC16); }
"ulong16"                           { __plb(); ival_return(TYPE_UINT64_VEC16); }
"unsigned"{WS}+"long16"        { __plb(); ival_return(TYPE_UINT64_VEC16); }
"float16"                           { __plb(); ival_return(TYPE_FLOAT_VEC16); }
"double16"                          { __plb(); ival_return(TYPE_DOUBLE_VEC16); }

"..."			{ __plb(); return(VARG); }

{DIG}+		{ __plb(); __xclnm_lval.ival = atoi(yytext); return(ICONST); }

{DIG}+{WS}*x		{ __plb(); __xclnm_lval.ival = atoi(yytext); return(VECN); }

{SYM1}{SYM}* {__plb();
                     if (is_type(yytext)) {
                        ival_return(TYPE_OPAQUE);
                     } else {
                        __xclnm_lval.ival = add_str(symbuf,&symbuf_sz,yytext);
                        return(SYMBOL);
                     }
                  }


"{" 	{ 
			__plb();
			int level = 1;
			char c;
			while(level > 0) {
				c = input();
				if (c=='{') ++level;
				else if (c=='}') --level;
			}
			ival_return(BODY); 
		};

"__attribute__(("		{
								__plb();
								int level = 2;
								char c;
								while(level > 0) {
									c = input();
									if (c=='(') ++level;
									else if (c==')') --level;
								}
							};

"//".+"\n"	{ __plb(); };

"/*"([^\*]|\*[^/])*"*/" 	{ __plb(); };

^{WS}*"#".+"\n"	{ __plb(); }

";"			{ __plb(); return(';'); }
","			{ __plb(); return(','); }
"*"			{ __plb(); return('*'); }
"("         { __plb(); return('('); }
")"         { __plb(); return(')'); }
"<"         { __plb(); return('<'); }
">"         { __plb(); return('>'); }
"["         { __plb(); return('['); }
"]"         { __plb(); return(']'); }
"="         { __plb(); return('='); }
"\n"         { __plb(); }

{WS}			{ __plb(); }
.

%%


int
yywrap(void)
{
   if (0) unput(0); /* quiet gcc */
   return(1);
}

FILE* __xclnm_yyin = 0;




