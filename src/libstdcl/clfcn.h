/* clfcn.h
 *
 * Copyright (c) 2009 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
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

#ifndef _CLFCN_H
#define _CLFCN_H

#include <sys/queue.h>

#if defined(__APPLE__)
#include "OpenCL/opencl.h"
#else
#include "CL/cl.h"
#endif

#include "clcontext.h"
#include "clinit.h"


#define CLLD_DEFAULT	0	
#define CLLD_LAZY		1
#define CLLD_NOW 		2
#define CLLD_NOBUILD	4
#define CLLD_GLOBAL	8


struct _prgs_struct {
	LIST_ENTRY(_prgs_struct) prgs_list;
	const char* fname;
	size_t len;
	int fd;
	void* ptr;
	unsigned int refc;
};


struct _krntab_struct {
	char kname[256];
	cl_uint nargs;
	cl_uint refc;
	cl_context kctx;
	cl_program kprg;
};


struct _txt_struct {
	LIST_ENTRY(_txt_struct) txt_list;
	struct _prgs_struct* prgs;
	cl_program prg;
	cl_uint nkrn;
	cl_kernel* krn;
	struct _krntab_struct* krntab;
};


#ifdef __cplusplus
extern "C" {
#endif

void* clload( CONTEXT* cp, void* ptr, size_t sz, int flags );
void* clbuild( CONTEXT* cp, void* handle, char* options, int flags );
void* clopen( CONTEXT* cp, const char* fname, int flags );
void* clsopen( CONTEXT* cp, const char* srcstr, int flags );
cl_kernel clsym( CONTEXT* cp, void* handle, const char* sname, int flags );
int clclose(CONTEXT* cp, void* handle);
char* clerror(void);

#ifdef __cplusplus
}
#endif

#endif

