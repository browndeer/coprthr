/* e32pth_engine_needham.h
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

#ifndef _e32pth_engine_needham_h
#define _e32pth_engine_needham_h

#include <sys/types.h>
#include <stdint.h>

#ifdef __xcl_kcall__
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


#include "workp.h"

struct thr_data {
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


#if !defined(__xcl_kcall__) && !defined(__xcl_kthr__)

void* e32pth_engine_startup_needham( void* p );

int e32pth_engine_klaunch_needham( int engid_base, int ne, struct workp* wp, 
	struct cmdcall_arg* argp );

int e32pth_engine_ready_needham();

#endif

#endif

