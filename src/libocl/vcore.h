/* vcore.h 
 *
 * Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _VCORE_H
#define _VCORE_H

#include <stdint.h>
//#include <setjmp.h>

//#include "cpuset_type.h"

#ifndef __xcl_kcall__
#include "cmdcall.h"
#else
/* XXX this brings in the vec types.  temporary soln. improve it. -DAR */
#include <CL/cl.h>
typedef cl_int4 int4;
typedef cl_float4 float4;
#endif

#if defined(USE_FAST_SETJMP)
#define __vc_setjmp  fast_setjmp
#define __vc_longjmp fast_longjmp
#define __vc_jmp_buf fast_jmp_buf
typedef unsigned long long fast_jmp_buf[8];
#else
#include <setjmp.h>
#define __vc_setjmp  setjmp
#define __vc_longjmp longjmp
#define __vc_jmp_buf jmp_buf
#endif


//#define VCORE_NE				2			/* NOT USED number of engines	*/
#define VCORE_NC				64			/* number of vcores per engine 	*/
#define VCORE_STACK_SZ		16384		/* stack size per vcore 			*/
//#define VCORE_STACK_SZ		32768		/* stack size per vcore 			*/
//#define VCORE_STACK_SZ		65536		/* stack size per vcore 			*/
#define VCORE_LOCAL_MEM_SZ	32768	/* local mem size per engine		*/

#define VCORE_STACK_MASK (~(VCORE_STACK_SZ-1))
#define __fp() __builtin_frame_address(0)
#define __getvcdata() (struct vc_data*)(((intptr_t)__fp())&VCORE_STACK_MASK)


#define __setvcdata(pdata) __asm__ __volatile__ ( \
	"movq %%rbp,%%r14\n\t" \
	"andq $-16384,%%r14\n\t" \
	"movq %%r14,%0\n" \
	: "=m" (pdata) : : "%r14" \
	)

/*
#define __setvcdata(pdata) __asm__ __volatile__ ( \
   "movq %%rbp,%%r14\n\t" \
   "andq $-32768,%%r14\n\t" \
   "movq %%r14,%0\n" \
   : "=m" (pdata) : : "%r14" \
   )
*/
/*
#define __setvcdata(pdata) __asm__ __volatile__ ( \
   "movq %%rbp,%%r14\n\t" \
   "andq $-65536,%%r14\n\t" \
   "movq %%r14,%0\n" \
   : "=m" (pdata) : : "%r14" \
   )
*/
/*
#define __setvcdata(pdata) __asm__ __volatile__ ( \
   "movq %%rbp,%%r14\n\t" \
   "andq $-262144,%%r14\n\t" \
   "movq %%r14,%0\n" \
   : "=m" (pdata) : : "%r14" \
   )
*/



struct vcengine_data {
	int veid;
	void* funcp;
	void* callp;
	void** pr_arg_vec;
	void* pr_arg_buf;
	int vc_runc;
};

struct work_struct {
	unsigned int tdim;
	size_t ltsz[3];
	size_t gsz[3];
	size_t gtsz[3];
	size_t gid[3];
	size_t gtid[3];
};

struct vc_data {
	int vcid;
	__vc_jmp_buf* vcengine_jbufp;
	__vc_jmp_buf* this_jbufp;
	__vc_jmp_buf* next_jbufp;
	struct work_struct* workp;
	size_t ltid[3];
};
#define VC_DATA_JB_THIS	12
#define VC_DATA_JB_NEXT	20



//struct vcengine_init_arg {
//	int dummy;
//};

//#define VCORE_STACK_MASK (~(VCORE_STACK_SZ-1))
//#define __fp() __builtin_frame_address(0)
//#define __getvcdata() (struct vc_data*)(((intptr_t)__fp())&VCORE_STACK_MASK)


#ifndef __xcl_kcall__
void* vcproc_startup(void*);
//void* vcproc_cmd( struct cmdcall_arg* );
void* vcproc_cmd( int veid_base, int nve, struct cmdcall_arg* );
#endif

#endif

