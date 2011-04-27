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

typedef unsigned int uint;

/*
 * intrinsics called from kernels
 */

void barrier( int flags )
{
	struct vc_data* data;
	__setvcdata(data);
	if (!(__vc_setjmp(*(data->this_jbufp)))) 
		__vc_longjmp(*(data->next_jbufp),flags);
}

uint get_work_dim() { return((__getvcdata())->workp->tdim); }

size_t get_local_size(uint d) { return((__getvcdata())->workp->ltsz[d]); }
//size_t get_local_id(uint d) { return((__getvcdata())->ltid[d]); }
size_t get_local_id(uint d) { return((__getvcdata())->ltid[d]); }

size_t get_num_groups(uint d) { return((__getvcdata())->workp->gsz[d]); }
size_t get_global_size(uint d) { return((__getvcdata())->workp->gtsz[d]); }

size_t get_group_id(uint d) { return((__getvcdata())->workp->gid[d]); }


unsigned int get_global_id(uint d) { 
	struct vc_data* data = __getvcdata();
	return((unsigned int)data->ltid[d] + data->workp->gtid[d]); 
}


#if(0)
#if defined(USE_BDT_BUILTINS)
/* XXX hack, prefetch should do somethig, fix this -DAR */
void __prefetch_g4f32( void* p, size_t n ) {}

//float __rsqrt_f32( float x ) { return(1.0f/sqrt(x)); }

//float __exp_f32( float x ) { return(expf(x)); }

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

float __acos_f32( float x ) { return(acosf(x)); }
float __acosh_f32( float x ) { return(acoshf(x)); }
float __acospi_f32( float x ) { return(M_1_PI*acosf(x)); }

float __asin_f32( float x ) { return(asinf(x)); }
float __asinh_f32( float x ) { return(asinhf(x)); }
float __asinpi_f32( float x ) { return(M_1_PI*asinf(x)); }

float __atan_f32( float x ) { return(atanf(x)); }
float __atanh_f32( float x ) { return(atanhf(x)); }
float __atanpi_f32( float x ) { return(M_1_PI*atanf(x)); }

float __cbrt_f32( float x ) { return(cbrtf(x)); }
float __ceil_f32( float x ) { return(ceilf(x)); }
float __cos_f32( float x ) { return(cosf(x)); }
float __cosh_f32( float x ) { return(coshf(x)); }
float __cospi_f32( float x ) { return(M_1_PI*cosf(x)); }

float __erfc_f32( float x ) { return(erfcf(x)); }
float __erf_f32( float x ) { return(erff(x)); }
float __exp_f32( float x ) { return(expf(x)); }
float __exp2_f32( float x ) { return(exp2f(x)); }
float __exp10_f32( float x ) { return(pow(10.0f,x)); } // exp10f() is broken?
float __expm1_f32( float x ) { return(expm1f(x)); }

float __fabs_f32( float x ) { return(fabsf(x)); }
float __floor_f32( float x ) { return(floorf(x)); }

float __lgamma_f32( float x ) { return(lgammaf(x)); }
float __log_f32( float x ) { return(logf(x)); }
float __log2_f32( float x ) { return(log2f(x)); }
float __log10_f32( float x ) { return(log10f(x)); }
float __log1p_f32( float x ) { return(log1pf(x)); }
float __logb_f32( float x ) { return(logbf(x)); }

float __rint_f32( float x ) { return(rintf(x)); }
float __round_f32( float x ) { return(roundf(x)); }
float __rsqrt_f32( float x ) { return(1.0f/sqrtf(x)); }

float __sin_f32( float x ) { return(sinf(x)); }
float __sinh_f32( float x ) { return(sinhf(x)); }
float __sinpi_f32( float x ) { return(M_1_PI*sinf(x)); }
float __sqrt_f32( float x ) { return(sqrtf(x)); }

float __tan_f32( float x ) { return(tanf(x)); }
float __tanh_f32( float x ) { return(tanhf(x)); }
float __tanpi_f32( float x ) { return(M_1_PI*tanf(x)); }
float __tgamma_f32( float x ) { return(tgammaf(x)); }
float __trunc_f32( float x ) { return(truncf(x)); }

#endif

#endif
