#include <math.h>
#include <stdio.h>
#include <stdint.h>

#define USING_21

#ifndef USING_21
int __convert_int_f32( float x ) { return( (int)x ); }
float __convert_float_i32( int i ) { return( (float)i ); }
#endif

double __lgamma_r_pf64i32 ( double x, int* ps )
{
return( lgamma_r(x,ps) ); 
}

typedef int32_t        cl_int;
typedef cl_int      __cl_int2       __attribute__((vector_size(8)));
typedef cl_int      __cl_int4       __attribute__((vector_size(16)));
typedef float      __cl_float2       __attribute__((vector_size(8)));
typedef float      __cl_float4       __attribute__((vector_size(16)));

__cl_int4 __read_imagei_image2d2i32( void* img, int smplr, __cl_int2 xy )
{ 
	return(*((__cl_int4*)((intptr_t)img+128) + ((size_t*)img)[0]*xy[1] + xy[0]));
}

__cl_float4 __read_imagef_image2d2i32( void* img, int smplr, __cl_int2 xy )
{ 
//	fprintf(stderr,"__read_imagef_image2d2i32: %d %d\n",xy[0],xy[1]); fflush(stderr);
	return(*((__cl_float4*)((intptr_t)img+128) + ((size_t*)img)[0]*xy[1] + xy[0]));
}


