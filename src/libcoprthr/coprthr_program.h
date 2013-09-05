/* coprthr_program.h
 *
 * Copyright (c) 2009-2013 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _coprthr_program_h
#define _coprthr_program_h

#include <sys/queue.h>
#include <pthread.h>

#include "cpuset_type.h"
#include "cmdcall.h"

/*
#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif
#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif
*/

#define CLARG_KIND_UNDEFINED	0x000
#define CLARG_KIND_VOID       0x001
#define CLARG_KIND_DATA       0x002
#define CLARG_KIND_GLOBAL     0x004
#define CLARG_KIND_LOCAL      0x008
#define CLARG_KIND_CONSTANT   0x010
#define CLARG_KIND_SAMPLER    0x020
#define CLARG_KIND_IMAGE2D    0x040
#define CLARG_KIND_IMAGE3D    0x080

/*
struct _coprthr_strtab_entry {
   size_t alloc_sz;
   size_t sz;
   char* buf;
};

struct _coprthr_clsymtab_entry {
	cl_uint kind;
	cl_uint type;
};
*/

#define CLSYM_KIND_


/* kernel */

struct _coprthr_ksyms_struct {
	int flags;
	void* kthr;
	void* kthr2;
	void* kcall;
	void* kcall2;
};

struct coprthr_kernel {
	void* v_kbin;
	struct _coprthr_ksyms_struct** v_ksyms;
	cl_uint knum;

	cl_uint* arg_kind;
	size_t* arg_sz;
	uint32_t* arg_off;

	size_t arg_buf_sz;
	void* arg_buf;
};

/* XXX does not yet deal with actual kbin and ksym -DAR */
#define __coprthr_init_kernel(imp) do { \
	(imp) = (struct coprthr_kernel*)malloc(sizeof(struct coprthr_kernel)); \
	(imp)->v_kbin = 0; \
	(imp)->v_ksyms = 0; \
	(imp)->knum = -1; \
	(imp)->arg_kind = 0; \
	(imp)->arg_sz = 0; \
	(imp)->arg_off = 0; \
	(imp)->arg_buf_sz = 0; \
	(imp)->arg_buf = 0; \
	} while(0)

#define __coprthr_free_kernel(imp) do { \
	__free((imp)->arg_kind); \
	__free((imp)->arg_sz); \
	__free((imp)->arg_off); \
	__free((imp)->arg_buf); \
	__free((imp)); \
	} while(0)


/* program */

struct coprthr_program {

	cl_uint nclsym;
	struct clsymtab_entry* clsymtab;
	struct clargtab_entry* clargtab;
	char* clstrtab;

	cl_uint nkrn;
	char** kname;
	cl_uint* knarg;
	size_t* karg_buf_sz;
	cl_uint** karg_kind;
	size_t** karg_sz;

	void** v_kbin;
	char** v_kbin_tmpfile;
	struct _coprthr_ksyms_struct** v_ksyms;

};

#define __coprthr_init_program(imp) do { \
	(imp) = (struct coprthr_program*)malloc(sizeof(struct coprthr_program)); \
	(imp)->nclsym = 0; \
	(imp)->clsymtab = 0; \
	(imp)->clstrtab = 0; \
	(imp)->nkrn = 0; \
	(imp)->kname = 0; \
	(imp)->knarg = 0; \
	(imp)->karg_kind = 0; \
	(imp)->karg_buf_sz = 0; \
	(imp)->karg_sz = 0; \
	(imp)->v_kbin = 0; \
	(imp)->v_kbin_tmpfile = 0; \
	(imp)->v_ksyms = 0; \
	} while(0)

/* XXX does not yet deal with actual kbin and ksym -DAR */
#define __coprthr_free_program(imp) do { \
	int k; \
	__free((imp)->clsymtab); \
	__free((imp)->clstrtab); \
	__free((imp)->kname); \
	__free((imp)->knarg); \
	__free((imp)->karg_buf_sz); \
	for(k=0;k<(imp)->nkrn;k++) {__free((imp)->karg_kind[k]); __free((imp)->karg_sz[k]);} \
	__free((imp)->karg_kind); \
	__free((imp)->karg_sz); \
	__free((imp)); \
	} while(0)

#define __nkernels_in_program(prg) (prg->imp->nkrn)


/*
struct _elf_data {
	char filename[256];
	void* dlh;
	void* map;
};

#define __init_elf_data(ed) do { \
   (ed).filename[0] = '\0'; \
   (ed).dlh = 0; \
   (ed).map = 0; \
   } while(0)
*/

//extern void* __icd_call_vector;

#endif

