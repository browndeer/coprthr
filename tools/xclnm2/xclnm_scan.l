%{
/* xclnm_scan.l
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

"__kernel" 	{ __plb(); ival_return(KERNEL); }
"__global" 	{ __plb(); }
"__local" 	{ __plb(); }
"__read_only" 	{ __plb(); }
"__write_only" 	{ __plb(); }

"void" 		{ __plb(); ival_return(TYPE_VOID); }
"i8" 			{ __plb(); ival_return(TYPE_INT8); }
"i16" 		{ __plb(); ival_return(TYPE_INT16); }
"i32" 		{ __plb(); ival_return(TYPE_INT32); }
"int" 		{ __plb(); ival_return(TYPE_INT32); }
"uint" 		{ __plb(); ival_return(TYPE_UINT32); }
"i64" 		{ __plb(); ival_return(TYPE_INT64); }
"float" 		{ __plb(); ival_return(TYPE_FLOAT); }
"double"		{ __plb(); ival_return(TYPE_DOUBLE); }
"image2d_t"	{ __plb(); ival_return(TYPE_OPAQUE); }

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
//fprintf(stderr,"XXX |%c|\n",c);
			}
			ival_return(BODY); 
		};

"//".+"\n"	{ __plb(); };

"/*"[^\*]*"*/"	{ __plb(); };

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




