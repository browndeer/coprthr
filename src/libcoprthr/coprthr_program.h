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

#define CLARG_KIND_UNDEFINED	0x000
#define CLARG_KIND_VOID       0x001
#define CLARG_KIND_DATA       0x002
#define CLARG_KIND_GLOBAL     0x004
#define CLARG_KIND_LOCAL      0x008
#define CLARG_KIND_CONSTANT   0x010
#define CLARG_KIND_SAMPLER    0x020
#define CLARG_KIND_IMAGE2D    0x040
#define CLARG_KIND_IMAGE3D    0x080

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
//	void* v_kbin;
//	struct _coprthr_ksyms_struct** v_ksyms;
//	cl_uint knum;

	unsigned int narg;

	cl_uint* arg_kind;
	size_t* arg_sz;
	uint32_t* arg_off;

	size_t arg_buf_sz;
	void* arg_buf;
};

/* XXX does not yet deal with actual kbin and ksym -DAR */
#define __coprthr_init_kernel(imp) do { \
	(imp) = (struct coprthr_kernel*)malloc(sizeof(struct coprthr_kernel)); \
	(imp)->narg = 0; \
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

//	cl_uint nclsym;
//	struct clsymtab_entry* clsymtab;
//	struct clargtab_entry* clargtab;
//	char* clstrtab;

//	cl_uint nkrn;
	char** kname;
	cl_uint* knarg;
	size_t* karg_buf_sz;
	cl_uint** karg_kind;
	size_t** karg_sz;

//	void** v_kbin;
//	char** v_kbin_tmpfile;
//	struct _coprthr_ksyms_struct** v_ksyms;

};

#define __coprthr_init_program(imp) do { \
	(imp) = (struct coprthr_program*)malloc(sizeof(struct coprthr_program)); \
	(imp)->kname = 0; \
	(imp)->knarg = 0; \
	(imp)->karg_kind = 0; \
	(imp)->karg_buf_sz = 0; \
	(imp)->karg_sz = 0; \
	} while(0)

/* XXX does not yet deal with actual kbin and ksym -DAR */
#define __coprthr_free_program(imp) do { \
	int k; \
	__free((imp)->kname); \
	__free((imp)->knarg); \
	__free((imp)->karg_buf_sz); \
	__free((imp)->karg_kind); \
	__free((imp)->karg_sz); \
	__free((imp)); \
	} while(0)

//#define __nkernels_in_program(prg) (prg->imp->nkrn)
#define __nkernels_in_program(prg) (prg->nkrn)

//extern void* __icd_call_vector;

struct coprthr1_program {

	size_t src_sz;
	char* src;

	size_t bin_sz;
	char* bin;
	int bin_stat;
	char* build_opt;
	char* build_log;

	unsigned int nclsym;
	struct clsymtab_entry* clsymtab;
	struct clargtab_entry* clargtab;
	char* clstrtab;

	unsigned int nkrn;
	char** kname;
	unsigned int* knarg;
	size_t* karg_buf_sz;
	unsigned int** karg_kind;
	size_t** karg_sz;

	void* dlh;
	char* dlfile;
	struct _coprthr_ksyms_struct* v_ksyms;
		
};

struct coprthr1_kernel {
	struct coprthr1_program* prg1;
	unsigned int knum;
};

#endif

