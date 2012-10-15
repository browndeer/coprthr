/* e32sl_if.h
 *
 * Copyright (c) 2012 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _E32SL_IF_H
#define _E32SL_IF_H

#include <sys/types.h>
#include <stdint.h>
#include <setjmp.h>

#include "e32_config.h"
#include "e32sl_mem_if.h"

#if defined(__x86_64__)
#define __host__
#endif

#if !defined(__host__)
#include <e_coreid.h>
#include <e_common.h>
#endif

typedef unsigned long size_t;
typedef unsigned char uchar;
typedef unsigned int uint;

#if defined(__xcl_kcall__)
/* XXX this brings in the vec types.  temporary soln. improve it. -DAR */
#include <CL/cl.h>
typedef unsigned char uchar;
typedef unsigned int uint;
typedef cl_char2 char2;
typedef cl_char4 char4;
typedef cl_uchar2 uchar2;
typedef cl_uchar4 uchar4;
typedef cl_int2 int2;
typedef cl_int4 int4;
typedef cl_uint2 uint2;
typedef cl_uint4 uint4;
typedef cl_long2 long2;
typedef cl_ulong2 ulong2;
typedef cl_float2 float2;
typedef cl_float4 float4;
typedef cl_double2 double2;
#endif

//#define _Ncores NCORES

struct core_control_struct {
   e32_int_t ready[E32_NCORES];
   e32_int_t run[E32_NCORES];
   e32_ptr_t callp[E32_NCORES];
   e32_int_t retval[E32_NCORES];
};

struct core_local_data_struct {
   int    coreID;
   int    corenum;
   int    row;
   int    col;
   int    count;
};

struct thr_data_struct {
   uint32_t blkidx[3];
   uint32_t gtdidx[3];
   uint32_t ltdidx[3];
	jmp_buf* this_jbufp;
	jmp_buf* next_jbufp;
};


//#define THR_STACK_MEMHI 0x7000
#define THR_STACK_MEMHI E32_CORE_LOCAL_MEM_HI - 0x1000
#define THR_STACK_SZ 512

#if defined(__xcl_kcall__) || defined(__xcl_kthr__)

extern struct core_local_data_struct core_local_data;
extern e32_workp_entry_t core_we;
extern struct thr_data_struct thr_data;

extern jmp_buf main_jbuf;

#define THR_STACK_MASK (~(THR_STACK_SZ-1))
#define __fp() __builtin_frame_address(0)
#define __get_thr_data() \
	((struct thr_data_struct*)(((intptr_t)__fp())&THR_STACK_MASK))
#endif

#define __KCALL_DECLS(sym) extern __XCL_func_##sym##_t sym;
#define __KCALL_ATTRIBUTES __attribute__((noreturn))
#define __KCALL_PRE void* arg_buf = e32_kdata_ptr_arg_buf; ++e32_ctrl_run[core_local_data.corenum]; if (!setjmp(*(__get_thr_data()->this_jbufp))) longjmp(main_jbuf,1);
#define __KCALL_POST --e32_ctrl_run[core_local_data.corenum]; longjmp(main_jbuf,1);
#define __kcall_thr_ptr(sym) &sym
#define __kcall_arg(t,n) *(t*)(arg_buf + e32_kdata_ptr_arg_off[n])


#ifdef __xcl_kthr__

static __inline unsigned int get_work_dim()
{ return( we_ndr_dim(core_we)); }

static __inline size_t get_num_groups(uint d)
{
   return( we_ndr_gtdsz(core_we,d)/we_ndr_ltdsz(core_we,d) );
}

static __inline size_t get_group_id(uint d)
{ return( thr_data.blkidx[d] ); }

static __inline size_t get_global_size(uint d)
{ return( we_ndr_gtdsz(core_we,d) ); }

static __inline unsigned int get_global_id(unsigned int d)
{ return((unsigned int)(__get_thr_data())->gtdidx[d]); }

static __inline size_t get_local_size(uint d)
{ return( we_ndr_ltdsz(core_we,d) ); }

static __inline size_t get_local_id(uint d)
{ return((__get_thr_data())->ltdidx[d]); }

/* XXX here we should set an error code in core local data -DAR */
#define barrier(flags) do { \
   if (!(setjmp(*(__get_thr_data()->this_jbufp))))\
      longjmp(*(__get_thr_data()->next_jbufp),flags);\
} while(0)

#endif


#endif

