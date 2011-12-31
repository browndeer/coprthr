
#ifndef _libocl_h
#define _libocl_h

#include "oclcall.h"

struct platform_struct {
   void* _reserved;
   void* dlh;
	struct oclent_struct* oclent;
	cl_platform_id imp_platform_id;
};

cl_int clGetPlatformIDs(
   cl_uint nplatforms,
   cl_platform_id* platforms,
   cl_uint* nplatforms_ret
);

cl_int clGetPlatformInfo(cl_platform_id a0,cl_platform_info a1,size_t a2,void* a3,size_t* a4);

#endif

