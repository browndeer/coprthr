/* ocl_kernel.c 
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


#include <CL/cl.h>

#include "xcl_structs.h"
#include "printcl.h"
#include "kernel.h"
#include "program.h"

// Kernel Object API Calls


cl_kernel 
_clCreateKernel(
	 cl_program prg,
	 const char* kname,
	 cl_int* err_ret
)
{
	printcl( CL_DEBUG "clCreateKernel");

	if (__invalid_program(prg)) __error_return(CL_INVALID_PROGRAM,cl_kernel);

	if (__invalid_executable(prg)) 
		__error_return(CL_INVALID_PROGRAM_EXECUTABLE,cl_kernel);

	cl_int k = __do_find_kernel_in_program(prg,kname);

	if (k < 0) __error_return(CL_INVALID_KERNEL_NAME,cl_kernel);

	struct _cl_kernel* krn 
		= (struct _cl_kernel*)malloc(sizeof(struct _cl_kernel));

	if (krn) {

		__init_kernel(krn);

		__retain_program(prg);

		krn->ctx = prg->ctx;
		krn->prg = prg;

		__do_create_kernel(krn,k);

		krn->refc = 1;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,cl_kernel);

	__success();

	return((cl_kernel)krn);
}


cl_int 
_clCreateKernelsInProgram(
	 cl_program prg,
	 cl_uint nkrn,
	 cl_kernel* kernels,
	 cl_uint* nkrn_ret
)
{
	printcl( CL_DEBUG "clCreateKernelsInProgram");

	if (__invalid_program(prg)) return(CL_INVALID_PROGRAM);

	if (__invalid_executable(prg)) return(CL_INVALID_PROGRAM_EXECUTABLE);

	if (kernels && nkrn < __nkernels_in_program(prg)) return(CL_INVALID_VALUE);

	if (nkrn_ret) *nkrn_ret = __nkernels_in_program(prg); 

	int k;

	if (kernels) for(k=0;k<__nkernels_in_program(prg);k++) {

		struct _cl_kernel* krn 
			= (struct _cl_kernel*)malloc(sizeof(struct _cl_kernel));

		if (krn) {

			__init_kernel(krn);

			__retain_program(prg);

			krn->ctx = prg->ctx;
			krn->prg = prg;

			__do_create_kernel(krn,k);

			krn->refc = 1;

			kernels[k] = krn;

		} else return(CL_OUT_OF_HOST_MEMORY);

	}

	return(CL_SUCCESS);
}


cl_int 
_clRetainKernel( cl_kernel krn )
{
	printcl( CL_DEBUG "clRetainKernel");

	if (__invalid_kernel(krn)) return(CL_INVALID_KERNEL);

	__retain_kernel(krn);
	
	return(CL_SUCCESS);
}


cl_int 
_clReleaseKernel( cl_kernel krn )
{
	printcl( CL_DEBUG "clReleaseKernel");

	if (__invalid_kernel(krn)) return(CL_INVALID_KERNEL);

	__release_kernel(krn);

	return(CL_SUCCESS);
}


cl_int 
_clSetKernelArg(
	 cl_kernel krn,
	 cl_uint argn,
	 size_t arg_sz,
	 const void* arg_val
)
{
	printcl( CL_DEBUG "clSetKernelArg");

	if (__invalid_kernel(krn)) return(CL_INVALID_KERNEL);

	if (argn >= krn->narg) return(CL_INVALID_ARG_INDEX);

	/* invalid_arg_value if null and not __local */
	/* invalid_mem_object if memobj mismatch */
	/* invalid_sampler if sampler mismatch */
	/* innvalid_arg_size if ... */

	cl_uint err = (cl_uint)__do_set_kernel_arg(krn,argn,arg_sz,arg_val);

	return((err==0)?CL_SUCCESS:err);
}


cl_int 
_clGetKernelInfo(
	 cl_kernel krn,
	 cl_kernel_info param_name,
	 size_t param_sz,
	 void* param_val,
	 size_t* param_sz_ret
)
{
	printcl( CL_WARNING "clGetKernelInfo: warning: unsupported");

	if (__invalid_kernel(krn)) return(CL_INVALID_KERNEL);

	size_t sz;

	switch (param_name) {

		case CL_KERNEL_FUNCTION_NAME:

			sz = strnlen(krn->name,__CLMAXSTR_LEN)+1;
			printcl( CL_DEBUG "clGetKernelInfo: name |%s|",krn->name);
			__case_get_param(sz,krn->name);
			printcl( CL_DEBUG "clGetKernelInfo: param_val |%s|",param_val);

			break;

		case CL_KERNEL_NUM_ARGS:

			__case_get_param(sizeof(cl_uint),&krn->narg);

			break;

		case CL_KERNEL_REFERENCE_COUNT:

			__case_get_param(sizeof(cl_uint),&krn->refc);

			break;

		case CL_KERNEL_CONTEXT:

			__case_get_param(sizeof(cl_context),&krn->ctx);

			break;

		case CL_KERNEL_PROGRAM:

			__case_get_param(sizeof(cl_program),&krn->prg);

			break;

		default:
			
			return(CL_INVALID_VALUE);

	}

	return(CL_SUCCESS);
}


cl_int 
_clGetKernelWorkGroupInfo(
	 cl_kernel krn,
	 cl_device_id devid,
	 cl_kernel_work_group_info param_name,
	 size_t param_sz,
	 void* param_val,
	 size_t* param_sz_ret
)
{
	printcl( CL_WARNING "clGetKernelWorkGroupInfo: warning: unsupported");

	if (__invalid_kernel(krn)) return(CL_INVALID_KERNEL);

	return(CL_ENOTSUP);

	size_t sz;

	switch (param_name) {

		case CL_KERNEL_WORK_GROUP_SIZE:

//			__case_get_param(sizeof(size_t),);
			return(CL_ENOTSUP);

			break;

		case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:

//			__case_get_param(sizeof(size_t),);
			return(CL_ENOTSUP);

			break;

		case CL_KERNEL_LOCAL_MEM_SIZE:

//			__case_get_param(sizeof(size_t),);
			return(CL_ENOTSUP);

			break;

		default:

			return(CL_INVALID_VALUE);

	}

	return(CL_ENOTSUP);
}


// Aliased Kernel Object API calls


cl_kernel
clCreateKernel( cl_program prg, const char* kname, cl_int* err_ret)
	__attribute__((alias("_clCreateKernel")));

cl_int
clCreateKernelsInProgram( cl_program prg, cl_uint nkrn, cl_kernel* kernels,
    cl_uint* nkrn_ret)
	__attribute__((alias("_clCreateKernelsInProgram")));

cl_int
clRetainKernel( cl_kernel krn )
	__attribute__((alias("_clRetainKernel")));

cl_int
clReleaseKernel( cl_kernel krn )
	__attribute__((alias("_clReleaseKernel")));

cl_int
clSetKernelArg( cl_kernel krn, cl_uint argn, size_t arg_sz, 
	const void* arg_val)
	__attribute__((alias("_clSetKernelArg")));

cl_int
clGetKernelInfo( cl_kernel krn, cl_kernel_info param_name, size_t param_sz,
    void* param_val, size_t* param_sz_ret)
	__attribute__((alias("_clGetKernelInfo")));

cl_int
clGetKernelWorkGroupInfo( cl_kernel krn, cl_device_id devid, 
	cl_kernel_work_group_info param_name, size_t param_sz, void* param_val,
	size_t* param_sz_ret)
	__attribute__((alias("_clGetKernelWorkGroupInfo")));

