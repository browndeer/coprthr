/* ocl_program.c 
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
#include "compiler.h"

void __do_create_program(cl_program prg);
void __do_release_program(cl_program prg);

cl_int __do_build_program_from_binary( cl_program prg,cl_device_id devid, 
	cl_uint devnum);

cl_int __do_build_program_from_source( cl_program prg,cl_device_id devid, 
	cl_uint devnum);

int __do_find_kernel_in_program( cl_program prg, const char* kname );
int __do_check_compiler_available( cl_device_id devid );


// Program Object APIs

cl_program 
_clCreateProgramWithSource(
	 cl_context ctx,
	 cl_uint count,
	 const char** strings,
	 const size_t* lens,
	 cl_int* err_ret
)
{
	printcl( CL_DEBUG "clCreateProgramWithSource");

	if (__invalid_context(ctx)) __error_return(CL_INVALID_CONTEXT,cl_program);

	if (count==0 || !strings) __error_return(CL_INVALID_VALUE,cl_program);

	int i;

	for(i=0;i<count;i++) 
		if (!strings[i]) __error_return(CL_INVALID_VALUE,cl_program);


	cl_uint ndev = ctx->ndev;

	struct _cl_program* prg 
		= (struct _cl_program*)malloc(sizeof(struct _cl_program));

	if (prg) {

		__init_program(prg);

		prg->prg1 = (struct coprthr_program**)
			calloc(ndev,sizeof(struct coprthr_program*));

		int i;
		for(i=0;i<ndev;i++)
			prg->prg1[i] = (struct coprthr_program*)
				calloc(1,sizeof(struct coprthr_program));

//		int i;

		size_t sz = 0;

		for(i=0;i<count;i++) {
			sz += (lens && lens[i] > 0)? lens[i] : strlen(strings[i]);
		}
		
		prg->src_sz = sz;
		char* src = prg->src = (unsigned char*)malloc((sz+1)*sizeof(unsigned char));

		if (!src) {
			__free_program(prg);
			__error_return(CL_OUT_OF_HOST_MEMORY,cl_program);
		}

		unsigned char* p = src;	
		for(i=0;i<count;i++) {
			sz = (lens && lens[i] > 0)? lens[i] : strlen(strings[i]);
			strncpy(p,strings[i],sz);
			p += sz;
		}

		prg->ctx = ctx;

		prg->ndev = ndev;
		__clone(prg->devices,ctx->devices,ndev,cl_device_id);

		for(i=0; i<ndev; i++) prg->prg1[i]->bin_stat = CL_BUILD_NONE;

		__do_create_program(prg);

		for(i=0;i<ndev;i++) {
			prg->prg1[i]->src = src;
			prg->prg1[i]->src_sz = sz;
			prg->prg1[i]->bin_stat = 0;
			prg->prg1[i]->bin = 0;
			prg->prg1[i]->bin_sz = 0;
			prg->prg1[i]->build_opt = 0;
			prg->prg1[i]->build_log = 0;
		}

		prg->refc = 1;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,cl_program);

	__success();
	
	return((cl_program)prg);
}


cl_program 
_clCreateProgramWithBinary(
	 cl_context ctx,
	 cl_uint ndev,
	 const cl_device_id* devices,
	 const size_t* lens,
	 const unsigned char** bins,
	 cl_int* stat,
	 cl_int* err_ret
)
{
	printcl( CL_DEBUG "clCreateProgramWithBinary");

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

	printcl( CL_DEBUG "here");

	if (prg) {

		__init_program(prg);

		prg->prg1 = (struct coprthr_program**)
			calloc(ndev,sizeof(struct coprthr_program*));

		int i;
		for(i=0;i<ndev;i++)
			prg->prg1[i] = (struct coprthr_program*)
				calloc(1,sizeof(struct coprthr_program));

		prg->ctx = ctx;

		prg->ndev = ndev;
		__clone(prg->devices,devices,ndev,cl_device_id);

		__do_create_program(prg);

		for(i=0;i<ndev;i++) {

			prg->prg1[i]->src = 0;
			prg->prg1[i]->src_sz = 0;
		
			printcl( CL_DEBUG "is this ELF |%c%c%c|",
				bins[i][1], bins[i][2], bins[i][3]);

			prg->prg1[i]->bin = bins[i];
			prg->prg1[i]->bin_sz = lens[i];

			prg->prg1[i]->bin_stat 
				= (prg->prg1[i]->bin && prg->prg1[i]->bin_sz > 0)?  
					CL_BUILD_SUCCESS : CL_BUILD_ERROR;

			prg->prg1[i]->build_opt = 0;
			prg->prg1[i]->build_log = 0;
		}

		prg->refc = 1;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,cl_program);

	__success();

	return((cl_program)prg);
}


cl_int 
_clRetainProgram( cl_program prg )
{
	printcl( CL_DEBUG "clRetainProgram");

	if (__invalid_program(prg)) return(CL_INVALID_VALUE);

	__retain_program(prg);

	return(CL_SUCCESS);
}


cl_int 
_clReleaseProgram( cl_program prg )
{
	printcl( CL_DEBUG "clReleaseProgram");

	if (__invalid_program(prg)) return(CL_INVALID_VALUE);

	__release_program(prg);

	return(CL_SUCCESS);
}


cl_int 
_clBuildProgram(
	 cl_program prg,
	 cl_uint ndev,
	 const cl_device_id* devices,
	 const char* options,
	 void (*pfn_notify) (cl_program, void*),
	 void* user_data
)
{

	if (__invalid_program(prg)) return(CL_INVALID_VALUE);

	printcl( CL_DEBUG "clBuildProgram: progam valid");

	if (!devices && ndev > 0) return(CL_INVALID_VALUE);

	if (devices && ndev == 0) return(CL_INVALID_VALUE);

	printcl( CL_DEBUG "clBuildProgram: device valid");

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

		if (prg->prg1[i]->src) {

			printcl( CL_DEBUG 
				"compiler avail %d",__do_check_compiler_available(devices[j]));

			if (options) prg->prg1[i]->build_opt = options;

			err = __do_build_program_from_source(prg,devid,j);

		} else {

			printcl( CL_DEBUG "bin bin_sz %p %d",
				prg->prg1[i]->bin, prg->prg1[i]->bin_sz);

			if (!prg->prg1[i]->bin || prg->prg1[i]->bin_sz == 0) 
				return(CL_INVALID_BINARY);

			printcl( CL_DEBUG "is this ELF |%c %c %c",
				prg->prg1[i]->bin[1], prg->prg1[i]->bin[2], prg->prg1[i]->bin[3]);
			
			err = __do_build_program_from_binary(prg,devid,j);

		}

		prg->prg1[i]->bin_stat = (err==0)? CL_BUILD_SUCCESS : CL_BUILD_ERROR;

	}
	
	if (err)
		printcl( CL_WARNING "clBuildProgram failed with err %d",err);

	return(err);
}


cl_int _clUnloadCompiler( void)
{
	printcl( CL_WARNING "clUnloadCompiler: warning: unsupported");

	return(CL_ENOTSUP);
}


cl_int 
_clGetProgramInfo(
	 cl_program prg,
	 cl_program_info param_name,
	 size_t param_sz, 
	 void* param_val,
	 size_t* param_sz_ret
)
{

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

			__case_get_param(prg->prg1[0]->src_sz,prg->prg1[0]->src);

			break;

		case CL_PROGRAM_BINARY_SIZES:

			{
				sz = prg->ndev*sizeof(size_t);
				if (param_sz_ret) *param_sz_ret = sz;
				if (param_sz < sz && param_val) return(CL_INVALID_VALUE);
				else if (param_val) {
					int j;
					for(j=0;j<prg->ndev;j++) 
						((size_t*)param_val)[j] = prg->prg1[j]->bin_sz;
				}
			}
			break;

		case CL_PROGRAM_BINARIES:

			{
				sz = prg->ndev*sizeof(char*);
				if (param_sz_ret) *param_sz_ret = sz;
				if (param_sz < sz && param_val) return(CL_INVALID_VALUE);
				else if (param_val) {
					int j;
					for(j=0;j<prg->ndev;j++) {
						((char**)param_val)[j] = prg->prg1[j]->bin;
						printcl( CL_DEBUG "is this ELF |%c %c %c|",
							prg->prg1[j]->bin[1],
							prg->prg1[j]->bin[2],
							prg->prg1[j]->bin[3]);
					}
				}
			}
			break;

		default:

			return(CL_INVALID_VALUE);

	}

	return(CL_SUCCESS);
}


cl_int 
_clGetProgramBuildInfo(
	 cl_program prg,
	 cl_device_id devid,
	 cl_program_build_info param_name,
	 size_t param_sz,
	 void* param_val,
	 size_t* param_sz_ret
)
{

	if (__invalid_program(prg)) return(CL_INVALID_PROGRAM);

	int j;
	size_t sz;

	for(j=0;j<prg->ndev;j++) if (devid == prg->devices[j]) break;

	switch (param_name) {

		case CL_PROGRAM_BUILD_STATUS:

			__case_get_param(sizeof(cl_build_status),&prg->prg1[j]->bin_stat);
			break;

		case CL_PROGRAM_BUILD_OPTIONS:

			sz = strnlen(prg->prg1[j]->build_opt,__CLMAXSTR_LEN);
			__case_get_param(sz,prg->prg1[j]->build_opt);

			break;

		case CL_PROGRAM_BUILD_LOG:

			printcl( CL_DEBUG "CL_PROGRAM_BUILD_LOG: build_log=%p", 
				prg->prg1[j]->build_log);

			sz = strnlen(prg->prg1[j]->build_log,__CLMAXSTR_LEN);
			__case_get_param(sz,prg->prg1[j]->build_log);

			break;

		default:

			return(CL_INVALID_VALUE);

	}

	return(CL_SUCCESS);
}


// Aliased Program API Calls

cl_program
clCreateProgramWithSource( cl_context ctx, cl_uint count, const char** strings,
    const size_t* lens, cl_int* err_ret)
	__attribute__((alias("_clCreateProgramWithSource")));

cl_program
clCreateProgramWithBinary( cl_context ctx, cl_uint ndev, 
	const cl_device_id* devices, const size_t* lens, const unsigned char** bins,
    cl_int* stat, cl_int* err_ret)
	__attribute__((alias("_clCreateProgramWithBinary")));

cl_int
clRetainProgram( cl_program prg )
	__attribute__((alias("_clRetainProgram")));

cl_int
clReleaseProgram( cl_program prg )
	__attribute__((alias("_clReleaseProgram")));

cl_int
clBuildProgram( cl_program prg, cl_uint ndev, const cl_device_id* devices,
    const char* options, void (*pfn_notify) (cl_program, void*),
    void* user_data)
	__attribute__((alias("_clBuildProgram")));

cl_int
clUnloadCompiler( void)
	__attribute__((alias("_clUnloadCompiler")));

cl_int
clGetProgramInfo( cl_program prg, cl_program_info param_name, size_t param_sz,
    void* param_val, size_t* param_sz_ret)
	__attribute__((alias("_clGetProgramInfo")));

cl_int
clGetProgramBuildInfo( cl_program prg, cl_device_id devid, 
	cl_program_build_info param_name, size_t param_sz, void* param_val,
    size_t* param_sz_ret)
	__attribute__((alias("_clGetProgramBuildInfo")));


/*
 * Internal program implementation calls
 */

void __do_create_program(cl_program prg)
{
   printcl( CL_DEBUG "__do_create_program with ndev = %d",prg->ndev);

}


void __do_release_program(cl_program prg)
{
   int i;
   for(i=0;i<prg->ndev;i++) {
      __do_release_program_1(prg->prg1[i]);
   }

}

cl_int __do_build_program_from_binary(
   cl_program prg,cl_device_id devid, cl_uint devnum
){
   printcl( CL_DEBUG "__do_build_program_from_binary");
   int retval = __do_build_program_from_binary_1(prg->prg1[devnum]);
   prg->nkrn = prg->prg1[0]->nkrn;
   return(retval);
}

cl_int __do_build_program_from_source(
   cl_program prg,cl_device_id devid, cl_uint devnum
){

   printcl( CL_DEBUG "__do_build_program_from_source");

   compiler_t comp = (compiler_t)__resolve_devid_devcomp(devid,comp);

   if (!comp) return(CL_COMPILER_NOT_AVAILABLE);

   printcl( CL_DEBUG
      "__do_build_program_from_source: compiler=%p",comp);

   /* XXX should optimize JIT by checking for device equivalence -DAR */

   printcl( CL_DEBUG "build_options[%d] |%s|",
      devnum,prg->prg1[devnum]->build_opt);

	printcl( CL_DEBUG "calling compiler : [%d] %p %p",devnum,&prg->prg1[devnum]->bin,&prg->prg1[devnum]->bin_sz);

   int err = comp( devid, prg->prg1[devnum]->src,prg->prg1[devnum]->src_sz,
      &prg->prg1[devnum]->bin,
      &prg->prg1[devnum]->bin_sz, prg->prg1[devnum]->build_opt,
      &prg->prg1[devnum]->build_log);

   if (!err) err = __do_build_program_from_binary(prg,devid,devnum);

   return((cl_int)err);
}

/* XXX this routine uses devnum 0 as a hack, fix it -DAR */
int __do_find_kernel_in_program( cl_program prg, const char* kname )
{
   int k;

   printcl( CL_DEBUG "nkrn %d",prg->prg1[0]->nkrn);

   for(k=0;k<prg->prg1[0]->nkrn;k++) {
      printcl( CL_DEBUG "compare |%s|%s\n",prg->prg1[0]->kname[k],kname);
      if (!strncmp(prg->prg1[0]->kname[k],kname,__CLMAXSTR_LEN)) break;
   }

   if (k==prg->prg1[0]->nkrn) return(-1);

   return(k);
}

int __do_check_compiler_available( cl_device_id devid )
{

   if (!__resolve_devid_devcomp(devid,comp)) return(0);

   return(1);
}

