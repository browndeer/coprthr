/* xcl_program.c 
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


// Program Object APIs


cl_program 
clCreateProgramWithSource(
	 cl_context ctx,
	 cl_uint count,
	 const char** strings,
	 const size_t* lens,
	 cl_int* err_ret
)
{
	DEBUG(__FILE__,__LINE__,"clCreateProgramWithSource");

	if (__invalid_context(ctx)) __error_return(CL_INVALID_CONTEXT,cl_program);

	if (count==0 || !strings) __error_return(CL_INVALID_VALUE,cl_program);

	int i;

	for(i=0;i<count;i++) 
		if (!strings[i]) __error_return(CL_INVALID_VALUE,cl_program);

	struct _cl_program* prg 
		= (struct _cl_program*)malloc(sizeof(struct _cl_program));

	if (prg) {

		__init_program(prg);

		int i;

		size_t sz = 0;

		for(i=0;i<count;i++) {
			sz += (lens && lens[i] > 0)? lens[i] : strlen(strings[i]);
		}
		
		prg->src_sz = sz;
		prg->src = (unsigned char*)malloc((sz+1)*sizeof(unsigned char));

		if (!prg->src) {
			__free_program(prg);
			__error_return(CL_OUT_OF_HOST_MEMORY,cl_program);
		}

		unsigned char* p = prg->src;	
		for(i=0;i<count;i++) {
			sz = (lens && lens[i] > 0)? lens[i] : strlen(strings[i]);
			strncpy(p,strings[i],sz);
			p += sz;
		}

		prg->ctx = ctx;

		cl_uint ndev = ctx->ndev;

		prg->ndev = ndev;
		__clone(prg->devices,ctx->devices,ndev,cl_device_id);

		prg->bin_stat = (cl_uint*)calloc(ndev,sizeof(cl_uint));
		prg->bin_sz = (size_t*)calloc(ndev,sizeof(size_t));
		prg->bin = (unsigned char**)calloc(ndev,sizeof(unsigned char*));
		prg->build_stat = (cl_build_status*)calloc(ndev,sizeof(cl_build_status));
		prg->build_options = (char**)calloc(ndev,sizeof(char*));
		prg->build_log = (char**)calloc(ndev,sizeof(char*));

		for(i=0; i<ndev; i++) prg->build_stat[i] = CL_BUILD_NONE;

		__do_create_program(prg);

		prg->refc = 1;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,cl_program);

	__success();
	
	return((cl_program)prg);
}


cl_program 
clCreateProgramWithBinary(
	 cl_context ctx,
	 cl_uint ndev,
	 const cl_device_id* devices,
	 const size_t* lens,
	 const unsigned char** bins,
	 cl_int* stat,
	 cl_int* err_ret
)
{
	DEBUG(__FILE__,__LINE__,"clCreateProgramWithBinary");

	if (__invalid_context(ctx)) __error_return(CL_INVALID_CONTEXT,cl_program);

	if (ndev==0 || !devices) __error_return(CL_INVALID_VALUE,cl_program);

	int i,j,k;

	for(i=0;i<ndev;i++) {
		k = -1;
		for(j=0;j<ctx->ndev;j++) if (devices[i] == ctx->devices[j]) k = j; 
		if (k < 0) __error_return(CL_INVALID_DEVICE,cl_program);
	}

	if (!lens || !bins) __error_return(CL_INVALID_VALUE,cl_program);

	for(i=0;i<ndev;i++) 
		if (lens[i]==0 || !bins[i]) __error_return(CL_INVALID_VALUE,cl_program);

	struct _cl_program* prg 
		= (struct _cl_program*)malloc(sizeof(struct _cl_program));

	if (prg) {

		__init_program(prg);

		prg->ctx = ctx;

		prg->ndev = ndev;
		__clone(prg->devices,devices,ndev,cl_device_id);

		/* XXX is this used for aything? -DAR */
		prg->bin_stat = (cl_uint*)calloc(ndev,sizeof(cl_uint));

		__clone(prg->bin,bins,ndev,unsigned char*);
		__clone(prg->bin_sz,lens,ndev,size_t);

		prg->build_stat = (cl_build_status*)malloc(ndev*sizeof(cl_build_status));
		prg->build_options = (char**)malloc(ndev*sizeof(char*));
		prg->build_log = (char**)malloc(ndev*sizeof(char*));

		for(i=0; i<ndev; i++) 
			prg->build_stat[i] = (prg->bin[i] && prg->bin_sz[i] > 0)? 
				CL_BUILD_SUCCESS : CL_BUILD_ERROR;

		__do_create_program(prg);

		prg->refc = 1;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,cl_program);

	__success();

	return((cl_program)prg);
}


cl_int 
clRetainProgram( cl_program prg )
{
	DEBUG(__FILE__,__LINE__,"clRetainProgram");

	if (__invalid_program(prg)) return(CL_INVALID_VALUE);

	__retain_program(prg);

	return(CL_SUCCESS);
}


cl_int 
clReleaseProgram( cl_program prg )
{
	DEBUG(__FILE__,__LINE__,"clReleaseProgram");

	if (__invalid_program(prg)) return(CL_INVALID_VALUE);

	__retain_program(prg);

	return(CL_SUCCESS);
}


cl_int 
clBuildProgram(
	 cl_program prg,
	 cl_uint ndev,
	 const cl_device_id* devices,
	 const char* options,
	 void (*pfn_notify) (cl_program, void*),
	 void* user_data
)
{
	WARN(__FILE__,__LINE__,"clBuildProgram: warning: unsupported");

	if (__invalid_program(prg)) return(CL_INVALID_VALUE);

	DEBUG(__FILE__,__LINE__,"clBuildProgram: progam valid");

	if (!devices && ndev > 0) return(CL_INVALID_VALUE);

	if (devices && ndev == 0) return(CL_INVALID_VALUE);

	DEBUG(__FILE__,__LINE__,"clBuildProgram: device valid");

	if (!pfn_notify && user_data) return(CL_INVALID_VALUE);

	if (!devices) {
		ndev = prg->ndev;
		devices = prg->devices;
	}

	int i,j;
	int err;

	for(i=0;i<ndev;i++) {

		for(j=0;j<prg->ndev;j++) if (devices[i] == prg->devices[j]) break;

		if (j == prg->ndev) return(CL_INVALID_DEVICE);

		cl_device_id devid = devices[j];

		if (prg->src) {

			DEBUG(__FILE__,__LINE__,
				"compiler avail %d",__do_check_compiler_available(devices[j]));

			if (options) prg->build_options[j] = options;

			err = __do_build_program_from_source(prg,devid,j);

		} else {

			DEBUG2("bin bin_sz %p %d",prg->bin[j],prg->bin_sz[j]);

			if (!prg->bin[j] || prg->bin_sz[j] == 0) return(CL_INVALID_BINARY);

			err = __do_build_program_from_binary(prg,devid,j);

		}

		prg->build_stat[j] = (err==0)? CL_BUILD_SUCCESS : CL_BUILD_ERROR;

		if (err != CL_SUCCESS) return(err);
	}
	
	return(CL_SUCCESS);
}


cl_int 
clUnloadCompiler( void)
{
	WARN(__FILE__,__LINE__,"clUnloadCompiler: warning: unsupported");

	return(CL_ENOTSUP);
}


cl_int 
clGetProgramInfo(
	 cl_program prg,
	 cl_program_info param_name,
	 size_t param_sz, 
	 void* param_val,
	 size_t* param_sz_ret
)
{
	WARN(__FILE__,__LINE__,"clGetProgramInfo: warning: unsupported");

	if (__invalid_program(prg)) return(CL_INVALID_PROGRAM);

	size_t sz;

	switch (param_name) {

		case CL_PROGRAM_REFERENCE_COUNT:

			__case_get_param(sizeof(cl_uint),&prg->refc);

			break;

		case CL_PROGRAM_CONTEXT:

			__case_get_param(sizeof(cl_context),&prg->ctx);

			break;

		case CL_PROGRAM_NUM_DEVICES:

			__case_get_param(sizeof(cl_uint),&prg->ndev);

			break;

		case CL_PROGRAM_DEVICES:

			__case_get_param(prg->ndev*sizeof(cl_device_id),prg->devices);

			break;

		case CL_PROGRAM_SOURCE:

			__case_get_param(prg->src_sz,prg->src);

			break;

		case CL_PROGRAM_BINARY_SIZES:

			__case_get_param(prg->ndev*sizeof(size_t),prg->bin_sz);

			break;

		case CL_PROGRAM_BINARIES:

			__case_get_param(prg->ndev*sizeof(unsigned char*),prg->bin);

			break;

		default:

			return(CL_INVALID_VALUE);

	}

	return(CL_SUCCESS);
}


cl_int 
clGetProgramBuildInfo(
	 cl_program prg,
	 cl_device_id devid,
	 cl_program_build_info param_name,
	 size_t param_sz,
	 void* param_val,
	 size_t* param_sz_ret
)
{
	WARN(__FILE__,__LINE__,"clGetProgramBuildInfo: warning: unsupported");

	if (__invalid_program(prg)) return(CL_INVALID_PROGRAM);

	int j;
	size_t sz;

	for(j=0;j<prg->ndev;j++) if (devid == prg->devices[j]) break;

	switch (param_name) {

		case CL_PROGRAM_BUILD_STATUS:

			__case_get_param(sizeof(cl_build_status),&prg->build_stat[j]);
printf("%d\n",(int)prg->build_stat[j]);
			break;

		case CL_PROGRAM_BUILD_OPTIONS:

			sz = strnlen(prg->build_options,__CLMAXSTR_LEN);
//			__case_get_param(prg->build_options_sz,prg->build_options);
			__case_get_param(sz,prg->build_options);

			break;

		case CL_PROGRAM_BUILD_LOG:

			sz = strnlen(prg->build_log,__CLMAXSTR_LEN);
//			__case_get_param(prg->build_log_sz,prg->build_log);
			__case_get_param(sz,prg->build_log);

			break;

		default:

			return(CL_INVALID_VALUE);

	}

	return(CL_SUCCESS);
}

