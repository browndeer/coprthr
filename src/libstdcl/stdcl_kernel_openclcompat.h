/* stdcl_kernel_openclcompat.h
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

#ifndef _STDCL_H
#error Do not include stdcl_kernel_openclcompat.h directly, include stdcl.h instead.
#endif

#ifndef _STDCL_KERNEL_OPENCLCOMPAT_H
#define _STDCL_KERNEL_OPENCLCOMPAT_H

#define __builtin_vector_short2(a,b)                               (short2)(a,b)
#define __builtin_vector_short4(a,b,c,d)                           (short4)(a,b,c,d)
#define __builtin_vector_short8(a,b,c,d,e,f,g,h)                   (short8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_short16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)  (short16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_ushort2(a,b)                              (ushort2)(a,b)
#define __builtin_vector_ushort4(a,b,c,d)                          (ushort4)(a,b,c,d)
#define __builtin_vector_ushort8(a,b,c,d,e,f,g,h)                  (ushort8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_ushort16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (ushort16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_char2(a,b)                                (char2)(a,b)
#define __builtin_vector_char4(a,b,c,d)                            (char4)(a,b,c,d)
#define __builtin_vector_char8(a,b,c,d,e,f,g,h)                    (char8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_char16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)   (char16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_uchar2(a,b)                               (uchar2)(a,b)
#define __builtin_vector_uchar4(a,b,c,d)                           (uchar4)(a,b,c,d)
#define __builtin_vector_uchar8(a,b,c,d,e,f,g,h)                   (uchar8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_uchar16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)  (uchar16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_int2(a,b)                                 (int2)(a,b)
#define __builtin_vector_int4(a,b,c,d)                             (int4)(a,b,c,d)
#define __builtin_vector_int8(a,b,c,d,e,f,g,h)                     (int8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_int16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)    (int16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_uint2(a,b)                                (uint2)(a,b)
#define __builtin_vector_uint4(a,b,c,d)                            (uint4)(a,b,c,d)
#define __builtin_vector_uint8(a,b,c,d,e,f,g,h)                    (uint8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_uint16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)   (uint16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_long2(a,b)                                (long2)(a,b)
#define __builtin_vector_long4(a,b,c,d)                            (long4)(a,b,c,d)
#define __builtin_vector_long8(a,b,c,d,e,f,g,h)                    (long8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_long16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)   (long16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_ulong2(a,b)                               (ulong2)(a,b)
#define __builtin_vector_ulong4(a,b,c,d)                           (ulong4)(a,b,c,d)
#define __builtin_vector_ulong8(a,b,c,d,e,f,g,h)                   (ulong8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_ulong16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)  (ulong16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_float2(a,b)                               (float2)(a,b)
#define __builtin_vector_float4(a,b,c,d)                           (float4)(a,b,c,d)
#define __builtin_vector_float8(a,b,c,d,e,f,g,h)                   (float8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_float16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)  (float16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#define __builtin_vector_double2(a,b)                              (double2)(a,b)
#define __builtin_vector_double4(a,b,c,d)                          (double4)(a,b,c,d)
#define __builtin_vector_double8(a,b,c,d,e,f,g,h)                  (double8)(a,b,c,d,e,f,g,h)
#define __builtin_vector_double16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) (double16)(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)

#endif

