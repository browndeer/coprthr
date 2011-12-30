
#ifndef _libocl_h
#define _libocl_h

#include "oclcall.h"

struct platform_struct {
   void* _reserved;
   void* dlh;
	struct oclent_struct* oclent;
	cl_platform_id imp_platform;
};

cl_int clGetPlatformIDs(
   cl_uint nplatforms,
   cl_platform_id* platforms,
   cl_uint* nplatforms_ret
);

#endif

