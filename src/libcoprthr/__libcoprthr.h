/* __libcoprthr.h
 *
 * Copyright (c) 2011-2012 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef __libcoprthr_h
#define __libcoprthr_h

//#define USE_SSE __SSE__

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

#if defined(USE_SSE) 
#include <xmmintrin.h>
#endif


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

#if defined(__x86_64__)
#define __setvcdata(pdata) __asm__ __volatile__ ( \
   "movq %%rbp,%%r14\n\t" \
   "andq $-16384,%%r14\n\t" \
   "movq %%r14,%0\n" \
   : "=m" (pdata) : : "%r14" \
   )
#elif defined(__arm__)
#define __setvcdata(pdata) __asm__ __volatile__ ( \
   "mov %%r3,%%fp\n\t" \
   "bic %%r3,%%r3,#16320\n\t" \
   "bic %%r3,%%r3,#63\n\t" \
   "str %%r3,%0\n" \
   : "=m" (pdata) : : "%r3" \
   )
#else
#error unsupported architecture
#endif


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
//void barrier( int flags );

#define barrier(flags) do { \
   struct vc_data* data;\
   __setvcdata(data);\
   if (!(__vc_setjmp(*(data->this_jbufp))))\
      __vc_longjmp(*(data->next_jbufp),flags);\
} while(0)


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


/*** fix restrict qualifier for GCC ***/
#define restrict __restrict


/*** builtin vector data types [6.1.2] ***/

//typedef int _v2si __attribute__((__vector_size__(8)));
//typedef int _v4si __attribute__((__vector_size__(16)));
//typedef long _v2sl __attribute__((__vector_size__(16)));
//typedef unsigned int _v2i __attribute__((__vector_size__(8)));
//typedef unsigned int _v4i __attribute__((__vector_size__(16)));
//typedef unsigned long _v2l __attribute__((__vector_size__(16)));
//typedef float _v2f __attribute__((__vector_size__(8)));
//typedef float _v4f __attribute__((__vector_size__(16)));
//typedef double _v2d __attribute__((__vector_size__(16)));
//
//typedef int __int2 __attribute__((__vector_size__(8)));
//typedef int __int4 __attribute__((__vector_size__(16)));
//typedef long __long2 __attribute__((__vector_size__(16)));
//typedef unsigned int __uint2 __attribute__((__vector_size__(8)));
//typedef unsigned int __uint4 __attribute__((__vector_size__(16)));
//typedef unsigned long __ulong2 __attribute__((__vector_size__(16)));
//typedef float __float2 __attribute__((__vector_size__(8)));
//typedef float __float4 __attribute__((__vector_size__(16)));
//typedef double __double2 __attribute__((__vector_size__(16)));

/* if not using sse we will define some equivalents -DAR */
//#if !defined(USE_SSE)
//typedef __int2 __m64;
//typedef __int4 __m128i;
//typedef __float4 __m128;
//typedef __double2 __m128d;
//#endif


/* let the templating begins ... -DAR */

/* stub class for vector low-level storage classes */

template< typename T, int D >
struct __vector_type { };


/* vec2 low-level storage class */

template <>
struct __vector_type<int,2>
   { typedef int type_t __attribute__((__vector_size__(8))); };

template <>
struct __vector_type<unsigned int,2>
   { typedef unsigned int type_t __attribute__((__vector_size__(8))); };

template <>
struct __vector_type<long,2>
   { typedef long type_t __attribute__((__vector_size__(16))); };

template <>
struct __vector_type<unsigned long,2>
   { typedef unsigned long type_t __attribute__((__vector_size__(16))); };

template <>
struct __vector_type<float,2>
   { typedef float type_t __attribute__((__vector_size__(8))); };

template <>
struct __vector_type<double,2>
   { typedef double type_t __attribute__((__vector_size__(16))); };


/* vec4 low-level storage class */

template <>
struct __vector_type<int,4>
   { typedef int type_t __attribute__((__vector_size__(16))); };

template <>
struct __vector_type<unsigned int,4>
   { typedef unsigned int type_t __attribute__((__vector_size__(16))); };

template <>
struct __vector_type<float,4>
   { typedef float type_t __attribute__((__vector_size__(16))); };


/* typedefs for low-level vector storage classes */

typedef __vector_type<int,2> __int2;
typedef __vector_type<unsigned int,2> __uint2;
typedef __vector_type<long,2> __long2;
typedef __vector_type<unsigned long,2> __ulong2;
typedef __vector_type<float,2> __float2;
typedef __vector_type<double,2> __double2;

typedef __vector_type<int,4> __int4;
typedef __vector_type<unsigned int,4> __uint4;
typedef __vector_type<float,4> __float4;


/* stub class for vector high-level implementation */

template < typename T, int D >
struct _vector_type { };

/***
 *** vec2 high-level implementation
 ***/

template < typename T >
struct _vector_type<T,2> {

	typedef T type_t;

	_vector_type() {}
	~_vector_type() {}

	_vector_type( const typename __vector_type<type_t,2>::type_t& v ) : vec(v) {}
//	_vector_type( typename __vector_type<type_t,2>::type_t v ) : vec(v) {}
	_vector_type( type_t a ) : s0(a), s1(a) {}
	_vector_type( type_t a0, type_t a1 ) : s0(a0), s1(a1) {}
	_vector_type( type_t* p ) : s0(p[0]), s1(p[1]) {}

	_vector_type& operator+();
	_vector_type& operator-();
	_vector_type& operator~();

	_vector_type& operator+=( _vector_type vec );
	_vector_type& operator-=( _vector_type vec );
	_vector_type& operator*=( _vector_type vec );
	_vector_type& operator%=( _vector_type vec );
	_vector_type& operator/=( _vector_type vec );
	_vector_type& operator&=( _vector_type vec );
	_vector_type& operator|=( _vector_type vec );
	_vector_type& operator^=( _vector_type vec );

	union {
		struct { typename __vector_type<type_t,2>::type_t vec; };
		struct { type_t x,y; };
		struct { type_t s0,s1; };
		struct { typename __vector_type<type_t,2>::type_t xy; };
		struct { typename __vector_type<type_t,2>::type_t s01; };
	};

};

/* unary plus */
template < typename T >
_vector_type<T,2>& _vector_type<T,2>::operator+() 
	{ return *this; }

/* unary minus */
template < typename T >
_vector_type<T,2>& 
	_vector_type<T,2>::operator-() 
	{ vec = -vec; return *this; }

/* unary bitwise not */
template < typename T >
_vector_type<T,2>& 
	_vector_type<T,2>::operator~() 
	{ vec = ~vec; return *this; }

/* add assign */
template < typename T >
_vector_type<T,2>& 
	_vector_type<T,2>::operator+=( _vector_type<T,2> rhs ) 
	{ vec += rhs.vec; return *this; }
#if defined (USE_SSE)
template <>
_vector_type<int,2>& 
	_vector_type<int,2>::operator+=(_vector_type<int,2> rhs)
	{ vec = _mm_add_pi32(vec,rhs.vec); return *this; }
#endif

/* subtract assign */
template < typename T >
_vector_type<T,2>& _vector_type<T,2>::operator-=( _vector_type<T,2> rhs ) 
	{ vec -= rhs.vec; return *this; }
#if defined (USE_SSE)
template <>
_vector_type<int,2>& 
	_vector_type<int,2>::operator-=(_vector_type<int,2> rhs)
	{ vec = _mm_sub_pi32(vec,rhs.vec); return *this; }
#endif

/* multiply assign */
template < typename T >
_vector_type<T,2>& _vector_type<T,2>::operator*=( _vector_type<T,2> rhs ) 
	{ vec *= rhs.vec; return *this; }

/* modulo assign */
template < typename T >
_vector_type<T,2>& _vector_type<T,2>::operator%=( _vector_type<T,2> rhs ) 
	{ vec %= rhs.vec; return *this; }

/* divide assign */
template < typename T >
_vector_type<T,2>& _vector_type<T,2>::operator/=( _vector_type<T,2> rhs ) 
	{ vec /= rhs.vec; return *this; }

/* bitwise and assign */
template < typename T >
_vector_type<T,2>& _vector_type<T,2>::operator&=( _vector_type<T,2> rhs ) 
	{ vec &= rhs.vec; return *this; }

/* bitwise or assign */
template < typename T >
_vector_type<T,2>& _vector_type<T,2>::operator|=( _vector_type<T,2> rhs ) 
	{ vec |= rhs.vec; return *this; }

/* bitwise xor assign */
template < typename T >
_vector_type<T,2>& _vector_type<T,2>::operator^=( _vector_type<T,2> rhs ) 
	{ vec ^= rhs.vec; return *this; }


/***
 *** vec4 high-level implementation
 ***/

#include <xmmintrin.h>

template < typename T >
struct _vector_type<T,4> {

	typedef T type_t;

	_vector_type() {}
	~_vector_type() {}

	_vector_type( const typename __vector_type<type_t,4>::type_t& v ) : vec(v) {}
//	_vector_type( typename __vector_type<type_t,4>::type_t v ) : vec(v) {}
	_vector_type( type_t a ) : s0(a), s1(a), s2(a), s3(a) {}
//	_vector_type( type_t a ) : vec(_mm_set1_ps(a)) {}
	_vector_type( type_t a0, type_t a1, type_t a2, type_t a3 ) 
		: s0(a0), s1(a1), s2(a2), s3(a3) {}
	_vector_type( type_t* p ) : s0(p[0]), s1(p[1]), s2(p[2]), s3(p[3]) {}

	_vector_type& operator+();
	_vector_type& operator-();
	_vector_type& operator~();

	_vector_type& operator+=( _vector_type vec );
	_vector_type& operator-=( _vector_type vec );
	_vector_type& operator*=( _vector_type vec );
	_vector_type& operator%=( _vector_type vec );
	_vector_type& operator/=( _vector_type vec );
	_vector_type& operator&=( _vector_type vec );
	_vector_type& operator|=( _vector_type vec );
	_vector_type& operator^=( _vector_type vec );

	union {
		struct { typename __vector_type<type_t,4>::type_t vec; };
		struct { type_t x,y,z,w; };
		struct { type_t s0,s1,s2,s3; };
		struct { typename __vector_type<type_t,4>::type_t xyzw; };
		struct { typename __vector_type<type_t,4>::type_t s0123; };
		struct { typename __vector_type<type_t,2>::type_t xy,zw; };
		struct { typename __vector_type<type_t,2>::type_t s01,s23; };
	};

};

/* unary plus */
template < typename T >
_vector_type<T,4>& 
_vector_type<T,4>::operator+() 
	{ return *this; }

/* unary minus */
template < typename T >
_vector_type<T,4>& 
_vector_type<T,4>::operator-() 
	{ vec = -vec; return *this; }

/* unary not */
template < typename T >
_vector_type<T,4>& 
_vector_type<T,4>::operator~() 
	{ vec = ~vec; return *this; }

/* add assign */
template < typename T >
_vector_type<T,4>& 
_vector_type<T,4>::operator+=( _vector_type<T,4> rhs ) 
	{ vec += rhs.vec; return *this; }
#if defined (USE_SSE)
template <>
_vector_type<long,2>& 
_vector_type<long,2>::operator+=( _vector_type<long,2> rhs ) 
	{ vec = _mm_add_epi64( vec, rhs.vec ); return *this; }

template <>
_vector_type<double,2>& 
_vector_type<double,2>::operator+=( _vector_type<double,2> rhs ) 
	{ vec = _mm_add_pd( vec, rhs.vec ); return *this; }

template <>
_vector_type<int,4>& 
_vector_type<int,4>::operator+=( _vector_type<int,4> rhs ) 
	{ vec = _mm_add_epi32( vec, rhs.vec ); return *this; }

template <>
_vector_type<float,4>& 
_vector_type<float,4>::operator+=( _vector_type<float,4> rhs ) 
	{ vec = _mm_add_ps( vec, rhs.vec ); return *this; }
#endif

/* subtract assign */
template < typename T >
_vector_type<T,4>& _vector_type<T,4>::operator-=( _vector_type<T,4> rhs ) 
	{ vec -= rhs.vec; return *this; }
#if defined (USE_SSE)
template <>
_vector_type<long,2>& 
_vector_type<long,2>::operator-=( _vector_type<long,2> rhs ) 
	{ vec = _mm_sub_epi64( vec, rhs.vec ); return *this; }

template <>
_vector_type<double,2>& 
_vector_type<double,2>::operator-=( _vector_type<double,2> rhs ) 
	{ vec = _mm_sub_pd( vec, rhs.vec ); return *this; }

template <>
_vector_type<int,4>& 
_vector_type<int,4>::operator-=( _vector_type<int,4> rhs ) 
	{ vec = _mm_sub_epi32( vec, rhs.vec ); return *this; }

template <>
_vector_type<float,4>& 
_vector_type<float,4>::operator-=( _vector_type<float,4> rhs ) 
	{ vec = _mm_sub_ps( vec, rhs.vec ); return *this; }
#endif

/* multiply assign */
template < typename T >
_vector_type<T,4>& _vector_type<T,4>::operator*=( _vector_type<T,4> rhs ) 
	{ vec *= rhs.vec; return *this; }

/* modulo assign */
template < typename T >
_vector_type<T,4>& _vector_type<T,4>::operator%=( _vector_type<T,4> rhs ) 
	{ vec %= rhs.vec; return *this; }

/* divide assign */
template < typename T >
_vector_type<T,4>& _vector_type<T,4>::operator/=( _vector_type<T,4> rhs ) 
	{ vec /= rhs.vec; return *this; }

/* bitwise and assign */
template < typename T >
_vector_type<T,4>& _vector_type<T,4>::operator&=( _vector_type<T,4> rhs ) 
	{ vec &= rhs.vec; return *this; }

/* bitwise or assign */
template < typename T >
_vector_type<T,4>& _vector_type<T,4>::operator|=( _vector_type<T,4> rhs ) 
	{ vec |= rhs.vec; return *this; }

/* bitwise xor assign */
template < typename T >
_vector_type<T,4>& _vector_type<T,4>::operator^=( _vector_type<T,4> rhs ) 
	{ vec ^= rhs.vec; return *this; }


/* typedefs for high-level vector implementations  */

typedef _vector_type<int,2> _int2;
typedef _vector_type<unsigned int,2> _uint2;
typedef _vector_type<long,2> _long2;
typedef _vector_type<unsigned long,2> _ulong2;
typedef _vector_type<float,2> _float2;
typedef _vector_type<double,2> _double2;

typedef _vector_type<int,4> _int4;
typedef _vector_type<unsigned int,4> _uint4;
typedef _vector_type<float,4> _float4;

typedef _int2 int2;
typedef _uint2 uint2;
typedef _long2 long2;
typedef _ulong2 ulong2;
typedef _float2 float2;
typedef _double2 double2;

typedef _int4 int4;
typedef _uint4 uint4;
typedef _float4 float4;



/*** other builtin data types [6.1.3] ***/

#define image2d_t __global void*


/*** conversions and type casting [6.2] ***/

#define convert_int(x) static_cast<int>(x) 
#define convert_float(x) static_cast<float>(x) 
static __inline double as_double( float2 f2 ) { return *(double*)(&f2); }


/*** 
 *** operators for vector data types [6.3] 
 ***/

#define __GENERIC_BINOP(op) \
template < typename T, int D > \
static __inline _vector_type<T,D>  \
operator op( _vector_type<T,D> a, _vector_type<T,D> b ) \
	{ return _vector_type<T,D>( a.vec op b.vec ); } \
template < typename T, int D > \
static __inline _vector_type<T,D> \
operator op( typename _vector_type<T,D>::type_t a, _vector_type<T,D> b ) \
	{ return _vector_type<T,D>(a) op b; } \
template < typename T, int D > \
static __inline _vector_type<T,D> \
operator op( _vector_type<T,D> a, typename _vector_type<T,D>::type_t b ) \
	{ return a op _vector_type<T,D>(b); }

/* binary operator + */

__GENERIC_BINOP(+)
__GENERIC_BINOP(-)
__GENERIC_BINOP(*)
__GENERIC_BINOP(%)
__GENERIC_BINOP(/)
__GENERIC_BINOP(&)
__GENERIC_BINOP(|)
__GENERIC_BINOP(^)

#if defined(USE_SSE)

/* binary operator + */

static __inline _int2
operator+( _int2 a, _int2 b ) 
	{ return _int2(_mm_add_pi32( a.vec, b.vec ) ); }

template <>
static __inline _long2
operator+( _long2 a, _long2 b ) 
	{ return _long2( _mm_add_epi64( a.vec, b.vec ) ); }

template <>
static __inline _double2
operator+( _double2 a, _double2 b ) 
	{ return _double2( _mm_add_pd( a.vec, b.vec ) ); }

template <>
static __inline _int4
operator+( _int4 a, _int4 b ) 
	{ return _int4( _mm_add_epi32( a.vec, b.vec ) ); }

template <>
static __inline _float4
operator+( _float4 a, _float4 b ) 
	{ return _float4( _mm_add_ps( a.vec, b.vec ) ); }

/* binary operator - */

static __inline _int2
operator-( _int2 a, _int2 b ) 
	{ return _int2(_mm_sub_pi32( a.vec, b.vec ) ); }

template <>
static __inline _long2
operator-( _long2 a, _long2 b ) 
	{ return _long2( _mm_sub_epi64( a.vec, b.vec ) ); }

template <>
static __inline _double2
operator-( _double2 a, _double2 b ) 
	{ return _double2( _mm_sub_pd( a.vec, b.vec ) ); }

template <>
static __inline _int4
operator-( _int4 a, _int4 b ) 
	{ return _int4( _mm_sub_epi32( a.vec, b.vec ) ); }

template <>
static __inline _float4
operator-( _float4 a, _float4 b ) 
	{ return _float4( _mm_sub_ps( a.vec, b.vec ) ); }

/* binary operator * */

template <>
static __inline _double2
operator*( _double2 a, _double2 b )
	{ return _double2( _mm_mul_pd( a.vec, b.vec ) ); }

template <>
static __inline _float4 
operator*( _float4 a, _float4 b )
	{ return _float4( _mm_mul_ps( a.vec, b.vec ) ); }

#endif



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
#ifndef __FreeBSD__
__MATH_BUILTIN_1(exp10)
#else
#warning FreeBSD missing exp10
#endif
__MATH_BUILTIN_1(expm1)
__MATH_BUILTIN_1(fabs)
__MATH_BUILTIN_1(floor)
__MATH_BUILTIN_1(log)
#ifndef __FreeBSD__
__MATH_BUILTIN_1(log2)
#else
#warning FreeBSD missing log2
#endif
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

static __inline float rsqrt_T( float a ) { return 1.0f/sqrtf(a); } \
static __inline float2 rsqrt_T( float2 a ) \
	{ return float2(1.0f/sqrtf(a.x),1.0f/sqrtf(a.y)); } \
static __inline float4 rsqrt_T( float4 a ) \
	{ return float4(1.0f/sqrtf(a.x),1.0f/sqrtf(a.y),1.0f/sqrtf(a.z),1.0f/sqrtf(a.w)); } \
static __inline double rsqrt_T( double a ) { return 1.0f/sqrt(a); } \
static __inline double2 rsqrt_T( double2 a ) \
	{ return double2(1.0f/sqrt(a.x),1.0f/sqrt(a.y)); }

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
#define rsqrt(a) rsqrt_T(a)
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

