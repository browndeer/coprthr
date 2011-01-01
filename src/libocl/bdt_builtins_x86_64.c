/* bdt_builtins_x86_64.c
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

//#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <setjmp.h>	/* XXX this is conditionally included in vcore.h */
#include <stdint.h>

/* XXX should include vcore.h, but test this for side effects -DAR */

//#include "vcore.h"


//#if defined(USE_BDT_BUILTINS)

/* XXX hack, prefetch should do somethig, fix this -DAR */

void __prefetch_g4f32( void* p, size_t n ) {}

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

int __ilogb_f32( float x ) { return(ilogb(x)); }


float __atan2_f32( float x, float y ) { return(atan2f(x,y)); }
float __atan2pi_f32( float x, float y ) { return(M_1_PI*atan2f(x,y)); }

float __copysign_f32( float x, float y ) { return(copysignf(x,y)); }

float __fdim_f32( float x, float y ) { return(fdimf(x,y)); }
float __fmax_f32( float x, float y ) { return(fmaxf(x,y)); }
float __fmin_f32( float x, float y ) { return(fminf(x,y)); }
float __fmod_f32( float x, float y ) { return(fmodf(x,y)); }

float __hypot_f32( float x, float y ) { return(hypotf(x,y)); }

float __nextafter_f32( float x, float y ) { return(nextafterf(x,y)); }

float __pow_f32( float x, float y ) { return(powf(x,y)); }
float __powr_f32( float x, float y ) { return(powf(x,y)); }

float __remainder_f32( float x, float y ) { return(remainderf(x,y)); }

float __ldexp_f32i32( float x, int y ) { return(ldexpf(x,y)); }
float __pown_f32i32( float x, int y ) { return(powf(x,(float)y)); }
float __rootn_f32i32( float x, int y ) { return(powf(x,1.0f/(float)y)); }

float __fma_f32( float x, float y, float z ) { return(fmaf(x,y,z)); }
float __mad_f32( float x, float y, float z ) { return(fmaf(x,y,z)); }

//#endif


