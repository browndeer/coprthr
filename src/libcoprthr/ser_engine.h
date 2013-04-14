/* ser_engine.h
 *
 * Copyright (c) 2009-2012 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _ser_engine_h
#define _ser_engine_h

#include <sys/types.h>
#include <stdint.h>
#include <setjmp.h>

#ifdef __xcl_kcall__
/* XXX this brings in the vec types.  temporary soln. improve it. -DAR */
#include <CL/cl.h>
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef cl_char2 char2;
typedef cl_char4 char4;
typedef cl_char8 char8;
typedef cl_char16 char16;
typedef cl_uchar2 uchar2;
typedef cl_uchar4 uchar4;
typedef cl_uchar8 uchar8;
typedef cl_uchar16 uchar16;
typedef cl_short2 short2;
typedef cl_short4 short4;
typedef cl_short8 short8;
typedef cl_short16 short16;
typedef cl_ushort2 ushort2;
typedef cl_ushort4 ushort4;
typedef cl_ushort8 ushort8;
typedef cl_ushort16 ushort16;
typedef cl_int2 int2;
typedef cl_int4 int4;
typedef cl_int8 int8;
typedef cl_int16 int16;
typedef cl_uint2 uint2;
typedef cl_uint4 uint4;
typedef cl_uint8 uint8;
typedef cl_uint16 uint16;
typedef cl_long2 long2;
typedef cl_long4 long4;
typedef cl_long8 long8;
typedef cl_long16 long16;
typedef cl_ulong2 ulong2;
typedef cl_ulong4 ulong4;
typedef cl_ulong8 ulong8;
typedef cl_ulong16 ulong16;
typedef cl_float2 float2;
typedef cl_float4 float4;
typedef cl_float8 float8;
typedef cl_float16 float16;
typedef cl_double2 double2;
typedef cl_double4 double4;
typedef cl_double8 double8;
typedef cl_double16 double16;
#endif


#include "workp.h"

#define MAX_NUM_THR  64
#define BLK_LOCAL_MEM_SZ 32768

#define THR_STACK_SZ 16384
#define THR_STACK_MASK (~(THR_STACK_SZ-1))
#define __fp() __builtin_frame_address(0)
#define __get_thr_data() (struct thr_data*)(((intptr_t)__fp())&THR_STACK_MASK)

struct thr_data {
//   int vcid;
//   jmp_buf* engine_jbufp;
//   jmp_buf* this_jbufp;
//   jmp_buf* next_jbufp;
   struct workp_entry* we;
   uint32_t blkidx[3];
   uint32_t gtdidx[3];
   uint32_t ltdidx[3];
};

struct engine_data { 
   int engid; 
   void* funcp;
   void* callp;
   uint32_t* pr_arg_off;
   void* pr_arg_buf;
	void* stack_base;
};

#ifdef __xcl_kthr__

static __inline unsigned int get_work_dim()
{ return((__get_thr_data())->we->ndr_dim); }

static __inline size_t get_num_groups(uint d)
{  
   size_t g = (__get_thr_data())->we->ndr_gtdsz[d];
   size_t l = (__get_thr_data())->we->ndr_ltdsz[d];
   return(g/l);
}

static __inline size_t get_group_id(uint d)
{ return((__get_thr_data())->blkidx[d]); }
   
static __inline size_t get_global_size(uint d)
{ return((__get_thr_data())->we->ndr_gtdsz[d]); }
   
static __inline unsigned int get_global_id(unsigned int d)
{ return((unsigned int)(__get_thr_data())->gtdidx[d]); }

static __inline size_t get_local_size(uint d)
{ return((__get_thr_data())->we->ndr_ltdsz[d]); }

static __inline size_t get_local_id(uint d)
{ return((__get_thr_data())->ltdidx[d]); }

#define barrier(flags) do { \
} while(0)

#endif

#if !defined(__xcl_kcall__) && !defined(__xcl_kthr__)

//static void* ser_engine( void* p );

void* ser_engine_startup( void* p );

void* ser_engine_klaunch( int engid_base, int ne, struct workp* wp, 
	struct cmdcall_arg* argp );

int ser_engine_ready();

#endif

#endif

