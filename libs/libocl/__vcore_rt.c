/* __vcore_rt.c
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

#include "CL/cl.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <setjmp.h>	/* XXX this is conditionally included in vcore.h */
#include <stdint.h>

/* XXX should include vcore.h, but test this for side effects -DAR */

#include "vcore.h"


/*
 * intrinsics called from kernels
 */

void barrier( int flags )
{
//	struct vc_data* data = __getvcdata();
	struct vc_data* data;
//	if (!(setjmp(*(data->this_jbufp)))) longjmp(*(data->next_jbufp),flags);
	__setvcdata(data);
	if (!(__vc_setjmp(*(data->this_jbufp)))) 
		__vc_longjmp(*(data->next_jbufp),flags);
}

uint get_work_dim() { return((__getvcdata())->workp->tdim); }

size_t get_local_size(uint d) { return((__getvcdata())->workp->ltsz[d]); }
size_t get_local_id(uint d) { return((__getvcdata())->ltid[d]); }

size_t get_num_groups(uint d) { return((__getvcdata())->workp->gsz[d]); }
size_t get_global_size(uint d) { return((__getvcdata())->workp->gtsz[d]); }

size_t get_group_id(uint d) { return((__getvcdata())->workp->gid[d]); }


unsigned int get_global_id(uint d) { 
	struct vc_data* data = __getvcdata();
	return((unsigned int)data->ltid[d] + data->workp->gtid[d]); 
}


#if(1)
/* XXX hack, prefetch should do somethig, fix this -DAR */
void __prefetch_g4f32( void* p, size_t n ) {}

float __rsqrt_f32( float x ) { return(1.0f/sqrt(x)); }

float __exp_f32( float x ) { return(expf(x)); }

cl_float4 __fabs_4f32( cl_float4 x)
{
   cl_int4 m = {0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff};
      asm (
      "movaps %1,%%xmm0\n\t"
      "movaps %2,%%xmm1\n\t"
      "andps %%xmm0,%%xmm1\n\t"
      "movaps %%xmm1,%0\n\t"
      : "=m" (x) : "m" (m), "m" (x)
   );
   return(x);
}
#endif


