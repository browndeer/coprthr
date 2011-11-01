
#ifndef __libocl_h
#define __libocl_h

#define image2d_t __global void*

#define read_imagei( _img, _smplr, _xy) \
*((int4*)((intptr_t)_img+128)+((__global size_t*)_img)[0]*(_xy).y+(_xy).x)

#define read_imagef( _img, _smplr, _xy) \
*((float4*)((intptr_t)_img+128)+((__global size_t*)_img)[0]*(_xy).y+(_xy).x)


#define __kernel
#define __global
#define __read_only

#include <math.h>

typedef int sampler_t;
#define CLK_NORMALIZED_COORDS_FALSE 0
#define CLK_ADDRESS_NONE 0
#define CLK_FILTER_NEAREST 0

//#define get_global_id(a) 0
#define convert_int(x) 0
#define convert_float(i) 0.0
#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a<b)?b:a)

//__inline float _sqrt( float x )  { return sqrtf(x); }
//__inline double _sqrt( double x ) { return sqrt(x); }
#define sqrt(x) ({ \
	typeof(x) tmp; \
	if (__builtin_types_compatible_p(typeof(x),float)) tmp = sqrtf(x); \
	else if (__builtin_types_compatible_p(typeof(x),double)) tmp = sqrt(x); \
	tmp; \
	}) 

struct _int2 {
	union {
		struct { int x,y; };
		struct { int s0,s1; };
	};
};
typedef struct _int2 int2;

struct _int4 {
	union {
   	struct { int x,y,z,w; };
   	struct { int2 xy,zw; };
   	struct { int s0,s1,s2,s3; };
   	struct { int2 s12,s34; };
	};
};
typedef struct _int4 int4;

struct _float2 {
	union {
  		struct { float x,y; };
  		struct { float s0,s1; };
	};
};
typedef struct _float2 float2;

struct _float4 {
	union {
   	struct { float x,y,z,w; };
   	struct { float2 xy,zw; };
	};
};
typedef struct _float4 float4;

__inline double as_double( float2 f2 ) { return *(double*)(&f2); }

#include <stdint.h>
#include <sys/types.h>

#endif

