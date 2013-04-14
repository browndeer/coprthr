/* opencl_lift.h
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

#ifndef _opencl_lift_h
#define _opencl_lift_h

#include <stdio.h>

#define GCC_VERSION \
	( __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ )

/* XXX tell compiler, yes, I really meant inline when I said inline -DAR */
#ifndef __always_inline
#define __always_inline __inline __attribute__((always_inline))
#endif

//#define USE_SSE __SSE__

#if defined(__GNUC__) 

/***
 *** included if compiling opencl kernel as c++ with gcc
 ***/

#if !defined(__cplusplus)
#error must be compiled as c++
#endif

#undef _FORTIFY_SOURCE

#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#include <math.h>

#if defined(USE_SSE) 
#include <xmmintrin.h>
#endif


#if defined(__FreeBSD__)
typedef unsigned int uint;
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;


#define CLK_LOCAL_MEM_FENCE 1
#define CLK_GLOBAL_MEM_FENCE 1

#define prefetch( x, y ) 

/*** math constants [6.11.2] ***/

#undef M_E
#undef M_LOG2E
#undef M_LOG10E
#undef M_LN2
#undef M_LN10
#undef M_PI
#undef M_PI_2
#undef M_PI_4
#undef M_1_PI
#undef M_2_PI
#undef M_2_SQRTPI
#undef M_SQRT2
#undef M_SQRT1_2

#define M_E        2.718281828459045090796
#define M_LOG2E    1.442695040888963387005
#define M_LOG10E   0.434294481903251816668
#define M_LN2      0.693147180559945286227
#define M_LN10     2.302585092994045901094
#define M_PI       3.141592653589793115998
#define M_PI_2     1.570796326794896557999
#define M_PI_4     0.785398163397448278999
#define M_1_PI     0.318309886183790691216
#define M_2_PI     0.636619772367581382433
#define M_2_SQRTPI 1.128379167095512558561
#define M_SQRT2    1.414213562373095145475
#define M_SQRT1_2  0.707106781186547572737

#define M_E_F        2.71828174591064f
#define M_LOG2E_F    1.44269502162933f
#define M_LOG10E_F   0.43429449200630f
#define M_LN2_F      0.69314718246460f
#define M_LN10_F     2.30258512496948f
#define M_PI_F       3.14159274101257f
#define M_PI_2_F     1.57079637050629f
#define M_PI_4_F     0.78539818525314f
#define M_1_PI_F     0.31830987334251f
#define M_2_PI_F     0.63661974668503f
#define M_2_SQRTPI_F 1.12837922573090f
#define M_SQRT2_F    1.41421353816986f
#define M_SQRT1_2_F  0.70710676908493f

/*** adress space qualifiers [6.5] ***/

#define __global
#define __local
#define __constant
#define __private

#define global __global
#define local __local
#define constant __constant
#define private __private



/*** image access qualifiers [6.6] ***/

#define __read_only
#define __write_only


/*** function qualifiers [6.7] ***/

#define __kernel


/*** fix restrict qualifier for GCC ***/
#define restrict __restrict


/*** builtin vector data types [6.1.2] ***/

#define VECTOR_TYPE_2( ctype, cltype ) \
typedef ctype __##cltype##2 __attribute__((vector_size(2*sizeof(ctype)))); \
typedef union { \
	typedef ctype type_t; \
   __##cltype##2 vec; \
   struct { ctype x,y; }; \
   struct { ctype s0,s1; }; \
   struct { ctype lo,hi; }; \
   ctype s[2]; \
} _##cltype##2; \
typedef _##cltype##2 cltype##2;

#define VECTOR_TYPE_4( ctype, cltype ) \
typedef ctype __##cltype##4 __attribute__((vector_size(4*sizeof(ctype)))); \
typedef union { \
	typedef ctype type_t; \
   __##cltype##4 vec; \
   struct { ctype x,y,z,w; }; \
   struct { ctype s0,s1,s2,s3; }; \
   struct { cltype##2 xy,zw; }; \
   struct { cltype##2 s01,s23; }; \
   struct { cltype##2 lo,hi; }; \
   ctype s[4]; \
} _##cltype##4; \
typedef _##cltype##4 cltype##4;

#define VECTOR_TYPE_8( ctype, cltype ) \
typedef ctype __##cltype##8 __attribute__((vector_size(8*sizeof(ctype)))); \
typedef union { \
	typedef ctype type_t; \
   __##cltype##8 vec; \
   struct { ctype x,y,z,w,__spacer4,__spacer5,__spacer6,__spacer7; }; \
   struct { ctype s0,s1,s2,s3,s4,s5,s6,s7; }; \
   struct { cltype##2 xy,zw,__spacer45,__spacer67; }; \
   struct { cltype##2 s01,s23,s45,s67; }; \
   struct { cltype##4 s0123,s4567; }; \
   struct { cltype##4 lo,hi; }; \
   ctype s[8]; \
} _##cltype##8; \
typedef _##cltype##8 cltype##8;

#define VECTOR_TYPE_16( ctype, cltype ) \
typedef ctype __##cltype##16 __attribute__((vector_size(16*sizeof(ctype)))); \
typedef union { \
	typedef ctype type_t; \
   __##cltype##16 vec; \
   struct { ctype x,y,z,w,__spacer4,__spacer5,__spacer6,__spacer7,__spacer8,__spacer9,sa,sb,sc,sd,se,sf; }; \
   struct { ctype s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sA,sB,sC,sD,sE,sF; }; \
   struct { cltype##2 xy,zw,__spacer45,__spacer67,__spacer89,sab,scd,sef; }; \
   struct { cltype##2 s01,s23,s45,s67,s89,sAB,sCD,sEF; }; \
   struct { cltype##4 xyzw,__spacer4567,s89ab,scdef; }; \
   struct { cltype##4 s0123,s4567,s89AB,sCDEF; }; \
   struct { cltype##8 s01234567,s89abcdef; }; \
   struct { cltype##8 __spacer01234567,s89ABCDEF; }; \
   struct { cltype##8 lo,hi; }; \
   ctype s[16]; \
} _##cltype##16; \
typedef _##cltype##16 cltype##16;

VECTOR_TYPE_2(char,char)
#define __vector_char2(a,b) (__char2){a,b}
#define _vector_char2(a,b) (_char2){(__char2){a,b}}
#define vector_char2(a,b) _vector_char2(a,b)

VECTOR_TYPE_2( unsigned char, uchar )
#define __vector_uchar2(a,b) (__uchar2){a,b}
#define _vector_uchar2(a,b) (_uchar2){(__uchar2){a,b}}
#define vector_uchar2(a,b) _vector_uchar2(a,b)

VECTOR_TYPE_2(short,short)
#define __vector_short2(a,b) (__short2){a,b}
#define _vector_short2(a,b) (_short2){(__short2){a,b}}
#define vector_short2(a,b) _vector_short2(a,b)

VECTOR_TYPE_2( unsigned short, ushort )
#define __vector_ushort2(a,b) (__ushort2){a,b}
#define _vector_ushort2(a,b) (_ushort2){(__ushort2){a,b}}
#define vector_ushort2(a,b) _vector_ushort2(a,b)

VECTOR_TYPE_2( int, int )
#define __vector_int2(a,b) (__int2){a,b}
#define _vector_int2(a,b) (_int2){(__int2){a,b}}
#define vector_int2(a,b) _vector_int2(a,b)

VECTOR_TYPE_2(unsigned int, uint)
#define __vector_uint2(a,b) (__uint2){a,b}
#define _vector_uint2(a,b) (_uint2){(__uint2){a,b}}
#define vector_uint2(a,b) _vector_uint2(a,b)

VECTOR_TYPE_2( long long, long )
#define __vector_long2(a,b) (__long2){a,b}
#define _vector_long2(a,b) (_long2){(__long2){a,b}}
#define vector_long2(a,b) _vector_long2(a,b)

VECTOR_TYPE_2( unsigned long long, ulong )
#define __vector_ulong2(a,b) (__ulong2){a,b}
#define _vector_ulong2(a,b) (_ulong2){(__ulong2){a,b}}
#define vector_ulong2(a,b) _vector_ulong2(a,b)

VECTOR_TYPE_2( float, float )
#define __vector_float2(a,b) (__float2){a,b}
#define _vector_float2(a,b) (_float2){(__float2){a,b}}
#define vector_float2(a,b) _vector_float2(a,b)

VECTOR_TYPE_2( double, double )
#define __vector_double2(a,b) (__double2){a,b}
#define _vector_double2(a,b) (_double2){(__double2){a,b}}
#define vector_double2(a,b) _vector_double2(a,b)

VECTOR_TYPE_4( char, char )
#define __vector_char4(a,b,c,d) (__char4){a,b,c,d}
#define _vector_char4(a,b,c,d) (_char4){(__char4){a,b,c,d}}
#define vector_char4(a,b,c,d) _vector_char4(a,b,c,d)

VECTOR_TYPE_4( unsigned char, uchar )
#define __vector_uchar4(a,b,c,d) (__uchar4){a,b,c,d}
#define _vector_uchar4(a,b,c,d) (_uchar4){(__uchar4){a,b,c,d}}
#define vector_uchar4(a,b,c,d) _vector_uchar4(a,b,c,d)

VECTOR_TYPE_4( short, short )
#define __vector_short4(a,b,c,d) (__short4){a,b,c,d}
#define _vector_short4(a,b,c,d) (_short4){(__short4){a,b,c,d}}
#define vector_short4(a,b,c,d) _vector_short4(a,b,c,d)

VECTOR_TYPE_4( unsigned short, ushort )
#define __vector_ushort4(a,b,c,d) (__ushort4){a,b,c,d}
#define _vector_ushort4(a,b,c,d) (_ushort4){(__ushort4){a,b,c,d}}
#define vector_ushort4(a,b,c,d) _vector_ushort4(a,b,c,d)

VECTOR_TYPE_4( int, int )
#define __vector_int4(a,b,c,d) (__int4){a,b,c,d}
#define _vector_int4(a,b,c,d) (_int4){(__int4){a,b,c,d}}
#define vector_int4(a,b,c,d) _vector_int4(a,b,c,d)

VECTOR_TYPE_4( unsigned int, uint )
#define __vector_uint4(a,b,c,d) (__uint4){a,b,c,d}
#define _vector_uint4(a,b,c,d) (_uint4){(__uint4){a,b,c,d}}
#define vector_uint4(a,b,c,d) _vector_uint4(a,b,c,d)

VECTOR_TYPE_4( long long, long )
#define __vector_long4(a,b,c,d) (__long4){a,b,c,d}
#define _vector_long4(a,b,c,d) (_long4){(__long4){a,b,c,d}}
#define vector_long4(a,b,c,d) _vector_long4(a,b,c,d)

VECTOR_TYPE_4( unsigned long long, ulong )
#define __vector_ulong4(a,b,c,d) (__ulong4){a,b,c,d}
#define _vector_ulong4(a,b,c,d) (_ulong4){(__ulong4){a,b,c,d}}
#define vector_ulong4(a,b,c,d) _vector_ulong4(a,b,c,d)

VECTOR_TYPE_4( float, float )
#define __vector_float4(a,b,c,d) (__float4){a,b,c,d}
#define _vector_float4(a,b,c,d) (_float4){(__float4){a,b,c,d}}
#define vector_float4(a,b,c,d) _vector_float4(a,b,c,d)

VECTOR_TYPE_4( double, double )
#define __vector_double4(a,b,c,d) (__double4){a,b,c,d}
#define _vector_double4(a,b,c,d) (_double4){(__double4){a,b,c,d}}
#define vector_double4(a,b,c,d) _vector_double4(a,b,c,d)

VECTOR_TYPE_8( char, char )
#define __vector_char8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__char8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_char8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_char8){(__char8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_char8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_char8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_8( unsigned char, uchar )
#define __vector_uchar8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__uchar8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_uchar8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_uchar8){(__uchar8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_uchar8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_uchar8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_8( short, short )
#define __vector_short8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__short8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_short8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_short8){(__short8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_short8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_short8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_8( unsigned short, ushort )
#define __vector_ushort8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__ushort8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_ushort8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_ushort8){(__ushort8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_ushort8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_ushort8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_8( int, int )
#define __vector_int8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__int8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_int8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_int8){(__int8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_int8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_int8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_8( unsigned int, uint )
#define __vector_uint8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__uint8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_uint8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_uint8){(__uint8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_uint8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_uint8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_8( long long, long )
#define __vector_long8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__long8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_long8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_long8){(__long8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_long8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_long8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_8( unsigned long long, ulong )
#define __vector_ulong8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__ulong8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_ulong8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_ulong8){(__ulong8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_ulong8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_ulong8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_8( float, float )
#define __vector_float8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__float8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_float8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_float8){(__float8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_float8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_float8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_8( double, double )
#define __vector_double8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(__double8){s0,s1,s2,s3,s4,s5,s6,s7}
#define _vector_double8(s0,s1,s2,s3,s4,s5,s6,s7) \
	(_double8){(__double8){s0,s1,s2,s3,s4,s5,s6,s7}}
#define vector_double8(s0,s1,s2,s3,s4,s5,s6,s7) \
	_vector_double8(s0,s1,s2,s3,s4,s5,s6,s7)

VECTOR_TYPE_16( char, char )
#define __vector_char16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__char16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_char16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_char16){(__char16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_char16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_char16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)

VECTOR_TYPE_16( unsigned char, uchar )
#define __vector_uchar16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__uchar16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_uchar16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_uchar16){(__uchar16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_uchar16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_uchar16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)

VECTOR_TYPE_16( short, short )
#define __vector_short16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__short16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_short16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_short16){(__short16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_short16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_short16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)

VECTOR_TYPE_16( unsigned short, ushort )
#define __vector_ushort16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__ushort16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_ushort16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_ushort16){(__ushort16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_ushort16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_ushort16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)

VECTOR_TYPE_16( int, int )
#define __vector_int16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__int16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_int16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_int16){(__int16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_int16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_int16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)

VECTOR_TYPE_16( unsigned int, uint )
#define __vector_uint16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__uint16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_uint16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_uint16){(__uint16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_uint16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_uint16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)

VECTOR_TYPE_16( long long, long )
#define __vector_long16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__long16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_long16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_long16){(__long16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_long16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_long16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)

VECTOR_TYPE_16( unsigned long long, ulong )
#define __vector_ulong16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__ulong16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_ulong16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_ulong16){(__ulong16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_ulong16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_ulong16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)

VECTOR_TYPE_16( float, float )
#define __vector_float16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__float16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_float16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_float16){(__float16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_float16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_float16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)

VECTOR_TYPE_16( double, double )
#define __vector_double16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(__double16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}
#define _vector_double16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	(_double16){(__double16){s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf}}
#define vector_double16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf) \
	_vector_double16(s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf)


#define GENERIC_UNARY_PLUS_SIZE(vectype,size) \
static __always_inline _##vectype##size operator + ( _##vectype##size& a ); \
static __always_inline _##vectype##size operator + ( _##vectype##size& a ) { return a; }

#define GENERIC_UNARY_PLUS(vectype,noop) \
GENERIC_UNARY_PLUS_SIZE(vectype,2) \
GENERIC_UNARY_PLUS_SIZE(vectype,4) \
GENERIC_UNARY_PLUS_SIZE(vectype,8) \
GENERIC_UNARY_PLUS_SIZE(vectype,16)

#define GENERIC_UNARY_OP_SIZE(vectype,op,size) \
static __always_inline _##vectype##size operator op ( _##vectype##size& a ); \
static __always_inline _##vectype##size operator op ( _##vectype##size& a ) { return (_##vectype##size){ op a.vec }; }

#define GENERIC_UNARY_OP(vectype,op) \
GENERIC_UNARY_OP_SIZE(vectype,op,2) \
GENERIC_UNARY_OP_SIZE(vectype,op,4) \
GENERIC_UNARY_OP_SIZE(vectype,op,8) \
GENERIC_UNARY_OP_SIZE(vectype,op,16)

#define EXPLICIT_UNARY_OP_ALL_TYPE_SIZE(vectype,op) \
static __always_inline _##vectype##2 operator op ( _##vectype##2& a ); \
static __always_inline _##vectype##2 operator op ( _##vectype##2& a ) \
     { \
          _##vectype##2 tmp \
               = _vector_##vectype##2( (vectype##2::type_t)op a.s0,(vectype##2::type_t)op a.s1 ); \
          return tmp; \
     } \
static __always_inline _##vectype##4 operator op ( _##vectype##4& a ); \
static __always_inline _##vectype##4 operator op ( _##vectype##4& a ) \
     { \
          _##vectype##4 tmp = _vector_##vectype##4( (vectype##4::type_t)op a.s0, \
               (vectype##4::type_t)op a.s1,(vectype##4::type_t)op a.s2,(vectype##4::type_t)op a.s3); \
          return tmp; \
     } \
static __always_inline _##vectype##8 operator op ( _##vectype##8& a ); \
static __always_inline _##vectype##8 operator op ( _##vectype##8& a ) \
     { \
          _##vectype##8 tmp = _vector_##vectype##8( (vectype##8::type_t)op a.s0,(vectype##8::type_t)op a.s1, \
               (vectype##8::type_t)op a.s2,(vectype##8::type_t)op a.s3,(vectype##8::type_t)op a.s4, \
               (vectype##8::type_t)op a.s5,(vectype##8::type_t)op a.s6,(vectype##8::type_t)op a.s7); \
          return tmp; \
     } \
static __always_inline _##vectype##16 operator op ( _##vectype##16& a ); \
static __always_inline _##vectype##16 operator op ( _##vectype##16& a ) \
     { \
          _##vectype##16 tmp = _vector_##vectype##16( (vectype##16::type_t)op a.s0,(vectype##16::type_t)op a.s1, \
               (vectype##16::type_t)op a.s2,(vectype##16::type_t)op a.s3,(vectype##16::type_t)op a.s4, \
               (vectype##16::type_t)op a.s5,(vectype##16::type_t)op a.s6,(vectype##16::type_t)op a.s7, \
               (vectype##16::type_t)op a.s8,(vectype##16::type_t)op a.s9,(vectype##16::type_t)op a.sA, \
               (vectype##16::type_t)op a.sB,(vectype##16::type_t)op a.sC,(vectype##16::type_t)op a.sD, \
               (vectype##16::type_t)op a.sE,(vectype##16::type_t)op a.sF); \
          return tmp; \
     }

#define GENERIC_OP_ASSIGN_SIZE(vectype,op,size) \
static __always_inline _##vectype##size operator op##= ( _##vectype##size& lhs, const _##vectype##size rhs ); \
static __always_inline _##vectype##size operator op##= ( _##vectype##size& lhs, const _##vectype##size rhs ) { lhs.vec op##= rhs.vec; return lhs; }

#define GENERIC_OP_ASSIGN(vectype,op) \
GENERIC_OP_ASSIGN_SIZE(vectype,op,2) \
GENERIC_OP_ASSIGN_SIZE(vectype,op,4) \
GENERIC_OP_ASSIGN_SIZE(vectype,op,8) \
GENERIC_OP_ASSIGN_SIZE(vectype,op,16)

#define EXPLICIT_OP_ASSIGN_ALL_TYPE_SIZE(vectype,op) \
static __always_inline _##vectype##2 \
	operator op##= ( _##vectype##2& lhs, const _##vectype##2 rhs ); \
static __always_inline _##vectype##2 \
	operator op##= ( _##vectype##2& lhs, const _##vectype##2 rhs ) \
	{ \
		lhs.vec = __vector_##vectype##2( lhs.s0 op rhs.s0, lhs.s1 op rhs.s1 ); \
		return lhs; \
	} \
static __always_inline _##vectype##4 \
	operator op##= ( _##vectype##4& lhs, const _##vectype##4 rhs ); \
static __always_inline _##vectype##4 \
	operator op##= ( _##vectype##4& lhs, const _##vectype##4 rhs ) \
	{ \
		lhs.vec = __vector_##vectype##4( lhs.s0 op rhs.s0, lhs.s1 op rhs.s1, \
			lhs.s2 op rhs.s2, lhs.s3 op rhs.s3 ); \
		return lhs; \
	} \
static __always_inline _##vectype##8 \
	operator op##= ( _##vectype##8& lhs, const _##vectype##8 rhs ); \
static __always_inline _##vectype##8 \
	operator op##= ( _##vectype##8& lhs, const _##vectype##8 rhs ) \
	{ \
		lhs.vec = __vector_##vectype##8( lhs.s0 op rhs.s0, lhs.s1 op rhs.s1, \
			lhs.s2 op rhs.s2, lhs.s3 op rhs.s3, lhs.s4 op rhs.s4, \
			lhs.s5 op rhs.s5, lhs.s6 op rhs.s6, lhs.s7 op rhs.s7 ); \
		return lhs; \
	} \
static __always_inline _##vectype##16 \
	operator op##= ( _##vectype##16& lhs, const _##vectype##16 rhs ); \
static __always_inline _##vectype##16 \
	operator op##= ( _##vectype##16& lhs, const _##vectype##16 rhs ) \
	{ \
		lhs.vec = __vector_##vectype##16( lhs.s0 op rhs.s0, lhs.s1 op rhs.s1, \
			lhs.s2 op rhs.s2, lhs.s3 op rhs.s3, lhs.s4 op rhs.s4, \
			lhs.s5 op rhs.s5, lhs.s6 op rhs.s6, lhs.s7 op rhs.s7, \
			lhs.s8 op rhs.s8, lhs.s9 op rhs.s9, lhs.sA op rhs.sA, \
			lhs.sB op rhs.sB, lhs.sC op rhs.sC, lhs.sD op rhs.sD, \
			lhs.sE op rhs.sE, lhs.sF op rhs.sF); \
		return lhs; \
	}

#define INT_TYPE(m,...) \
m(char,__VA_ARGS__) \
m(uchar,__VA_ARGS__) \
m(short,__VA_ARGS__) \
m(ushort,__VA_ARGS__) \
m(int,__VA_ARGS__) \
m(uint,__VA_ARGS__) \
m(long,__VA_ARGS__) \
m(ulong,__VA_ARGS__)

#define ALL_TYPE(m,...) \
INT_TYPE(m,__VA_ARGS__) \
m(float,__VA_ARGS__) \
m(double,__VA_ARGS__)

/* unary plus */
ALL_TYPE(GENERIC_UNARY_PLUS)

#if GCC_VERSION < 40600
ALL_TYPE(EXPLICIT_UNARY_OP_ALL_TYPE_SIZE,-) /* unary minus */
ALL_TYPE(EXPLICIT_OP_ASSIGN_ALL_TYPE_SIZE,+) /* add assign */
ALL_TYPE(EXPLICIT_OP_ASSIGN_ALL_TYPE_SIZE,-) /* subtract assign */
ALL_TYPE(EXPLICIT_OP_ASSIGN_ALL_TYPE_SIZE,*) /* multiply assign */
ALL_TYPE(EXPLICIT_OP_ASSIGN_ALL_TYPE_SIZE,/) /* divide assign */
#else
ALL_TYPE(GENERIC_UNARY_OP,-) /* unary minus */
ALL_TYPE(GENERIC_OP_ASSIGN,+) /* add assign */
ALL_TYPE(GENERIC_OP_ASSIGN,-) /* subtract assign */
ALL_TYPE(GENERIC_OP_ASSIGN,*) /* multiply assign */
ALL_TYPE(GENERIC_OP_ASSIGN,/) /* divide assign */
#endif

INT_TYPE(EXPLICIT_UNARY_OP_ALL_TYPE_SIZE,~) /* unary bitwise not */
INT_TYPE(EXPLICIT_OP_ASSIGN_ALL_TYPE_SIZE,%) /* modulo assign */
INT_TYPE(EXPLICIT_OP_ASSIGN_ALL_TYPE_SIZE,&) /* bitwise and assign */
INT_TYPE(EXPLICIT_OP_ASSIGN_ALL_TYPE_SIZE,|) /* bitwise or assign */
INT_TYPE(EXPLICIT_OP_ASSIGN_ALL_TYPE_SIZE,^) /* bitwise xor assign */


/*** other builtin data types [6.1.3] ***/

#define image2d_t __global void*


/*** conversions and type casting [6.2] ***/

#define convert_char(x) static_cast<char>(x) 
#define convert_uchar(x) static_cast<unsigned char>(x) 
#define convert_short(x) static_cast<short>(x) 
#define convert_ushort(x) static_cast<unsigned short>(x) 
#define convert_int(x) static_cast<int>(x) 
#define convert_uint(x) static_cast<unsigned int>(x) 
#define convert_long(x) static_cast<long>(x) 
#define convert_ulong(x) static_cast<unsigned long>(x) 
#define convert_float(x) static_cast<float>(x) 
#define convert_double(x) static_cast<double>(x) 

#define convert_char2(x) static_cast<char2>(x) 
#define convert_uchar2(x) static_cast<uchar2>(x) 
#define convert_short2(x) static_cast<short2>(x) 
#define convert_ushort2(x) static_cast<ushort2>(x) 
#define convert_int2(x) static_cast<int2>(x) 
#define convert_uint2(x) static_cast<uint2>(x) 
#define convert_long2(x) static_cast<long2>(x) 
#define convert_ulong2(x) static_cast<ulong2>(x) 
#define convert_float2(x) static_cast<float2>(x) 
#define convert_double2(x) static_cast<double2>(x) 

#define convert_char4(x) static_cast<char4>(x)
#define convert_uchar4(x) static_cast<uchar4>(x)       
#define convert_short4(x) static_cast<short4>(x)
#define convert_ushort4(x) static_cast<ushort4>(x)
#define convert_int4(x) static_cast<int4>(x)
#define convert_uint4(x) static_cast<uint4>(x)       
#define convert_long4(x) static_cast<long4>(x)
#define convert_ulong4(x) static_cast<ulong4>(x)       
#define convert_float4(x) static_cast<float4>(x)
#define convert_double4(x) static_cast<double4>(x)

#define convert_char8(x) static_cast<char8>(x)
#define convert_uchar8(x) static_cast<uchar8>(x)       
#define convert_short8(x) static_cast<short8>(x)
#define convert_ushort8(x) static_cast<ushort8>(x)
#define convert_int8(x) static_cast<int8>(x)
#define convert_uint8(x) static_cast<uint8>(x)       
#define convert_long8(x) static_cast<long8>(x)
#define convert_ulong8(x) static_cast<ulong8>(x)       
#define convert_float8(x) static_cast<float8>(x)
#define convert_double8(x) static_cast<double8>(x)

#define convert_char16(x) static_cast<char16>(x)
#define convert_uchar16(x) static_cast<uchar16>(x)
#define convert_short16(x) static_cast<short16>(x)
#define convert_ushort16(x) static_cast<ushort16>(x)
#define convert_int16(x) static_cast<int16>(x)
#define convert_uint16(x) static_cast<uint16>(x)
#define convert_long16(x) static_cast<long16>(x)
#define convert_ulong16(x) static_cast<ulong16>(x)     
#define convert_float16(x) static_cast<float16>(x)
#define convert_double16(x) static_cast<double16>(x)


#define AS_TYPE(type) \
template < typename T > \
static __always_inline type as_##type( T x ) { return *(type*)(&x); }

AS_TYPE(char2)
AS_TYPE(uchar2)
AS_TYPE(short2)
AS_TYPE(ushort2)
AS_TYPE(int2)
AS_TYPE(uint2)
AS_TYPE(long2)
AS_TYPE(ulong2)
AS_TYPE(float2)
AS_TYPE(double2)
AS_TYPE(char4)
AS_TYPE(uchar4)
AS_TYPE(short4)
AS_TYPE(ushort4)
AS_TYPE(int4)
AS_TYPE(uint4)
AS_TYPE(long4)
AS_TYPE(ulong4)
AS_TYPE(float4)
AS_TYPE(double4)
AS_TYPE(char8)
AS_TYPE(uchar8)
AS_TYPE(short8)
AS_TYPE(ushort8)
AS_TYPE(int8)
AS_TYPE(uint8)
AS_TYPE(long8)
AS_TYPE(ulong8)
AS_TYPE(float8)
AS_TYPE(double8)
AS_TYPE(char16)
AS_TYPE(uchar16)
AS_TYPE(short16)
AS_TYPE(ushort16)
AS_TYPE(int16)
AS_TYPE(uint16)
AS_TYPE(long16)
AS_TYPE(ulong16)
AS_TYPE(float16)
AS_TYPE(double16)


/*** 
 *** operators for vector data types [6.3] 
 ***/

#define GENERIC_BINOP_ALL_TYPE_SIZE(vectype,type,op) \
static __always_inline _##vectype##2 \
	operator op ( const _##vectype##2 a, const _##vectype##2 b ); \
static __always_inline _##vectype##2 \
	operator op ( const _##vectype##2 a, const _##vectype##2 b ) \
	{ return (_##vectype##2){ a.vec op b.vec }; } \
static __always_inline _##vectype##2 \
	operator op ( type a, const _##vectype##2 b ); \
static __always_inline _##vectype##2 \
	operator op ( type a, const _##vectype##2 b ) \
	{ return (_##vectype##2){ __vector_##vectype##2(a,a) op b.vec }; } \
static __always_inline _##vectype##2 \
	operator op ( const _##vectype##2 a, type b ); \
static __always_inline _##vectype##2 \
	operator op ( const _##vectype##2 a, type b ) \
	{ return (_##vectype##2){ a.vec op __vector_##vectype##2(b,b) }; } \
static __always_inline _##vectype##4 \
	operator op ( const _##vectype##4 a, const _##vectype##4 b ); \
static __always_inline _##vectype##4 \
	operator op ( const _##vectype##4 a, const _##vectype##4 b ) \
	{ return (_##vectype##4){ a.vec op b.vec }; } \
static __always_inline _##vectype##4 \
	operator op ( type a, const _##vectype##4 b ); \
static __always_inline _##vectype##4 \
	operator op ( type a, const _##vectype##4 b ) \
	{ return (_##vectype##4){ __vector_##vectype##4(a,a,a,a) op b.vec }; } \
static __always_inline _##vectype##4 \
	operator op ( const _##vectype##4 a, type b ); \
static __always_inline _##vectype##4 \
	operator op ( const _##vectype##4 a, type b ) \
	{ return (_##vectype##4){ a.vec op __vector_##vectype##4(b,b,b,b) }; } \
static __always_inline _##vectype##8 \
	operator op ( const _##vectype##8 a, const _##vectype##8 b ); \
static __always_inline _##vectype##8 \
	operator op ( const _##vectype##8 a, const _##vectype##8 b ) \
	{ return (_##vectype##8){ a.vec op b.vec }; } \
static __always_inline _##vectype##8 \
	operator op ( type a, const _##vectype##8 b ); \
static __always_inline _##vectype##8 \
	operator op ( type a, const _##vectype##8 b ) \
	{ return (_##vectype##8){ __vector_##vectype##8(a,a,a,a,a,a,a,a) op b.vec }; } \
static __always_inline _##vectype##8 \
	operator op ( const _##vectype##8 a, type b ); \
static __always_inline _##vectype##8 \
	operator op ( const _##vectype##8 a, type b ) \
	{ return (_##vectype##8){ a.vec op __vector_##vectype##8(b,b,b,b,b,b,b,b) }; } \
static __always_inline _##vectype##16 \
	operator op ( const _##vectype##16 a, const _##vectype##16 b ); \
static __always_inline _##vectype##16 \
	operator op ( const _##vectype##16 a, const _##vectype##16 b ) \
	{ return (_##vectype##16){ a.vec op b.vec }; } \
static __always_inline _##vectype##16 \
	operator op ( type a, const _##vectype##16 b ); \
static __always_inline _##vectype##16 \
	operator op ( type a, const _##vectype##16 b ) \
	{ return (_##vectype##16){ __vector_##vectype##16(a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a) op b.vec }; } \
static __always_inline _##vectype##16 \
	operator op ( const _##vectype##16 a, type b ); \
static __always_inline _##vectype##16 \
	operator op ( const _##vectype##16 a, type b ) \
	{ return (_##vectype##16){ a.vec op __vector_##vectype##16(b,b,b,b,b,b,b,b,b,b,b,b,b,b,b,b) }; } 

#define EXPLICIT_BINOP_ALL_TYPE_SIZE(vectype,type,op) \
static __always_inline _##vectype##2 \
	operator op ( const _##vectype##2 a, const _##vectype##2 b ); \
static __always_inline _##vectype##2 \
	operator op ( const _##vectype##2 a, const _##vectype##2 b ) \
	{ \
		_##vectype##2 tmp = { a.s0 op b.s0, a.s1 op b.s1 }; \
		return tmp; \
	} \
static __always_inline _##vectype##2 \
	operator op ( type a, const _##vectype##2 b ); \
static __always_inline _##vectype##2 \
	operator op ( type a, const _##vectype##2 b ) \
	{ \
		_##vectype##2 tmp = { a op b.s0, a op b.s1 }; \
		return tmp; \
	} \
static __always_inline _##vectype##2 \
	operator op ( const _##vectype##2 a, type b ); \
static __always_inline _##vectype##2 \
	operator op ( const _##vectype##2 a, type b ) \
	{ \
		_##vectype##2 tmp = { a.s0 op b, a.s1 op b }; \
		return tmp; \
	} \
static __always_inline _##vectype##4 \
	operator op ( const _##vectype##4 a, const _##vectype##4 b ); \
static __always_inline _##vectype##4 \
	operator op ( const _##vectype##4 a, const _##vectype##4 b ) \
	{ \
		_##vectype##4 tmp={a.s0 op b.s0,a.s1 op b.s1,a.s2 op b.s2,a.s3 op b.s3}; \
		return tmp; \
	} \
static __always_inline _##vectype##4 operator op ( type a, const _##vectype##4 b );\
static __always_inline _##vectype##4 \
	operator op ( type a, const _##vectype##4 b ) \
	{ \
		_##vectype##4 tmp={a op b.s0,a op b.s1,a op b.s2,a op b.s3}; \
		return tmp; \
	} \
static __always_inline _##vectype##4 operator op ( const _##vectype##4 a, type b ); \
static __always_inline _##vectype##4 \
	operator op ( const _##vectype##4 a, type b ) \
	{ \
		_##vectype##4 tmp={a.s0 op b,a.s1 op b,a.s2 op b,a.s3 op b}; \
		return tmp; \
	} \
static __always_inline _##vectype##8 \
	operator op ( const _##vectype##8 a, const _##vectype##8 b ); \
static __always_inline _##vectype##8 \
	operator op ( const _##vectype##8 a, const _##vectype##8 b ) \
	{ \
		_##vectype##8 tmp={a.s0 op b.s0,a.s1 op b.s1,a.s2 op b.s2,a.s3 op b.s3, \
			a.s4 op b.s4,a.s5 op b.s5,a.s6 op b.s6,a.s7 op b.s7}; \
		return tmp; \
	} \
static __always_inline _##vectype##8 operator op ( type a, const _##vectype##8 b );\
static __always_inline _##vectype##8 \
	operator op ( type a, const _##vectype##8 b ) \
	{ \
		_##vectype##8 tmp={a op b.s0,a op b.s1,a op b.s2,a op b.s3, \
			a op b.s4,a op b.s5,a op b.s6,a op b.s7}; \
		return tmp; \
	} \
static __always_inline _##vectype##8 operator op ( const _##vectype##8 a, type b );\
static __always_inline _##vectype##8 \
	operator op ( const _##vectype##8 a, type b ) \
	{ \
		_##vectype##8 tmp={a.s0 op b,a.s1 op b,a.s2 op b,a.s3 op b, \
			a.s4 op b,a.s5 op b,a.s6 op b,a.s7 op b}; \
		return tmp; \
	} \
static __always_inline _##vectype##16 \
	operator op ( const _##vectype##16 a, const _##vectype##16 b ); \
static __always_inline _##vectype##16 \
	operator op ( const _##vectype##16 a, const _##vectype##16 b ) \
	{ \
		_##vectype##16 tmp={a.s0 op b.s0,a.s1 op b.s1,a.s2 op b.s2,a.s3 op b.s3, \
			a.s4 op b.s4,a.s5 op b.s5,a.s6 op b.s6,a.s7 op b.s7, \
			a.s8 op b.s8,a.s9 op b.s9,a.sA op b.sA,a.sB op b.sB, \
			a.sC op b.sC,a.sD op b.sD,a.sE op b.sE,a.sF op b.sF}; \
		return tmp; \
	} \
static __always_inline _##vectype##16 operator op ( type a, const _##vectype##16 b );\
static __always_inline _##vectype##16 \
	operator op ( type a, const _##vectype##16 b ) \
	{ \
		_##vectype##16 tmp={a op b.s0,a op b.s1,a op b.s2,a op b.s3, \
			a op b.s4,a op b.s5,a op b.s6,a op b.s7, \
			a op b.s8,a op b.s9,a op b.sA,a op b.sB, \
			a op b.sC,a op b.sD,a op b.sE,a op b.sF}; \
		return tmp; \
	}\
static __always_inline _##vectype##16 operator op ( const _##vectype##16 a, type b );\
static __always_inline _##vectype##16 \
	operator op ( const _##vectype##16 a, type b ) \
	{ \
		_##vectype##16 tmp={a.s0 op b,a.s1 op b,a.s2 op b,a.s3 op b, \
			a.s4 op b,a.s5 op b,a.s6 op b,a.s7 op b, \
			a.s8 op b,a.s9 op b,a.sA op b,a.sB op b, \
			a.sC op b,a.sD op b,a.sE op b,a.sF op b}; \
		return tmp; \
	} 

#define INT_TYPE2(m,...) \
m(char,char,__VA_ARGS__) \
m(uchar,unsigned char,__VA_ARGS__) \
m(short,short,__VA_ARGS__) \
m(ushort,unsigned short,__VA_ARGS__) \
m(int,int,__VA_ARGS__) \
m(uint,unsigned int,__VA_ARGS__) \
m(long,long long int,__VA_ARGS__) \
m(ulong,unsigned long long int,__VA_ARGS__)

#define ALL_TYPE2(m,...) \
INT_TYPE2(m,__VA_ARGS__) \
m(float,float,__VA_ARGS__) \
m(double,float,__VA_ARGS__)

#if GCC_VERSION < 40600
ALL_TYPE2(EXPLICIT_BINOP_ALL_TYPE_SIZE,+) /* binary add */
ALL_TYPE2(EXPLICIT_BINOP_ALL_TYPE_SIZE,-) /* binary sub */
ALL_TYPE2(EXPLICIT_BINOP_ALL_TYPE_SIZE,*) /* binary mult */
INT_TYPE2(EXPLICIT_BINOP_ALL_TYPE_SIZE,%) /* binary mod */
ALL_TYPE2(EXPLICIT_BINOP_ALL_TYPE_SIZE,/) /* binary div */
INT_TYPE2(EXPLICIT_BINOP_ALL_TYPE_SIZE,&) /* binary and */
INT_TYPE2(EXPLICIT_BINOP_ALL_TYPE_SIZE,|) /* binary or */
INT_TYPE2(EXPLICIT_BINOP_ALL_TYPE_SIZE,^) /* binary xor */
#else
ALL_TYPE2(GENERIC_BINOP_ALL_TYPE_SIZE,+) /* binary add */
ALL_TYPE2(GENERIC_BINOP_ALL_TYPE_SIZE,-) /* binary sub */
ALL_TYPE2(GENERIC_BINOP_ALL_TYPE_SIZE,*) /* binary mult */
INT_TYPE2(GENERIC_BINOP_ALL_TYPE_SIZE,%) /* binary mod */
ALL_TYPE2(GENERIC_BINOP_ALL_TYPE_SIZE,/) /* binary div */
INT_TYPE2(GENERIC_BINOP_ALL_TYPE_SIZE,&) /* binary and */
INT_TYPE2(GENERIC_BINOP_ALL_TYPE_SIZE,|) /* binary or */
INT_TYPE2(GENERIC_BINOP_ALL_TYPE_SIZE,^) /* binary xor */
#endif


/*** these are generic min and max definitions - they should be corrected ***/

#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a<b)?b:a)


/*** math builtin functions [6.11.2] ***/

#define __MATH_BUILTIN_1(func) \
static __always_inline float func##_T( float a ); \
static __always_inline float func##_T( float a ) \
	{ return func##f(a); } \
static __always_inline float2 func##_T( float2 a ); \
static __always_inline float2 func##_T( float2 a ) \
	{ return vector_float2( func##f(a.x),func##f(a.y) ); } \
static __always_inline float4 func##_T( float4 a ); \
static __always_inline float4 func##_T( float4 a ) \
	{ \
		return vector_float4( \
			func##f(a.x),func##f(a.y),func##f(a.z),func##f(a.w)); \
	} \
static __always_inline float8 func##_T( float8 a ); \
static __always_inline float8 func##_T( float8 a ) \
	{ \
		return vector_float8( \
			func##f(a.s0),func##f(a.s1),func##f(a.s2),func##f(a.s3), \
			func##f(a.s4),func##f(a.s5),func##f(a.s6),func##f(a.s7)); \
	} \
static __always_inline float16 func##_T( float16 a ); \
static __always_inline float16 func##_T( float16 a ) \
	{ \
		return vector_float16( \
			func##f(a.s0),func##f(a.s1),func##f(a.s2),func##f(a.s3), \
			func##f(a.s4),func##f(a.s5),func##f(a.s6),func##f(a.s7), \
			func##f(a.s8),func##f(a.s9),func##f(a.sA),func##f(a.sB), \
			func##f(a.sC),func##f(a.sD),func##f(a.sE),func##f(a.sF)); \
	} \
static __always_inline double func##_T( double a ); \
static __always_inline double func##_T( double a ) \
	{ return func(a); } \
static __always_inline double2 func##_T( double2 a ); \
static __always_inline double2 func##_T( double2 a ) \
	{ return vector_double2( func(a.x),func(a.y) ); } \
static __always_inline double4 func##_T( double4 a ); \
static __always_inline double4 func##_T( double4 a ) \
	{ \
		return vector_double4( \
			func(a.x),func(a.y),func(a.z),func(a.w)); \
	} \
static __always_inline double8 func##_T( double8 a ) \
	{ \
		return vector_double8( \
			func(a.s0),func(a.s1),func(a.s2),func(a.s3), \
			func(a.s4),func(a.s5),func(a.s6),func(a.s7)); \
	} \
static __always_inline double16 func##_T( double16 a ) \
	{ \
		return vector_double16( \
			func(a.s0),func(a.s1),func(a.s2),func(a.s3), \
			func(a.s4),func(a.s5),func(a.s6),func(a.s7), \
			func(a.s8),func(a.s9),func(a.sA),func(a.sB), \
			func(a.sC),func(a.sD),func(a.sE),func(a.sF)); \
	}

#define __MATH_BUILTIN_2(func) \
static __always_inline float func##_T( float a, float b ); \
static __always_inline float func##_T( float a, float b ) \
	{ return func##f(a,b); } \
static __always_inline float2 func##_T( float2 a, float2 b ); \
static __always_inline float2 func##_T( float2 a, float2 b ) \
	{ return vector_float2( func##f(a.s0,b.s0),func##f(a.s1,b.s1) ); } \
static __always_inline float4 func##_T( float4 a, float4 b ); \
static __always_inline float4 func##_T( float4 a, float4 b ) \
	{ \
		return vector_float4( \
			func##f(a.s0,b.s0),func##f(a.s1,b.s1),func##f(a.s2,b.s2),func##f(a.s3,b.s3)); \
	} \
static __always_inline float8 func##_T( float8 a, float8 b ); \
static __always_inline float8 func##_T( float8 a, float8 b ) \
	{ \
		return vector_float8( \
			func##f(a.s0,b.s0),func##f(a.s1,b.s1),func##f(a.s2,b.s2),func##f(a.s3,b.s3), \
			func##f(a.s4,b.s4),func##f(a.s5,b.s5),func##f(a.s6,b.s6),func##f(a.s7,b.s7)); \
	} \
static __always_inline float16 func##_T( float16 a, float16 b ); \
static __always_inline float16 func##_T( float16 a, float16 b ) \
	{ \
		return vector_float16( \
			func##f(a.s0,b.s0),func##f(a.s1,b.s1),func##f(a.s2,b.s2),func##f(a.s3,b.s3), \
			func##f(a.s4,b.s4),func##f(a.s5,b.s5),func##f(a.s6,b.s6),func##f(a.s7,b.s7), \
			func##f(a.s8,b.s8),func##f(a.s9,b.s9),func##f(a.sA,b.sA),func##f(a.sB,b.sB), \
			func##f(a.sC,b.sC),func##f(a.sD,b.sD),func##f(a.sE,b.sE),func##f(a.sF,b.sF)); \
	} \
static __always_inline double func##_T( double a, double b ); \
static __always_inline double func##_T( double a, double b ) \
	{ return func(a,b); } \
static __always_inline double2 func##_T( double2 a, double2 b ); \
static __always_inline double2 func##_T( double2 a, double2 b ) \
	{ return vector_double2( func(a.s0,b.s0),func(a.s1,b.s1) ); } \
static __always_inline double4 func##_T( double4 a, double4 b ); \
static __always_inline double4 func##_T( double4 a, double4 b ) \
	{ \
		return vector_double4( \
			func(a.s0,b.s0),func(a.s1,b.s1),func(a.s2,b.s2),func(a.s3,b.s3)); \
	} \
static __always_inline double8 func##_T( double8 a, double8 b ); \
static __always_inline double8 func##_T( double8 a, double8 b ) \
	{ \
		return vector_double8( \
			func(a.s0,b.s0),func(a.s1,b.s1),func(a.s2,b.s2),func(a.s3,b.s3), \
			func(a.s4,b.s4),func(a.s5,b.s5),func(a.s6,b.s6),func(a.s7,b.s7)); \
	} \
static __always_inline double16 func##_T( double16 a, double16 b ); \
static __always_inline double16 func##_T( double16 a, double16 b ) \
	{ \
		return vector_double16( \
			func(a.s0,b.s0),func(a.s1,b.s1),func(a.s2,b.s2),func(a.s3,b.s3), \
			func(a.s4,b.s4),func(a.s5,b.s5),func(a.s6,b.s6),func(a.s7,b.s7), \
			func(a.s8,b.s8),func(a.s9,b.s9),func(a.sA,b.sA),func(a.sB,b.sB), \
			func(a.sC,b.sC),func(a.sD,b.sD),func(a.sE,b.sE),func(a.sF,b.sF)); \
	}

#define __MATH_BUILTIN_3(func) \
static __always_inline float func##_T( float a, float b, float c); \
static __always_inline float func##_T( float a, float b, float c ) \
	{ return func##f(a,b,c); } \
static __always_inline float2 func##_T( float2 a, float2 b, float2 c ); \
static __always_inline float2 func##_T( float2 a, float2 b, float2 c ) \
	{ return vector_float2( func##f(a.s0,b.s0,c.s0),func##f(a.s1,b.s1,c.s1) ); } \
static __always_inline float4 func##_T( float4 a, float4 b, float4 c ); \
static __always_inline float4 func##_T( float4 a, float4 b, float4 c ) \
	{ \
		return vector_float4( \
			func##f(a.s0,b.s0,c.s0),func##f(a.s1,b.s1,c.s1),func##f(a.s2,b.s2,c.s2),func##f(a.s3,b.s3,c.s3)); \
	} \
static __always_inline float8 func##_T( float8 a, float8 b, float8 c ); \
static __always_inline float8 func##_T( float8 a, float8 b, float8 c ) \
	{ \
		return vector_float8( \
			func##f(a.s0,b.s0,c.s0),func##f(a.s1,b.s1,c.s1),func##f(a.s2,b.s2,c.s2),func##f(a.s3,b.s3,c.s3), \
			func##f(a.s4,b.s4,c.s4),func##f(a.s5,b.s5,c.s5),func##f(a.s6,b.s6,c.s6),func##f(a.s7,b.s7,c.s7)); \
	} \
static __always_inline float16 func##_T( float16 a, float16 b, float16 c ); \
static __always_inline float16 func##_T( float16 a, float16 b, float16 c ) \
	{ \
		return vector_float16( \
			func##f(a.s0,b.s0,c.s0),func##f(a.s1,b.s1,c.s1),func##f(a.s2,b.s2,c.s2),func##f(a.s3,b.s3,c.s3), \
			func##f(a.s4,b.s4,c.s4),func##f(a.s5,b.s5,c.s5),func##f(a.s6,b.s6,c.s6),func##f(a.s7,b.s7,c.s7), \
			func##f(a.s8,b.s8,c.s8),func##f(a.s9,b.s9,c.s9),func##f(a.sA,b.sA,c.sA),func##f(a.sB,b.sB,c.sB), \
			func##f(a.sC,b.sC,c.sC),func##f(a.sD,b.sD,c.sD),func##f(a.sE,b.sE,c.sE),func##f(a.sF,b.sF,c.sF)); \
	} \
static __always_inline double func##_T( double a, double b, double c ); \
static __always_inline double func##_T( double a, double b, double c ) \
	{ return func(a,b,c); } \
static __always_inline double2 func##_T( double2 a, double2 b, double2 c ); \
static __always_inline double2 func##_T( double2 a, double2 b, double2 c ) \
	{ return vector_double2( func(a.s0,b.s0,c.s0),func(a.s1,b.s1,c.s1) ); } \
static __always_inline double4 func##_T( double4 a, double4 b, double4 c ); \
static __always_inline double4 func##_T( double4 a, double4 b, double4 c ) \
	{ \
		return vector_double4( \
			func(a.s0,b.s0,c.s0),func(a.s1,b.s1,c.s1),func(a.s2,b.s2,c.s2),func(a.s3,b.s3,c.s3)); \
	} \
static __always_inline double8 func##_T( double8 a, double8 b, double8 c ); \
static __always_inline double8 func##_T( double8 a, double8 b, double8 c ) \
	{ \
		return vector_double8( \
			func(a.s0,b.s0,c.s0),func(a.s1,b.s1,c.s1),func(a.s2,b.s2,c.s2),func(a.s3,b.s3,c.s3), \
			func(a.s4,b.s4,c.s4),func(a.s5,b.s5,c.s5),func(a.s6,b.s6,c.s6),func(a.s7,b.s7,c.s7)); \
	} \
static __always_inline double16 func##_T( double16 a, double16 b, double16 c ); \
static __always_inline double16 func##_T( double16 a, double16 b, double16 c ) \
	{ \
		return vector_double16( \
			func(a.s0,b.s0,c.s0),func(a.s1,b.s1,c.s1),func(a.s2,b.s2,c.s2),func(a.s3,b.s3,c.s3), \
			func(a.s4,b.s4,c.s4),func(a.s5,b.s5,c.s5),func(a.s6,b.s6,c.s6),func(a.s7,b.s7,c.s7), \
			func(a.s8,b.s8,c.s8),func(a.s9,b.s9,c.s9),func(a.sA,b.sA,c.sA),func(a.sB,b.sB,c.sB), \
			func(a.sC,b.sC,c.sC),func(a.sD,b.sD,c.sD),func(a.sE,b.sE,c.sE),func(a.sF,b.sF,c.sF)); \
	}


__MATH_BUILTIN_1(acos)
__MATH_BUILTIN_1(acosh)
static __always_inline float acospif( float a );
static __always_inline float acospif( float a ) { return acosf(a)*M_1_PI_F; }
static __always_inline double acospi( double a );
static __always_inline double acospi( double a ) { return acos(a)*M_1_PI; }
__MATH_BUILTIN_1(acospi)
__MATH_BUILTIN_1(asin)
__MATH_BUILTIN_1(asinh)
static __always_inline float asinpif( float a );
static __always_inline float asinpif( float a ) { return asinf(a)*M_1_PI_F; }
static __always_inline double asinpi( double a );
static __always_inline double asinpi( double a ) { return asin(a)*M_1_PI; }
__MATH_BUILTIN_1(asinpi)
__MATH_BUILTIN_1(atan)
__MATH_BUILTIN_2(atan2)
__MATH_BUILTIN_1(atanh)
static __always_inline float atanpif( float a );
static __always_inline float atanpif( float a ) { return atanf(a)*M_1_PI_F; }
static __always_inline double atanpi( double a );
static __always_inline double atanpi( double a ) { return atan(a)*M_1_PI; }
__MATH_BUILTIN_1(atanpi)
static __always_inline float atan2pif( float a, float b );
static __always_inline float atan2pif( float a, float b ) { return atan2f(a,b)*M_1_PI_F; }
static __always_inline double atan2pi( double a, double b );
static __always_inline double atan2pi( double a, double b ) { return atan2(a,b)*M_1_PI; }
__MATH_BUILTIN_2(atan2pi)
__MATH_BUILTIN_1(cbrt)
__MATH_BUILTIN_1(ceil)
__MATH_BUILTIN_2(copysign)
__MATH_BUILTIN_1(cos)
__MATH_BUILTIN_1(cosh)
static __always_inline float cospif( float a );
static __always_inline float cospif( float a ) { return cosf(a*M_PI_F); }
static __always_inline double cospi( double a );
static __always_inline double cospi( double a ) { return cos(a*M_PI); }
__MATH_BUILTIN_1(cospi)
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
__MATH_BUILTIN_2(fdim)
__MATH_BUILTIN_1(floor)
__MATH_BUILTIN_3(fma)
__MATH_BUILTIN_2(fmax);
static __always_inline float2 fmax_T( float2 a, float b );
static __always_inline float2 fmax_T( float2 a, float b ) { return fmax_T(a,vector_float2(b,b)); }
static __always_inline float4 fmax_T( float4 a, float b );
static __always_inline float4 fmax_T( float4 a, float b ) { return fmax_T(a,vector_float4(b,b,b,b)); }
static __always_inline float8 fmax_T( float8 a, float b );
static __always_inline float8 fmax_T( float8 a, float b ) { return fmax_T(a,vector_float8(b,b,b,b,b,b,b,b)); }
static __always_inline float16 fmax_T( float16 a, float b );
static __always_inline float16 fmax_T( float16 a, float b ) { return fmax_T(a,vector_float16(b,b,b,b,b,b,b,b,b,b,b,b,b,b,b,b)); }
static __always_inline double2 fmax_T( double2 a, double b );
static __always_inline double2 fmax_T( double2 a, double b ) { return fmax_T(a,vector_double2(b,b)); }
static __always_inline double4 fmax_T( double4 a, double b );
static __always_inline double4 fmax_T( double4 a, double b ) { return fmax_T(a,vector_double4(b,b,b,b)); }
static __always_inline double8 fmax_T( double8 a, double b );
static __always_inline double8 fmax_T( double8 a, double b ) { return fmax_T(a,vector_double8(b,b,b,b,b,b,b,b)); }
static __always_inline double16 fmax_T( double16 a, double b );
static __always_inline double16 fmax_T( double16 a, double b ) { return fmax_T(a,vector_double16(b,b,b,b,b,b,b,b,b,b,b,b,b,b,b,b)); }
__MATH_BUILTIN_2(fmin);
static __always_inline float2 fmin_T( float2 a, float b );
static __always_inline float2 fmin_T( float2 a, float b ) { return fmin_T(a,vector_float2(b,b)); }
static __always_inline float4 fmin_T( float4 a, float b );
static __always_inline float4 fmin_T( float4 a, float b ) { return fmin_T(a,vector_float4(b,b,b,b)); }
static __always_inline float8 fmin_T( float8 a, float b );
static __always_inline float8 fmin_T( float8 a, float b ) { return fmin_T(a,vector_float8(b,b,b,b,b,b,b,b)); }
static __always_inline float16 fmin_T( float16 a, float b );
static __always_inline float16 fmin_T( float16 a, float b ) { return fmin_T(a,vector_float16(b,b,b,b,b,b,b,b,b,b,b,b,b,b,b,b)); }
static __always_inline double2 fmin_T( double2 a, double b );
static __always_inline double2 fmin_T( double2 a, double b ) { return fmin_T(a,vector_double2(b,b)); }
static __always_inline double4 fmin_T( double4 a, double b );
static __always_inline double4 fmin_T( double4 a, double b ) { return fmin_T(a,vector_double4(b,b,b,b)); }
static __always_inline double8 fmin_T( double8 a, double b );
static __always_inline double8 fmin_T( double8 a, double b ) { return fmin_T(a,vector_double8(b,b,b,b,b,b,b,b)); }
static __always_inline double16 fmin_T( double16 a, double b );
static __always_inline double16 fmin_T( double16 a, double b ) { return fmin_T(a,vector_double16(b,b,b,b,b,b,b,b,b,b,b,b,b,b,b,b)); }
__MATH_BUILTIN_2(fmod)
static __always_inline float fract_T( float a, float* b );
static __always_inline float fract_T( float a, float* b ) { *b = floor_T(a); return fmin_T(a-floor_T(a), 0x1.fffffep-1f); }
static __always_inline float2 fract_T( float2 a, float2* b );
static __always_inline float2 fract_T( float2 a, float2* b ) { *b = floor_T(a); return fmin_T(a-floor_T(a), 0x1.fffffep-1f); }
static __always_inline float4 fract_T( float4 a, float4* b );
static __always_inline float4 fract_T( float4 a, float4* b ) { *b = floor_T(a); return fmin_T(a-floor_T(a), 0x1.fffffep-1f); }
static __always_inline float8 fract_T( float8 a, float8* b );
static __always_inline float8 fract_T( float8 a, float8* b ) { *b = floor_T(a); return fmin_T(a-floor_T(a), 0x1.fffffep-1f); }
static __always_inline float16 fract_T( float16 a, float16* b );
static __always_inline float16 fract_T( float16 a, float16* b ) { *b = floor_T(a); return fmin_T(a-floor_T(a), 0x1.fffffep-1f); }
static __always_inline double fract_T( double a, double* b );
static __always_inline double fract_T( double a, double* b ) { *b = floor(a); return fmin(a-floor(a), 0x1.fffffep-1f); }
static __always_inline double2 fract_T( double2 a, double2* b );
static __always_inline double2 fract_T( double2 a, double2* b ) { *b = floor_T(a); return fmin_T(a-floor_T(a), 0x1.fffffep-1f); }
static __always_inline double4 fract_T( double4 a, double4* b );
static __always_inline double4 fract_T( double4 a, double4* b ) { *b = floor_T(a); return fmin_T(a-floor_T(a), 0x1.fffffep-1f); }
static __always_inline double8 fract_T( double8 a, double8* b );
static __always_inline double8 fract_T( double8 a, double8* b ) { *b = floor_T(a); return fmin_T(a-floor_T(a), 0x1.fffffep-1f); }
static __always_inline double16 fract_T( double16 a, double16* b );
static __always_inline double16 fract_T( double16 a, double16* b ) { *b = floor_T(a); return fmin_T(a-floor_T(a), 0x1.fffffep-1f); }
// frexp
__MATH_BUILTIN_2(hypot)
// ilogb
// ldexp
__MATH_BUILTIN_1(lgamma)
// lgamma_r
__MATH_BUILTIN_1(log)
#ifndef __FreeBSD__
__MATH_BUILTIN_1(log2)
#else
#warning FreeBSD missing log2
#endif
__MATH_BUILTIN_1(log10)
__MATH_BUILTIN_1(log1p)
__MATH_BUILTIN_1(logb)
static __always_inline float madf( float a, float b, float c );
static __always_inline float madf( float a, float b, float c ) { return a*b+c; }
static __always_inline double mad( double a, double b, double c );
static __always_inline double mad( double a, double b, double c ) { return a*b+c; }
__MATH_BUILTIN_3(mad)
static __always_inline float maxmagf( float a, float b );
static __always_inline float maxmagf( float a, float b ) {
	if ( fabsf(a) > fabsf(b) ) return a;
	else if ( fabsf(b) > fabsf(a) ) return b;
	else return fmaxf(a,b);
}
static __always_inline double maxmag( double a, double b );
static __always_inline double maxmag( double a, double b ) {
	if ( fabs(a) > fabs(b) ) return a;
	else if ( fabs(b) > fabs(a) ) return b;
	else return fmax(a,b);
}
__MATH_BUILTIN_2(maxmag)
static __always_inline float minmagf( float a, float b );
static __always_inline float minmagf( float a, float b ) {
	if ( fabsf(a) < fabsf(b) ) return a;
	else if ( fabsf(b) < fabsf(a) ) return b;
	else return fminf(a,b);
}
static __always_inline double minmag( double a, double b );
static __always_inline double minmag( double a, double b ) {
	if ( fabs(a) < fabs(b) ) return a;
	else if ( fabs(b) < fabs(a) ) return b;
	else return fmin(a,b);
}
__MATH_BUILTIN_2(minmag)
// modf
// nan
__MATH_BUILTIN_2(nextafter)
__MATH_BUILTIN_2(pow)
// pown
// powr
__MATH_BUILTIN_2(remainder)
// remquo
__MATH_BUILTIN_1(rint)
// rootn
__MATH_BUILTIN_1(round)
static __always_inline float rsqrtf( float a );
static __always_inline float rsqrtf( float a ) { return 1.0f/sqrtf(a); }
static __always_inline double rsqrt( double a );
static __always_inline double rsqrt( double a ) { return 1.0/sqrt(a); }
__MATH_BUILTIN_1(rsqrt)
__MATH_BUILTIN_1(sin)
// sincos
__MATH_BUILTIN_1(sinh)
__MATH_BUILTIN_1(sqrt)
__MATH_BUILTIN_1(tan)
__MATH_BUILTIN_1(tanh)
static __always_inline float tanpif( float a );
static __always_inline float tanpif( float a ) { return tanf(a*M_PI_F); }
static __always_inline double tanpi( double a );
static __always_inline double tanpi( double a ) { return tan(a*M_PI); }
__MATH_BUILTIN_1(tanpi)
__MATH_BUILTIN_1(tgamma)
__MATH_BUILTIN_1(trunc)

#define acos(a) acos_T(a)
#define acosh(a) acosh_T(a)
#define acospi(a) acospi_T(a)
#define asin(a) asin_T(a)
#define asinh(a) asinh_T(a)
#define asinpi(a) asinpi_T(a)
#define atan(a) atan_T(a)
#define atan2(a,b) atan2_T(a,b)
#define atanh(a) atanh_T(a)
#define atanpi(a) atanpi_T(a)
#define atan2pi(a,b) atan2pi_T(a)
#define cbrt(a) cbrt_T(a)
#define ceil(a) ceil_T(a)
#define copysign(a,b) copysign_T(a,b)
#define cos(a) cos_T(a)
#define cosh(a) cosh_T(a)
#define cospi(a) cospi_T(a)
#define erfc(a) erfc_T(a)
#define erf(a) erf_T(a)
#define exp(a) exp_T(a)
#define exp2(a) exp2_T(a)
#define exp10(a) exp10_T(a)
#define expm1(a) expm1_T(a)
#define fabs(a) fabs_T(a)
#define fdim(a,b) fdim_T(a,b)
#define floor(a) floor_T(a)
#define fma(a,b,c) fma_T(a,b,c)
#define fmax(a,b) fmax_T(a,b)
#define fmin(a,b) fmin_T(a,b)
#define fmod(a,b) fmod_T(a,b)
#define fract(a,b) fract_T(a,b)
//#define frexp(a,b) frexp_T(a,b)
#define hypot(a,b) hypot_T(a,b)
//#define ilogb(a) ilogb_T(a)
//#define ldexp(a,b) ldexp_T(a,b)
#define lgamma(a) lgamma_T(a)
//#define lgamma_r(a,b) lgamma_r_T(a,b)
#define log(a) log_T(a)
#define log2(a) log2_T(a)
#define log10(a) log10_T(a)
#define log1p(a) log1p_T(a)
#define logb(a) logb_T(a)
#define mad(a,b,c) mad_T(a,b,c)
#define maxmag(a,b) maxmag_T(a,b)
#define minmag(a,b) minmag_T(a,b)
//#define modf(a,b) modf_T(a,b)
//#define nan(a) nan_T(a)
#define nextafter(a,b) nextafter_T(a,b)
#define pow(a,b) pow_T(a,b)
//#define pown(a,b) pown_T(a,b)
//#define powr(a,b) powr(a,b)
#define remainder(a,b) remainder_T(a,b)
//#define remquo(a,b,c) remquo_T(a,b,c)
#define rint(a) rint_T(a)
//#define rootn(a,b) rootn_T(a,b)
#define round(a) round_T(a)
#define rsqrt(a) rsqrt_T(a)
#define sin(a) sin_T(a)
//#define sincos(a,b) sincos_T(a,b)
#define sinh(a) sinh_T(a)
#define sqrt(a) sqrt_T(a)
#define tan(a) tan_T(a)
#define tanh(a) tanh_T(a)
#define tanpi(a) tanpi_T(a)
#define tgamma(a) tgamma_T(a)
#define trunc(a) trunc_T(a)


/*** common builtin functions [6.11.4] ***/

template < typename T >
static __always_inline T clamp_T( T a, T b0, T b1 );
template < typename T >
static __always_inline T clamp_T( T a, T b0, T b1 )
{ return min(max(a,b0),b1); }
static __always_inline _float2 clamp_T( _float2 a, float b0, float b1);
static __always_inline _float2 clamp_T( _float2 a, float b0, float b1)
{ return _vector_float2( clamp_T(a.x,b0,b1), clamp_T(a.y,b0,b1) ); }
static __always_inline _float4 clamp_T( _float4 a, float b0, float b1);
static __always_inline _float4 clamp_T( _float4 a, float b0, float b1)
{ 
	return _vector_float4( clamp_T(a.x,b0,b1), clamp_T(a.y,b0,b1), 
		clamp_T(a.z,b0,b1), clamp_T(a.w,b0,b1) ); 
}
static __always_inline _float8 clamp_T( _float8 a, float b0, float b1);
static __always_inline _float8 clamp_T( _float8 a, float b0, float b1)
{ 
	return _vector_float8( clamp_T(a.s0,b0,b1), clamp_T(a.s1,b0,b1), 
		clamp_T(a.s2,b0,b1), clamp_T(a.s3,b0,b1),
		clamp_T(a.s4,b0,b1), clamp_T(a.s5,b0,b1), 
		clamp_T(a.s6,b0,b1), clamp_T(a.s7,b0,b1) ); 
}
static __always_inline _float16 clamp_T( _float16 a, float b0, float b1);
static __always_inline _float16 clamp_T( _float16 a, float b0, float b1)
{ 
	return _vector_float16( clamp_T(a.s0,b0,b1), clamp_T(a.s1,b0,b1), 
		clamp_T(a.s2,b0,b1), clamp_T(a.s3,b0,b1),
		clamp_T(a.s4,b0,b1), clamp_T(a.s5,b0,b1), 
		clamp_T(a.s6,b0,b1), clamp_T(a.s7,b0,b1),
		clamp_T(a.s7,b0,b1), clamp_T(a.s9,b0,b1), 
		clamp_T(a.sA,b0,b1), clamp_T(a.sB,b0,b1),
		clamp_T(a.sC,b0,b1), clamp_T(a.sD,b0,b1), 
		clamp_T(a.sE,b0,b1), clamp_T(a.sF,b0,b1) ); 
}
static __always_inline _double2 clamp_T( _double2 a, double b0, double b1);
static __always_inline _double2 clamp_T( _double2 a, double b0, double b1)
{ return _vector_double2( clamp_T(a.x,b0,b1), clamp_T(a.y,b0,b1) ); }
static __always_inline _double4 clamp_T( _double4 a, double b0, double b1);
static __always_inline _double4 clamp_T( _double4 a, double b0, double b1)
{ 
	return _vector_double4( clamp_T(a.x,b0,b1), clamp_T(a.y,b0,b1), 
		clamp_T(a.z,b0,b1), clamp_T(a.w,b0,b1) ); 
}
static __always_inline _double8 clamp_T( _double8 a, double b0, double b1);
static __always_inline _double8 clamp_T( _double8 a, double b0, double b1)
{ 
	return _vector_double8( clamp_T(a.s0,b0,b1), clamp_T(a.s1,b0,b1), 
		clamp_T(a.s2,b0,b1), clamp_T(a.s3,b0,b1),
		clamp_T(a.s4,b0,b1), clamp_T(a.s5,b0,b1), 
		clamp_T(a.s6,b0,b1), clamp_T(a.s7,b0,b1) ); 
}
static __always_inline _double16 clamp_T( _double16 a, double b0, double b1);
static __always_inline _double16 clamp_T( _double16 a, double b0, double b1)
{ 
	return _vector_double16( clamp_T(a.s0,b0,b1), clamp_T(a.s1,b0,b1), 
		clamp_T(a.s2,b0,b1), clamp_T(a.s3,b0,b1),
		clamp_T(a.s4,b0,b1), clamp_T(a.s5,b0,b1), 
		clamp_T(a.s6,b0,b1), clamp_T(a.s7,b0,b1),
		clamp_T(a.s7,b0,b1), clamp_T(a.s9,b0,b1), 
		clamp_T(a.sA,b0,b1), clamp_T(a.sB,b0,b1),
		clamp_T(a.sC,b0,b1), clamp_T(a.sD,b0,b1), 
		clamp_T(a.sE,b0,b1), clamp_T(a.sF,b0,b1) ); 
}
static __always_inline float degreesf( float a );
static __always_inline float degreesf( float a ) { return (a*180.0f)/M_PI_F; }
static __always_inline double degrees( double a );
static __always_inline double degrees( double a ) { return (a*180.0)/M_PI; }
__MATH_BUILTIN_1(degrees)
// max
// min
// mix
static __always_inline float radiansf( float a );
static __always_inline float radiansf( float a ) { return (a*M_PI_F)/180.0f; }
static __always_inline double radians( double a );
static __always_inline double radians( double a ) { return (a*M_PI)/180.0; }
__MATH_BUILTIN_1(radians)
static __always_inline float stepf( float a, float b );
static __always_inline float stepf( float a, float b ) { return ((b<a)?0.0f:1.0f); }
static __always_inline double step( double a, double b );
static __always_inline double step( double a, double b ) { return ((b<a)?0.0:1.0); }
__MATH_BUILTIN_2(step)
static __always_inline float2 stepf( float a, float2 b );
static __always_inline float2 stepf( float a, float2 b ) { return stepf(_vector_float2(a,a), b); }
static __always_inline float4 stepf( float a, float4 b );
static __always_inline float4 stepf( float a, float4 b ) { return stepf(_vector_float4(a,a,a,a), b); }
static __always_inline float8 stepf( float a, float8 b );
static __always_inline float8 stepf( float a, float8 b ) { return stepf(_vector_float8(a,a,a,a,a,a,a,a), b); }
static __always_inline float16 stepf( float a, float16 b );
static __always_inline float16 stepf( float a, float16 b ) { return stepf(_vector_float16(a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a), b); }
static __always_inline double2 step( double a, double2 b );
static __always_inline double2 step( double a, double2 b ) { return step(_vector_double2(a,a), b); }
static __always_inline double4 step( double a, double4 b );
static __always_inline double4 step( double a, double4 b ) { return step(_vector_double4(a,a,a,a), b); }
static __always_inline double8 step( double a, double8 b );
static __always_inline double8 step( double a, double8 b ) { return step(_vector_double8(a,a,a,a,a,a,a,a), b); }
static __always_inline double16 step( double a, double16 b );
static __always_inline double16 step( double a, double16 b ) { return step(_vector_double16(a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a), b); }
static __always_inline float smoothstepf( float a, float b, float c );
static __always_inline float smoothstepf( float a, float b, float c ) {
	float t = clamp((c - a)/(b - a), 0.0f, 1.0f);
	return t*t*(3.0f-2.0f*t);
}
static __always_inline double smoothstep( double a, double b, double c );
static __always_inline double smoothstep( double a, double b, double c ) {
	double t = clamp((c - a)/(b - a), 0.0, 1.0);
	return t*t*(3.0-2.0*t);
}
__MATH_BUILTIN_3(smoothstep)
static __always_inline float2 smoothstepf( float a, float b, float2 c );
static __always_inline float2 smoothstepf( float a, float b, float2 c ) { return smoothstepf( _vector_float2(a,a), _vector_float2(b,b), c); }
static __always_inline float4 smoothstepf( float a, float b, float4 c );
static __always_inline float4 smoothstepf( float a, float b, float4 c ) { return smoothstepf( _vector_float4(a,a,a,a), _vector_float4(b,b,b,b), c); }
static __always_inline float8 smoothstepf( float a, float b, float8 c );
static __always_inline float8 smoothstepf( float a, float b, float8 c ) { return smoothstepf( _vector_float8(a,a,a,a,a,a,a,a), _vector_float8(b,b,b,b,b,b,b,b), c); }
static __always_inline float16 smoothstepf( float a, float b, float16 c );
static __always_inline float16 smoothstepf( float a, float b, float16 c ) { return smoothstepf( _vector_float16(a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a), _vector_float8(b,b,b,b,b,b,b,b,b,b,b,b,b,b,b,b), c); }
static __always_inline double2 smoothstep( double a, double b, double2 c );
static __always_inline double2 smoothstep( double a, double b, double2 c ) { return smoothstep( _vector_double2(a,a), _vector_double2(b,b), c); }
static __always_inline double4 smoothstep( double a, double b, double4 c );
static __always_inline double4 smoothstep( double a, double b, double4 c ) { return smoothstep( _vector_double4(a,a,a,a), _vector_double4(b,b,b,b), c); }
static __always_inline double8 smoothstep( double a, double b, double8 c );
static __always_inline double8 smoothstep( double a, double b, double8 c ) { return smoothstep( _vector_double8(a,a,a,a,a,a,a,a), _vector_double8(b,b,b,b,b,b,b,b), c); }
static __always_inline double16 smoothstep( double a, double b, double16 c );
static __always_inline double16 smoothstep( double a, double b, double16 c ) { return smoothstep( _vector_double16(a,a,a,a,a,a,a,a,a,a,a,a,a,a,a,a), _vector_double8(b,b,b,b,b,b,b,b,b,b,b,b,b,b,b,b), c); }

static __always_inline float sign_T( int a );
static __always_inline float sign_T( int a ) { return copysignf(1.0f,a); }
static __always_inline double sign_T( double a );
static __always_inline double sign_T( double a ) { return copysign(1.0,a); }
__MATH_BUILTIN_1(sign)

#define clamp(a,b0,b1) clamp_T(a,b0,b1)
#define degrees(a) degrees_T(b)
//#define max(a,b) max_T(a,b)
//#define min(a,b) min_T(a,b)
//#define mix(a,b,c) mix_T(a,b,c)
#define radians(a) radians_T(a)
#define step(a,b) step_T(a,b)
#define smoothstep(a,b,c) smoothstep_T(a,b,c)
#define sign(a) sign_T(a)


/*** geometric builtin functions [6.11.6] ***/

static __always_inline float dot_T( float a, float b);
static __always_inline float dot_T( float a, float b)
{ return a*b; }
static __always_inline float dot_T( _float2 a, _float2 b);
static __always_inline float dot_T( _float2 a, _float2 b)
{ float2 tmp = a*b; return tmp.x+tmp.y; }
static __always_inline float dot_T( _float4 a, _float4 b);
static __always_inline float dot_T( _float4 a, _float4 b)
{ float4 tmp = a*b; float2 tmp2 = tmp.lo+tmp.hi;
	return tmp2.lo+tmp2.hi; }
static __always_inline float dot_T( _float8 a, _float8 b);
static __always_inline float dot_T( _float8 a, _float8 b)
{ float8 tmp = a*b; float4 tmp2 = tmp.lo+tmp.hi;
	float2 tmp3 = tmp2.lo+tmp2.hi; return tmp3.lo+tmp3.hi; }
static __always_inline float dot_T( _float16 a, _float16 b);
static __always_inline float dot_T( _float16 a, _float16 b)
{ float16 tmp = a*b; float8 tmp2 = tmp.lo+tmp.hi;
	float4 tmp3 = tmp2.lo+tmp2.hi; float2 tmp4 = tmp3.lo+tmp3.hi;
	return tmp4.lo+tmp4.hi; }
static __always_inline double dot_T( _double2 a, _double2 b);
static __always_inline double dot_T( _double2 a, _double2 b)
{ double2 tmp = a*b; return tmp.x+tmp.y; }
static __always_inline double dot_T( _double4 a, _double4 b);
static __always_inline double dot_T( _double4 a, _double4 b)
{ double4 tmp = a*b; double2 tmp2 = tmp.lo+tmp.hi;
	return tmp2.lo+tmp2.hi; }
static __always_inline double dot_T( _double8 a, _double8 b);
static __always_inline double dot_T( _double8 a, _double8 b)
{ double8 tmp = a*b; double4 tmp2 = tmp.lo+tmp.hi;
	double2 tmp3 = tmp2.lo+tmp2.hi; return tmp3.lo+tmp3.hi; }
static __always_inline double dot_T( _double16 a, _double16 b);
static __always_inline double dot_T( _double16 a, _double16 b)
{ double16 tmp = a*b; double8 tmp2 = tmp.lo+tmp.hi;
	double4 tmp3 = tmp2.lo+tmp2.hi; double2 tmp4 = tmp3.lo+tmp3.hi;
	return tmp4.lo+tmp4.hi; }

static __always_inline _float4 cross_T( _float4 a, _float4 b);
static __always_inline _float4 cross_T( _float4 a, _float4 b)
{ return _vector_float4(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x, 0.0f); }

template < typename T >
static __always_inline T length_T( T a );
template < typename T >
static __always_inline T length_T( T a ) { return sqrt_T(dot_T(a,a)); }

template < typename T >
static __always_inline T distance_T( T a, T b );
template < typename T >
static __always_inline T distance_T( T a, T b ) { return length_T(b-a); }

template < typename T >
static __always_inline T normalize_T( T a );
template < typename T >
static __always_inline T normalize_T( T a ) { return rsqrt_T(dot_T(a,a))*a; }

// fast_distance
// fast_length
// fast_normalize

#define dot(a,b) dot_T(a,b)
#define cross(a,b) cross_T(a,b)
#define distance(a,b) distance_T(a,b)
#define length(a) length_T(a)
#define normalize(a) normalize_T(a)
#define fast_distance(a,b) distance_T(a,b)
#define fast_length(a,b) length_T(a,b)
#define fast_normalize(a) normalize_T(a)


/*** sampler declarations [6.11.8.1] ***/

typedef int sampler_t;
#define CLK_NORMALIZED_COORDS_FALSE 0
#define CLK_ADDRESS_NONE 0
#define CLK_FILTER_NEAREST 0


/*** image read write builtins [6.11.18] ***/

#define read_imagei( _img, _smplr, _xy) \
*((int4*)((intptr_t)_img+128)+((__global size_t*)_img)[0]*(_xy).y+(_xy).x)

#define read_imageui( _img, _smplr, _xy) \
*((uint4*)((intptr_t)_img+128)+((__global size_t*)_img)[0]*(_xy).y+(_xy).x)

#define read_imagef( _img, _smplr, _xy) \
*((float4*)((intptr_t)_img+128)+((__global size_t*)_img)[0]*(_xy).y+(_xy).x)


/*** integer builtins [6.12.3] ***/


/*** builtin extensions for initializing vector data types [non-standard] ***/

#define __builtin_vector_char2(a,b) vector_char2(a,b)
#define __builtin_vector_char4(a,b,c,d) vector_char4(a,b,c,d)
#define __builtin_vector_char8(a,b,c,d,e,f,g,h) vector_char8(a,b,c,d,e,f,g,h)
#define __builtin_vector_char16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_char16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_uchar2(a,b) vector_uchar2(a,b)
#define __builtin_vector_uchar4(a,b,c,d) vector_uchar4(a,b,c,d)
#define __builtin_vector_uchar8(a,b,c,d,e,f,g,h) vector_uchar8(a,b,c,d,e,f,g,h)
#define __builtin_vector_uchar16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_uchar16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_short2(a,b) vector_short2(a,b)
#define __builtin_vector_short4(a,b,c,d) vector_short4(a,b,c,d)
#define __builtin_vector_short8(a,b,c,d,e,f,g,h) vector_short8(a,b,c,d,e,f,g,h)
#define __builtin_vector_short16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_short16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_ushort2(a,b) vector_ushort2(a,b)
#define __builtin_vector_ushort4(a,b,c,d) vector_ushort4(a,b,c,d)
#define __builtin_vector_ushort8(a,b,c,d,e,f,g,h) vector_ushort8(a,b,c,d,e,f,g,h)
#define __builtin_vector_ushort16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_ushort16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_int2(a,b) vector_int2(a,b)
#define __builtin_vector_int4(a,b,c,d) vector_int4(a,b,c,d)
#define __builtin_vector_int8(a,b,c,d,e,f,g,h) vector_int8(a,b,c,d,e,f,g,h)
#define __builtin_vector_int16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_int16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_uint2(a,b) vector_uint2(a,b)
#define __builtin_vector_uint4(a,b,c,d) vector_uint4(a,b,c,d)
#define __builtin_vector_uint8(a,b,c,d,e,f,g,h) vector_uint8(a,b,c,d,e,f,g,h)
#define __builtin_vector_uint16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_uint16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_long2(a,b) vector_long2(a,b)
#define __builtin_vector_long4(a,b,c,d) vector_long4(a,b,c,d)
#define __builtin_vector_long8(a,b,c,d,e,f,g,h) vector_long8(a,b,c,d,e,f,g,h)
#define __builtin_vector_long16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_long16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_ulong2(a,b) vector_ulong2(a,b)
#define __builtin_vector_ulong4(a,b,c,d) vector_ulong4(a,b,c,d)
#define __builtin_vector_ulong8(a,b,c,d,e,f,g,h) vector_ulong8(a,b,c,d,e,f,g,h)
#define __builtin_vector_ulong16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_ulong16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_float2(a,b) vector_float2(a,b)
#define __builtin_vector_float4(a,b,c,d) vector_float4(a,b,c,d)
#define __builtin_vector_float8(a,b,c,d,e,f,g,h) vector_float8(a,b,c,d,e,f,g,h)
#define __builtin_vector_float16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_float16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_double2(a,b) vector_double2(a,b)
#define __builtin_vector_double4(a,b,c,d) vector_double4(a,b,c,d)
#define __builtin_vector_double8(a,b,c,d,e,f,g,h) vector_double8(a,b,c,d,e,f,g,h)
#define __builtin_vector_double16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) vector_double16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)

#else

/***
 *** included if compiling opencl kernel with an opencl compiler
 ***/

/*** builtin extensions for initializing vector data types [non-standard] ***/

#define __builtin_vector_char2(a,b) (vector_char2)(a,b)
#define __builtin_vector_char4(a,b,c,d) (vector_char4)(a,b,c,d)
#define __builtin_vector_char8(a,b,c,d,e,f,g,h) (vector_char8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_char16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_char16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_uchar2(a,b) (vector_uchar2)(a,b)
#define __builtin_vector_uchar4(a,b,c,d) (vector_uchar4)(a,b,c,d)
#define __builtin_vector_uchar8(a,b,c,d,e,f,g,h) (vector_uchar8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_uchar16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_uchar16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_short2(a,b) (vector_short2)(a,b)
#define __builtin_vector_short4(a,b,c,d) (vector_short4)(a,b,c,d)
#define __builtin_vector_short8(a,b,c,d,e,f,g,h) (vector_short8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_short16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_short16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_ushort2(a,b) (vector_ushort2)(a,b)
#define __builtin_vector_ushort4(a,b,c,d) (vector_ushort4)(a,b,c,d)
#define __builtin_vector_ushort8(a,b,c,d,e,f,g,h) (vector_ushort8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_ushort16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_ushort16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_int2(a,b) (vector_int2)(a,b)
#define __builtin_vector_int4(a,b,c,d) (vector_int4)(a,b,c,d)
#define __builtin_vector_int8(a,b,c,d,e,f,g,h) (vector_int8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_int16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_int16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_uint2(a,b) (vector_uint2)(a,b)
#define __builtin_vector_uint4(a,b,c,d) (vector_uint4)(a,b,c,d)
#define __builtin_vector_uint8(a,b,c,d,e,f,g,h) (vector_uint8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_uint16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_uint16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_long2(a,b) (vector_long2)(a,b)
#define __builtin_vector_long4(a,b,c,d) (vector_long4)(a,b,c,d)
#define __builtin_vector_long8(a,b,c,d,e,f,g,h) (vector_long8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_long16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_long16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_ulong2(a,b) (vector_ulong2)(a,b)
#define __builtin_vector_ulong4(a,b,c,d) (vector_ulong4)(a,b,c,d)
#define __builtin_vector_ulong8(a,b,c,d,e,f,g,h) (vector_ulong8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_ulong16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_ulong16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_float2(a,b) (vector_float2)(a,b)
#define __builtin_vector_float4(a,b,c,d) (vector_float4)(a,b,c,d)
#define __builtin_vector_float8(a,b,c,d,e,f,g,h) (vector_float8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_float16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_float16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_double2(a,b) (vector_double2)(a,b)
#define __builtin_vector_double4(a,b,c,d) (vector_double4)(a,b,c,d)
#define __builtin_vector_double8(a,b,c,d,e,f,g,h) (vector_double8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_double16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (vector_double16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)

#endif

#endif // _opencl_lift_h
