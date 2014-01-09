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


void __do_create_kernel(cl_kernel, cl_uint knum);
void __do_release_kernel(cl_kernel);
int __do_set_kernel_arg(cl_kernel, cl_uint, size_t arg_sz, const void* arg_val);


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

printcl( CL_WARNING "clGetKernelWorkGroupInfo: krn %p",krn);

	if (__invalid_kernel(krn)) return(CL_INVALID_KERNEL);

	size_t sz;

	size_t hack = __resolve_devid_ocldevinfo(devid,max_wg_sz);

	switch (param_name) {

		case CL_KERNEL_WORK_GROUP_SIZE:

			__case_get_param(sizeof(size_t),&hack);
//			return(CL_ENOTSUP);

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

	return(CL_SUCCESS);
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

/* kernel.c
 *
 * Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
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


void __do_create_kernel(cl_kernel krn, cl_uint k) 
{
	int i,idev;

	cl_program prg = krn->prg;

	printcl( CL_DEBUG "__do_create_kernel: knum=%d",k);

	krn->name = prg->prg1[0]->kname[k];

	printcl( CL_DEBUG "__do_create_kernel: kname=%s",krn->name);

	krn->krn1 = (struct coprthr_kernel**)
		malloc(prg->ndev * sizeof(struct coprthr_kernel*));

	for(i=0; i<prg->ndev; i++) {
		krn->krn1[i] = (struct coprthr_kernel*)
			malloc(sizeof(struct coprthr_kernel));
		krn->krn1[i]->prg1 = prg->prg1[i];
		krn->krn1[i]->knum = k;
	}	

	cl_uint narg = krn->narg = prg->prg1[0]->knarg[k];

	printcl( CL_DEBUG "__do_create_kernel: narg=%d",narg);

	if (narg == 0) return;

/*
	krn->krn1[0]->arg_off = malloc(narg*sizeof(uint32_t));
		
	size_t arg_buf_sz = krn->krn1[0]->arg_buf_sz = prg->prg1[0]->karg_buf_sz[k];

	if (arg_buf_sz > 0) krn->krn1[0]->arg_buf = malloc(arg_buf_sz);

	size_t sz = 0;

	for(i=0;i<narg;i++) {
		krn->krn1[0]->arg_off[i] = sz;
		sz += krn->krn1[0]->prg1->karg_sz[k][i];
		printcl( CL_DEBUG "CHECKING arg_sz[%d] %d",
			i,krn->krn1[0]->prg1->karg_sz[k][i]);
	}
*/

/* XXX need multiple buffers for each device */

	for(idev=0; idev<prg->ndev; idev++) {

	krn->krn1[idev]->arg_off = malloc(narg*sizeof(uint32_t));
		
	size_t arg_buf_sz = krn->krn1[idev]->arg_buf_sz 
		= prg->prg1[idev]->karg_buf_sz[k];

	if (arg_buf_sz > 0) krn->krn1[idev]->arg_buf = malloc(arg_buf_sz);

	size_t sz = 0;

	for(i=0;i<narg;i++) {
		krn->krn1[idev]->arg_off[i] = sz;
		sz += krn->krn1[idev]->prg1->karg_sz[k][i];
		printcl( CL_DEBUG "CHECKING arg_sz[%d] %d",
			i,krn->krn1[idev]->prg1->karg_sz[k][i]);
	}

	}

}

void __do_release_kernel(cl_kernel krn) {}


int __do_set_kernel_arg( 
	cl_kernel krn, cl_uint argn, size_t arg_sz, const void* arg_val 
)
{
//	__do_set_kernel_arg_1( krn->krn1[0], argn, arg_sz, arg_val );

	struct coprthr_mem* mem1;

	/* resolve arg_val for all devices to compensate for opencl design mistake */

	unsigned int ndev = krn->ctx->ndev;

	int idev;

	for(idev=0;idev<ndev;idev++) {

		int knum = krn->krn1[idev]->knum;

		printcl( CL_DEBUG "__do_set_kernel_arg: %d/%d knum=%d",idev,ndev,knum);

		cl_uint arg_kind = krn->krn1[idev]->prg1->karg_kind[knum][argn];

		/* XXX hack to allow user to strongly imply local address space -DAR */
		if (arg_sz > 0 && arg_val == 0) arg_kind = CLARG_KIND_LOCAL;

		int err;
		struct coprthr_mem* mem1;
		switch (arg_kind) {

			case CLARG_KIND_GLOBAL:
			case CLARG_KIND_CONSTANT:

				mem1 = (*(cl_mem*)arg_val)->mem1[idev];
				err = __do_set_kernel_arg_1(krn->krn1[idev], argn, arg_sz, &mem1);
				break;

			default:

				err = __do_set_kernel_arg_1(krn->krn1[idev], argn, arg_sz, arg_val);
				break;

		}

		if (err) return err;

	}

#if(0)
	void* p = krn->krn1[0]->arg_buf + krn->krn1[0]->arg_off[argn];

	int knum = krn->krn1[0]->knum;

	cl_uint arg_kind = krn->krn1[0]->prg1->karg_kind[knum][argn];

	/* XXX hack to allow user to strongly imply local address space -DAR */
	if (arg_sz > 0 && arg_val == 0) arg_kind = CLARG_KIND_LOCAL;

	switch (arg_kind) {

		case CLARG_KIND_VOID:

			printcl( CL_WARNING "setting void arg has no effect");

			break;

		case CLARG_KIND_DATA:

			printcl( CL_DEBUG "CLARG_KIND_DATA compare sz %d %d",
				arg_sz,krn->krn1[0]->prg1->karg_sz[knum][argn]);

			if (arg_sz != krn->krn1[0]->prg1->karg_sz[knum][argn]) 
				return(CL_INVALID_ARG_SIZE);

			if (!arg_val) return (CL_INVALID_ARG_VALUE);

			memcpy(p,arg_val,arg_sz);

			break; 

		case CLARG_KIND_GLOBAL:

			if (arg_sz != krn->krn1[0]->prg1->karg_sz[knum][argn]) 
				return(CL_INVALID_ARG_SIZE);

			if (!arg_val) return (CL_INVALID_ARG_VALUE);

			if (__invalid_memobj(arg_val)) return(CL_INVALID_MEM_OBJECT);

			if (arg_sz != sizeof(cl_mem)) return(CL_INVALID_ARG_SIZE);

			printcl( CL_DEBUG "from set arg %p %p",
				arg_val,*(cl_mem*)arg_val);

			memcpy(p,arg_val,arg_sz);

			break;

		case CLARG_KIND_LOCAL:

			if (arg_val) return (CL_INVALID_ARG_VALUE);

			if (arg_sz == 0) return(CL_INVALID_ARG_SIZE);

			*(size_t*)p = arg_sz;

			break;

		case CLARG_KIND_CONSTANT:

			if (arg_sz != krn->krn1[0]->prg1->karg_sz[knum][argn]) 
				return(CL_INVALID_ARG_SIZE);

			if (!arg_val) return (CL_INVALID_ARG_VALUE);

			if (__invalid_memobj(arg_val)) return(CL_INVALID_MEM_OBJECT);

			if (arg_sz != sizeof(cl_mem)) return(CL_INVALID_ARG_SIZE);

			printcl( CL_DEBUG "from set arg %p %p",
				arg_val,*(cl_mem*)arg_val);

			memcpy(p,arg_val,arg_sz);

			break;

		case CLARG_KIND_SAMPLER:

			if (arg_sz != sizeof(cl_sampler)) return(CL_INVALID_ARG_SIZE);

			printcl( CL_ERR "sampler arg not supported");
			return(CL_ENOTSUP);

			break;

		case CLARG_KIND_IMAGE2D:

			printcl( CL_ERR "image2d arg not supported");
			return(CL_ENOTSUP);

			break;

		case CLARG_KIND_IMAGE3D:

			printcl( CL_ERR "image3d arg not supported");
			return(CL_ENOTSUP);

			break;

		default:
			break;
	}
#endif

	return(CL_SUCCESS);

}

