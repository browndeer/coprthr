

#ifndef _CLRPC_H
#define _CLRPC_H

#include <CL/cl.h>

#include "util.h"
#include "clrpc_common.h"

int clrpc_init( void );
int clrpc_final( void );

cl_int 
clrpc_clGetPlatformIDs( cl_uint nplatforms,
	cl_platform_id* platforms, cl_uint* nplatforms_ret);

cl_int
clrpc_clGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name,
	size_t param_sz, void *param_val, size_t *param_sz_ret);

cl_int
clrpc_clGetDeviceIDs( cl_platform_id platform, cl_device_type devtype,
   cl_uint ndevices, cl_device_id* devices, cl_uint* ndevices_ret);
   
cl_int
clrpc_clGetDeviceInfo(cl_device_id device, cl_device_info param_name,
	size_t param_sz, void *param_val, size_t *param_sz_ret);

#endif

