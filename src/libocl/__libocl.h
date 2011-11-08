/* __libocl.h
 *
 * Copyright (c) 2011 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef __libocl_h
#define __libocl_h

#if defined(__GNUC__) 

/***
 *** included if compiling opencl kernel as c++ with gcc
 ***/

#if !defined(__cplusplus)
#error must be compiled as c++
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#include <math.h>
#include <xmmintrin.h>


/***
 *** vcore defines and structs 
 ***/

#define __vc_setjmp  setjmp
#define __vc_longjmp longjmp
#define __vc_jmp_buf jmp_buf

#define VCORE_NC           64       /* number of vcores per engine   */
#define VCORE_STACK_SZ     16384    /* stack size per vcore          */
#define VCORE_LOCAL_MEM_SZ 32768 /* local mem size per engine     */

#define VCORE_STACK_MASK (~(VCORE_STACK_SZ-1))
#define __fp() __builtin_frame_address(0)
#define __getvcdata() (struct vc_data*)(((intptr_t)__fp())&VCORE_STACK_MASK)

#define __setvcdata(pdata) __asm__ __volatile__ ( \
   "movq %%rbp,%%r14\n\t" \
   "andq $-16384,%%r14\n\t" \
   "movq %%r14,%0\n" \
   : "=m" (pdata) : : "%r14" \
   )

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

#if defined(__FreeBSD__)
typedef unsigned int uint;
#endif

extern "C" {

static __inline unsigned int get_global_id(unsigned int d) 
{
   struct vc_data* data = __getvcdata();
   return((unsigned int)data->ltid[d] + data->workp->gtid[d]);
}

static __inline uint get_work_dim() 
{ return((__getvcdata())->workp->tdim); }

static __inline size_t get_local_size(uint d) 
{ return((__getvcdata())->workp->ltsz[d]); }

static __inline size_t get_local_id(uint d) 
{ return((__getvcdata())->ltid[d]); }

static __inline size_t get_num_groups(uint d) 
{ return((__getvcdata())->workp->gsz[d]); }

static __inline size_t get_global_size(uint d) 
{ return((__getvcdata())->workp->gtsz[d]); }

static __inline size_t get_group_id(uint d) 
{ return((__getvcdata())->workp->gid[d]); }

/*
static __inline void barrier( int flags )
{
   struct vc_data* data;
   __setvcdata(data);
   if (!(__vc_setjmp(*(data->this_jbufp))))
      __vc_longjmp(*(data->next_jbufp),flags);
}
*/
//#define barrier(a)
void barrier( int flags );

/*
#define barrier(flags) do { \
   struct vc_data* data;\
   __setvcdata(data);\
   if (!(__vc_setjmp(*(data->this_jbufp))))\
      __vc_longjmp(*(data->next_jbufp),flags);\
} while(0)
*/


}

#define CLK_LOCAL_MEM_FENCE 1

/*** adress space qualifiers [6.5] ***/

#define __global
#define __local
#define __constant


/*** image access qualifiers [6.6] ***/

#define __read_only
#define __write_only


/*** function qualifiers [6.7] ***/

#define __kernel


/*** builtin vector data types [6.1.2] ***/

typedef int _v2si __attribute__((__vector_size__(8)));
typedef int _v4si __attribute__((__vector_size__(16)));
typedef long _v2sl __attribute__((__vector_size__(16)));
typedef unsigned int _v2i __attribute__((__vector_size__(8)));
typedef unsigned int _v4i __attribute__((__vector_size__(16)));
typedef unsigned long _v2l __attribute__((__vector_size__(16)));
typedef float _v2f __attribute__((__vector_size__(8)));
typedef float _v4f __attribute__((__vector_size__(16)));
typedef double _v2d __attribute__((__vector_size__(16)));

typedef int __int2 __attribute__((__vector_size__(8)));
typedef int __int4 __attribute__((__vector_size__(16)));
typedef long __long2 __attribute__((__vector_size__(16)));
typedef unsigned int __uint2 __attribute__((__vector_size__(8)));
typedef unsigned int __uint4 __attribute__((__vector_size__(16)));
typedef unsigned long __ulong2 __attribute__((__vector_size__(16)));
typedef float __float2 __attribute__((__vector_size__(8)));
typedef float __float4 __attribute__((__vector_size__(16)));
typedef double __double2 __attribute__((__vector_size__(16)));

struct _int2 {
	_int2() {}
	~_int2() {}
	_int2( __m64 v ) : vec(v) {}
	_int2( int a ) : x(a), y(a) {}
	_int2( int a, int b ) : x(a), y(b) {}
	_int2( int* p ) : x(p[0]), y(p[1]) {}
	_int2& operator+ () { return *this; }
	union {
		struct { __m64 vec; };
		struct { int x,y; };
		struct { __m64 xy; };
		struct { int s0,s1; };
		struct { __m64 s01; };
	};
};
typedef struct _int2 int2;

struct _int4 {
	_int4() {}
	~_int4() {}
	_int4( __m128i v ) : vec(v) {}
	_int4( int a ) : x(a), y(a), z(a), w(a) {}
	_int4( int a, int b, int c, int d ) : x(a), y(b), z(c), w(d) {}
	_int4( int* p ) : x(p[0]), y(p[1]), z(p[2]), w(p[3]) {}
	_int4& operator+ () { return *this; }
	union {
		struct { __m128i vec; };
		struct { int x,y,z,w; };
		struct { __m64 xy,zw; };
		struct { __m128i xyzw; };
		struct { int s0,s1,s2,s3; };
		struct { __m64 s12,s34; };
		struct { __m128i s1234; };
	};
};
typedef struct _int4 int4;

struct _long2 {
	_long2() {}
	~_long2() {}
	_long2( __m128i v ) : vec(v) {}
	_long2( long a ) : x(a), y(a) {}
	_long2( long a, long b ) : x(a), y(b) {}
	_long2( long* p ) : x(p[0]), y(p[1]) {}
	_long2& operator+ () { return *this; }
	union {
		struct { __m128i vec; };
		struct { long x,y; };
		struct { __m128i xy; };
		struct { long s0,s1; };
		struct { __m128i s01; };
	};
};
typedef struct _long2 long2;

struct _uint2 {
	_uint2() {}
	~_uint2() {}
	_uint2( __m64 v ) : vec(v) {}
	_uint2( unsigned int a ) : x(a), y(a) {}
	_uint2( unsigned int a, unsigned int b ) : x(a), y(b) {}
	_uint2( unsigned int* p ) : x(p[0]), y(p[1]) {}
	_uint2& operator+ () { return *this; }
	union {
		struct { __m64 vec;};
		struct { unsigned int x,y; };
		struct { __m64 xy; };
		struct { unsigned int s0,s1; };
		struct { __m64 s01; };
	};
};
typedef struct _uint2 uint2;

struct _uint4 {
	_uint4() {}
	~_uint4() {}
	_uint4( __m128i v ) : vec(v) {}
	_uint4( unsigned int a ) : x(a), y(a), z(a), w(a) {}
	_uint4( unsigned int a, unsigned int b, unsigned int c, unsigned int d )
		: x(a), y(b), z(c), w(d) {}
	_uint4( unsigned int* p ) : x(p[0]), y(p[1]), z(p[2]), w(p[3]) {}
	_uint4& operator+ () { return *this; }
	union {
		struct { __m128i vec; };
		struct { unsigned int x,y,z,w; };
		struct { __m64 xy,zw; };
		struct { __m128i xyzw; };
		struct { unsigned int s0,s1,s2,s3; };
		struct { __m64 s12,s34; };
		struct { __m128i s1234; };
	};
};
typedef struct _uint4 uint4;

struct _ulong2 {
	_ulong2() {}
	~_ulong2() {}
	_ulong2( __m128i v ) : vec(v) {}
	_ulong2( unsigned long a ) : x(a), y(a) {}
	_ulong2( unsigned long a, unsigned long b ) : x(a), y(b) {}
	_ulong2( unsigned long* p ) : x(p[0]), y(p[1]) {}
	_ulong2& operator+ () { return *this; }
	union {
		struct { __m128i vec; };
		struct { unsigned long x,y; };
		struct { __m128i xy; };
		struct { unsigned long s0,s1; };
		struct { __m128i s01; };
	};
};
typedef struct _ulong2 ulong2;

struct _float2 {
	_float2() {}
	~_float2() {}
	_float2( __m64 v ) : vec(v) {}
	_float2( float a ) : x(a), y(a) {}
	_float2( float a, float b ) : x(a), y(b) {}
	_float2( float* p ) : x(p[0]), y(p[1]) {}
	_float2& operator+ () { return *this; }
	union {
		struct { __m64 vec; };
		struct { float x,y; };
		struct { __m64 xy; };
		struct { float s0,s1; };
		struct { __m64 s01; };
	};
};
typedef struct _float2 float2;

struct _float4 {
	_float4() {}
	~_float4() {}
	_float4( __m128 v ) : vec(v) {}
	_float4( float a ) : x(a), y(a), z(a), w(a) {}
	_float4( float a, float b, float c, float d )
		: x(a), y(b), z(c), w(d) {}
	_float4( float* p ) : x(p[0]), y(p[1]), z(p[2]), w(p[3]) {}
	_float4& operator+ () { return *this; }
	union {
		struct { __m128 vec; };
		struct { float x,y,z,w; };
		struct { __m64 xy,zw; };
		struct { __m128 xyzw; };
		struct { float s0,s1,s2,s3; };
		struct { __m64 s12,s34; };
		struct { __m128 s12s34; };
	};
};
typedef struct _float4 float4;

struct _double2 {
	_double2() {}
	~_double2() {}
	_double2( __m128d v ) : vec(v) {}
	_double2( double a ) : x(a), y(a) {}
	_double2( double a, double b ) : x(a), y(b) {}
	_double2( double* p ) : x(p[0]), y(p[1]) {}
	_double2& operator+ () { return *this; }
	union {
		struct { __m128d vec;	};
		struct { double x,y; };
		struct { __m128d xy; };
		struct { double s0,s1; };
		struct { __m128d s01; };
	};
};
typedef struct _double2 double2;


/*** other builtin data types [6.1.3] ***/

#define image2d_t __global void*


/*** conversions and type casting [6.2] ***/

#define convert_int(x) static_cast<int>(x) 
#define convert_float(x) static_cast<float>(x) 
static __inline double as_double( float2 f2 ) { return *(double*)(&f2); }


/*** operators for vector data types [6.3] ***/

/* operator + */

static __inline _int2 operator+( _int2 a, _int2 b ) 
{ return _mm_add_pi32( a.vec, b.vec ); }

static __inline _int4 operator+( _int4 a, _int4 b ) 
//{ return __builtin_ia32_paddd128( a.vec, b.vec ); }
{ return _mm_add_epi32( a.vec, b.vec ); }

static __inline _uint2 operator+( _uint2 a, _uint2 b ) 
{ return _uint2( a.x + b.x, a.y + b.y ); }

static __inline _uint4 operator+( _uint4 a, _uint4 b ) 
{ return _uint4( a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w ); }

static __inline _long2 operator+( _long2 a, _long2 b ) 
{ return _mm_add_epi64( a.vec, b.vec ); }

static __inline _ulong2 operator+( _ulong2 a, _ulong2 b ) 
{ return _ulong2( a.x + b.x, a.y + b.y ); }

static __inline _float2 operator+( _float2 a, _float2 b ) 
{ return _float2( a.x + b.x, a.y + b.y ); }

static __inline _float4 operator+( _float4 a, _float4 b ) 
{ return _mm_add_ps( a.vec, b.vec ); }

static __inline _double2 operator+( _double2 a, _double2 b ) 
{ return _mm_add_pd( a.vec, b.vec ); }


/* operator - */

static __inline _int2 operator-( _int2 a, _int2 b ) 
{ return _int2( a.x - b.x, a.y - b.y ); }

static __inline _int4 operator-( _int4 a, _int4 b ) 
{ return _int4(  a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w ); }

static __inline _uint2 operator-( _uint2 a, _uint2 b ) 
{ return _uint2( a.x - b.x, a.y - b.y ); }

static __inline _uint4 operator-( _uint4 a, _uint4 b ) 
{ return _uint4( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w ); }

static __inline _long2 operator-( _long2 a, _long2 b ) 
{ return _long2( a.x - b.x, a.y - b.y ); }

static __inline _ulong2 operator-( _ulong2 a, _ulong2 b ) 
{ return _ulong2( a.x - b.x, a.y - b.y ); }

static __inline _float2 operator-( _float2 a, _float2 b ) 
{ return _float2( a.x - b.x, a.y - b.y ); }

static __inline _float4 operator-( _float4 a, _float4 b ) 
{ return _float4( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w ); }

static __inline _double2 operator-( _double2 a, _double2 b ) 
{ return _double2( a.x - b.x, a.y - b.y ); }


/* operator * */

static __inline _int2 operator*( _int2 a, _int2 b ) 
{ return _int2( a.x * b.x, a.y * b.y ); }

static __inline _int4 operator*( _int4 a, _int4 b ) 
{ return _int4( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w ); }

static __inline _uint2 operator*( _uint2 a, _uint2 b ) 
{ return _uint2( a.x * b.x, a.y * b.y ); }

static __inline _uint4 operator*( _uint4 a, _uint4 b ) 
{ return _uint4( a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w ); }

static __inline _long2 operator*( _long2 a, _long2 b ) 
{ return _long2( a.x * b.x, a.y * b.y ); }

static __inline _ulong2 operator*( _ulong2 a, _ulong2 b ) 
{ return _ulong2( a.x * b.x, a.y * b.y ); }

static __inline _float2 operator*( _float2 a, _float2 b ) 
{ return _float2( a.x * b.x, a.y * b.y ); }

static __inline _float4 operator*( _float4 a, _float4 b ) 
{ return _mm_mul_ps( a.vec, b.vec ); }

static __inline _double2 operator*( _double2 a, _double2 b ) 
{ return _mm_mul_pd( a.vec, b.vec ); }


/* operator % */

static __inline _int2 operator%( _int2 a, _int2 b ) 
{ return _int2( a.x % b.x, a.y % b.y ); }

static __inline _int4 operator%( _int4 a, _int4 b ) 
{ return _int4(  a.x % b.x, a.y % b.y, a.z % b.z, a.w % b.w ); }

static __inline _uint2 operator%( _uint2 a, _uint2 b ) 
{ return _uint2( a.x % b.x, a.y % b.y ); }

static __inline _uint4 operator%( _uint4 a, _uint4 b ) 
{ return _uint4( a.x % b.x, a.y % b.y, a.z % b.z, a.w % b.w ); }

static __inline _long2 operator%( _long2 a, _long2 b ) 
{ return _long2( a.x % b.x, a.y % b.y ); }

static __inline _ulong2 operator%( _ulong2 a, _ulong2 b ) 
{ return _ulong2( a.x % b.x, a.y % b.y ); }


/* operator / */

static __inline _int2 operator/( _int2 a, _int2 b ) 
{ return _int2( a.x / b.x, a.y / b.y ); }

static __inline _int4 operator/( _int4 a, _int4 b ) 
{ return _int4(  a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w ); }

static __inline _uint2 operator/( _uint2 a, _uint2 b ) 
{ return _uint2( a.x / b.x, a.y / b.y ); }

static __inline _uint4 operator/( _uint4 a, _uint4 b ) 
{ return _uint4( a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w ); }

static __inline _long2 operator/( _long2 a, _long2 b ) 
{ return _long2( a.x / b.x, a.y / b.y ); }

static __inline _ulong2 operator/( _ulong2 a, _ulong2 b ) 
{ return _ulong2( a.x / b.x, a.y / b.y ); }

static __inline _float2 operator/( _float2 a, _float2 b ) 
{ return _float2( a.x / b.x, a.y / b.y ); }

static __inline _float4 operator/( _float4 a, _float4 b ) 
{ return _float4( a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w ); }

static __inline _double2 operator/( _double2 a, _double2 b ) 
{ return _double2( a.x / b.x, a.y / b.y ); }


/*** these are generic min and max definitions - they should be corrected ***/

#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a<b)?b:a)


/*** math builtin functions [6.11.2] ***/

#define __MATH_BUILTIN_1(func) \
static __inline float func##_T( float a ) { return func##f(a); } \
static __inline float2 func##_T( float2 a ) \
	{ return float2(func##f(a.x),func##f(a.y)); } \
static __inline float4 func##_T( float4 a ) \
	{ return float4(func##f(a.x),func##f(a.y),func##f(a.z),func##f(a.w)); } \
static __inline double func##_T( double a ) { return func(a); } \
static __inline double2 func##_T( double2 a ) \
	{ return double2(func(a.x),func(a.y)); }

__MATH_BUILTIN_1(acos)
__MATH_BUILTIN_1(acosh)
__MATH_BUILTIN_1(asin)
__MATH_BUILTIN_1(asinh)
__MATH_BUILTIN_1(atan)
__MATH_BUILTIN_1(atanh)
__MATH_BUILTIN_1(cbrt)
__MATH_BUILTIN_1(ceil)
__MATH_BUILTIN_1(cos)
__MATH_BUILTIN_1(cosh)
__MATH_BUILTIN_1(erfc)
__MATH_BUILTIN_1(erf)
__MATH_BUILTIN_1(exp)
__MATH_BUILTIN_1(exp2)
__MATH_BUILTIN_1(exp10)
__MATH_BUILTIN_1(expm1)
__MATH_BUILTIN_1(fabs)
__MATH_BUILTIN_1(floor)
__MATH_BUILTIN_1(log)
__MATH_BUILTIN_1(log2)
__MATH_BUILTIN_1(log10)
__MATH_BUILTIN_1(log1p)
__MATH_BUILTIN_1(logb)
__MATH_BUILTIN_1(rint)
__MATH_BUILTIN_1(round)
__MATH_BUILTIN_1(sin)
__MATH_BUILTIN_1(sinh)
__MATH_BUILTIN_1(sqrt)
__MATH_BUILTIN_1(tan)
__MATH_BUILTIN_1(tanh)
__MATH_BUILTIN_1(tgamma)
__MATH_BUILTIN_1(trunc)

#define sqrt(a) sqrt_T(a)
#define acos(a) acos_T(a)
#define acosh(a) acosh_T(a)
#define asin(a) asin_T(a)
#define asinh(a) asinh_T(a)
#define tan(a) tan_T(a)
#define tanh(a) tanh_T(a)
#define cbrt(a) cbrt_T(a)
#define ceil(a) ceil_T(a)
#define cos(a) cos_T(a)
#define cosh(a) cosh_T(a)
#define erfc(a) erfc_T(a)
#define erf(a) erf_T(a)
#define exp(a) exp_T(a)
#define exp2(a) exp2_T(a)
#define exp10(a) exp10_T(a)
#define expm1(a) expm1_T(a)
#define fabs(a) fabs_T(a)
#define floor(a) floor_T(a)
#define log(a) log_T(a)
#define log2(a) log2_T(a)
#define log10(a) log10_T(a)
#define log1p(a) log1p_T(a)
#define logb(a) logb_T(a)
#define rint(a) rint_T(a)
#define round(a) round_T(a)
#define sin(a) sin_T(a)
#define sinh(a) sinh_T(a)
#define sqrt(a) sqrt_T(a)
#define tan(a) tan_T(a)
#define tanh(a) tanh_T(a)
#define tgamma(a) tgamma_T(a)
#define tfunc(a) tfunc_T(a)


/*** sampler declarations [6.11.8.1] ***/

typedef int sampler_t;
#define CLK_NORMALIZED_COORDS_FALSE 0
#define CLK_ADDRESS_NONE 0
#define CLK_FILTER_NEAREST 0


/*** image read write builtins [6.11.18] ***/

#define read_imagei( _img, _smplr, _xy) \
*((int4*)((intptr_t)_img+128)+((__global size_t*)_img)[0]*(_xy).y+(_xy).x)

#define read_imagef( _img, _smplr, _xy) \
*((float4*)((intptr_t)_img+128)+((__global size_t*)_img)[0]*(_xy).y+(_xy).x)


/*** builtin extensions for initializing vector data types [non-standard] ***/

#define __builtin_vector_int2(x,y) 			_int2(x,y)
#define __builtin_vector_int4(x,y,z,w) 	_int4(x,y,z,w)
#define __builtin_vector_long2(x,y) 		_long2(x,y)
#define __builtin_vector_uint2(x,y) 		_uint2(x,y)
#define __builtin_vector_uint4(x,y,z,w) 	_uint4(x,y,z,w)
#define __builtin_vector_ulong2(x,y) 		_ulong2(x,y)
#define __builtin_vector_float2(x,y) 		_float2(x,y)
#define __builtin_vector_float4(x,y,z,w) 	_float4(x,y,z,w)
#define __builtin_vector_double2(x,y) 		_double2(x,y)

#else

/***
 *** included if compiling opencl kernel with an opencl compiler
 ***/

/*** builtin extensions for initializing vector data types [non-standard] ***/

#define __builtin_vector_int2(x,y) 			(int2)(x,y)
#define __builtin_vector_int4(x,y,z,w) 	(int4)(x,y,z,w)
#define __builtin_vector_uint2(x,y) 		(uint2)(x,y)
#define __builtin_vector_uint4(x,y,z,w)	(uint4)(x,y,z,w)
#define __builtin_vector_float2(x,y) 		(float2)(x,y)
#define __builtin_vector_float4(x,y,z,w)	(float4)(x,y,z,w)
#define __builtin_vector_double2(x,y) 		(double2)(x,y)

#endif

#endif

