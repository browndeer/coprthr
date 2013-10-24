/* xclnm.h
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


/*
 * Declares globals initialized by main assembler and referenced by yyparse
 */

/* DAR */

#ifndef _XCLNM_H
#define _XCLNM_H

extern char* yy_ch_buf;
extern char* yy_ch_pos;


#include "elf_cl.h"
//#include "../../src/libocl/ocl_types.h"
//#include "ocl_types.h"
#include "coprthr_types.h"

#include "xclnm_node.h"

#define STRBUFSIZE	1048576

extern char* symbuf;

extern int symbuf_sz;

extern node_t* cur_nptr;

extern int add_str(char* buf, int* sz, const char* s);
extern int add_typedef(char* s);
extern int is_type(char* s);

node_t* sched0( node_t* nptr );

#define YY_(m) m

struct _clsymtab_entry* xclnm( unsigned char* pfile, size_t filesz );

extern char* typbuf[16384];
extern int ntypbuf;

extern char* locbuf;
extern size_t locbufsz;

#define __locbuf_append_char( c ) locbuf[locbufsz++];

#define __locbuf_append_string( s ) do { \
	strncpy(locbuf+locbufsz,s,strnlen(s,262144)+1); \
	locbufsz += strnlen(s,262144); \
	} while(0)

#define __locbuf_clear() locbufsz = 0; *locbuf = '\0';

//#define __plb() __locbuf_append_string(yytext)
//#define __rlb() printf("%d:|%s|\n",locbufsz,locbuf); locbufsz=0
#define __plb() do {} while(0)
#define __rlb() do {} while(0)

#endif


