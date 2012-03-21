/* libocl.c
 *
 * Copyright (c) 2009-2011 Brown Deer Technology, LLC.  All Rights Reserved.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#define __USE_GNU
#include <dlfcn.h>

#include "util.h"
#include "libocl.h"
#include "oclcall.h"

#ifndef OPENCL_ICD_PATH
#define OPENCL_ICD_PATH "/etc/OpenCL/vendors"
#endif

#define min(a,b) ((a<b)?a:b)

static struct platform_struct _libocl_platforms[8] = { 0,0,0,0,0,0,0,0 };
static unsigned int _nplatforms = 0;


struct oclent_struct*
load_oclent( void* dlh )
{
	int n;

	DEBUG2("load_oclent:");

	size_t oclent_table_sz = oclncalls*oclent_sz;

	struct oclent_struct* oclent
		= (struct oclent_struct*)malloc(oclent_table_sz);

	memcpy(oclent,empty_oclent,oclent_table_sz);

	DEBUG2("oclncalls=%d",oclncalls);

	for(n=0;n<oclncalls;n++) {
		void* sym = dlsym(dlh,oclcallnames[n]);
		if (sym) oclent[n].ocl_call = sym;
//		DEBUG2("oclent[%d] load attempt '%s' %p",n,oclcallnames[n],sym);
	}

	return(oclent);
}


clGetPlatformIDs(
	cl_uint nplatforms,
   cl_platform_id* platforms,
   cl_uint* nplatforms_ret
)
{
	int n;
	char fullpath[256];
	DEBUG2("OPENCL_ICD_PATH '%s'",OPENCL_ICD_PATH);
//   DIR* dirp = opendir("/etc/OpenCL/vendors/");
   DIR* dirp = opendir( OPENCL_ICD_PATH );
   struct dirent* dp;
	_nplatforms = 0;
   if (dirp) while ( (dp=readdir(dirp)) ) {
//		strncpy(fullpath,"/etc/OpenCL/vendors/",256);
		strncpy(fullpath, OPENCL_ICD_PATH "/" ,256);
		strncat(fullpath,dp->d_name,256);
      DEBUG2("is this an icd file |%s|",fullpath);
      char* p;
      if ( (p=strrchr(dp->d_name,'.')) && !strncasecmp(p,".icd",5) ) {
         DEBUG2("this is an icd file |%s|",fullpath);
			struct stat st;
			stat(fullpath,&st);
			DEBUG2("size of icd file is %d",st.st_size);
			if (S_ISREG(st.st_mode) && st.st_size>0) {
				int fd = open(fullpath,O_RDONLY);
				read(fd,fullpath,st.st_size);
				close(fd);
				fullpath[strcspn(fullpath," \t\n")] = '\0';
         	DEBUG2("lib is |%s|",fullpath);
				void* h = dlopen(fullpath,RTLD_LAZY);
				DEBUG2("dlopen dlh=%p",h);
				if (h) {
					DEBUG2("dlopen successful");
					_libocl_platforms[_nplatforms].dlh = h;
					struct oclent_struct* oclent 
						= _libocl_platforms[_nplatforms].oclent = load_oclent(h);
					DEBUG2("platform [%d] oclent %p",_nplatforms,oclent);
					if (oclent!=0 && oclent[OCLCALL_clGetPlatformIDs].ocl_call) {
						int n = 0;
						DEBUG2("entry number %d",OCLCALL_clGetPlatformIDs);
						typedef cl_int (*pf_t)(cl_uint,cl_platform_id*,cl_uint*);
						((pf_t)oclent[OCLCALL_clGetPlatformIDs].ocl_call)(0,0,&n);
						DEBUG2("imp nplatforms %d",n);
						if (n != 1) {
							DEBUG2("vendor does not report nplatforms=1, skipping\n");
							continue;
						}
						((pf_t)oclent[OCLCALL_clGetPlatformIDs].ocl_call)(
							1,&_libocl_platforms[_nplatforms].imp_platform_id,0);
						*(void***)_libocl_platforms[_nplatforms].imp_platform_id 
							= (void**)oclent;
					}	
					++_nplatforms;
				}
			}
      }
   }
   closedir(dirp);

	DEBUG2("libocl: found %d platforms",_nplatforms);

	if (nplatforms_ret) *nplatforms_ret = _nplatforms;

	if (platforms) {
		for(n=0;n<min(nplatforms,_nplatforms);n++) 
			platforms[n] = _libocl_platforms[n].imp_platform_id;
	}

	return(CL_SUCCESS);
	
}


cl_int clGetDeviceIDs( 
	cl_platform_id a0,cl_device_type a1,cl_uint a2,cl_device_id* a3,cl_uint* a4
)
{
	struct oclent_struct* oclent = *(struct oclent_struct**)a0;
	typedef cl_int (*pf_t) (cl_platform_id,cl_device_type,cl_uint,
		cl_device_id*,cl_uint*);
	cl_int rv = ((pf_t)(oclent[OCLCALL_clGetDeviceIDs].ocl_call))(
		a0,a1,a2,a3,a4);
	int n=0;
	for(n=0;n<a2;n++,a3++) *(void***)(*a3) = (void**)oclent;
	return rv;
}


cl_context clCreateContext(
	const cl_context_properties* a0,
	cl_uint a1,const cl_device_id* a2,
	cl_pfn_notify_t a3,void* a4,cl_int* a5
)
{
	DEBUG2("clCreateContext:");
	cl_context_properties* p = (cl_context_properties*)a0;
	int n=0;
	for(;*p != 0 && n<256; p+=2,n++)
		if (*p == CL_CONTEXT_PLATFORM) { ++p; break; }
	if (*p==0 || n==256) return (cl_context)0;
	struct oclent_struct* oclent = *(struct oclent_struct**)(*p);
	DEBUG2("oclent=%p",oclent);
	typedef cl_context (*pf_t) (const cl_context_properties*,
		cl_uint,const cl_device_id*,cl_pfn_notify_t,void*,cl_int*);
	cl_context rv 
		= ((pf_t)(oclent[OCLCALL_clCreateContext].ocl_call))(
		a0,a1,a2,a3,a4,a5);
	*(void***)rv = (void**)oclent;
	return rv;
}


cl_context clCreateContextFromType(
	const cl_context_properties* a0,
	cl_device_type a1,cl_pfn_notify_t a2,void* a3,cl_int* a4
)
{
	DEBUG2("clCreateContextFromType:");
	cl_context_properties* p = (cl_context_properties*)a0;
	int n=0;
	for(;*p != 0 && n<256; p+=2,n++)
		if (*p == CL_CONTEXT_PLATFORM) { ++p; break; }
	if (*p==0 || n==256) return (cl_context)0;
	struct oclent_struct* oclent = *(struct oclent_struct**)(*p);
	DEBUG2("oclent=%p",oclent);
	typedef cl_context (*pf_t) (const cl_context_properties*,
		cl_device_type,cl_pfn_notify_t,void*,cl_int*);
	cl_context rv 
		= ((pf_t)(oclent[OCLCALL_clCreateContextFromType].ocl_call))(
		a0,a1,a2,a3,a4);
	*(void***)rv = (void**)oclent;
	return rv;
}


cl_int clGetContextInfo(
	cl_context a0,cl_context_info a1,size_t a2,void* a3,size_t* a4)
{
	int j;
	DEBUG2("clGetContextInfo:");
   struct oclent_struct* oclent
      = *(struct oclent_struct**)a0;
	DEBUG2(" oclent=%p",oclent);
   typedef cl_int (*pf_t) (cl_context,cl_context_info,size_t,void*,size_t*);
   cl_int rv
      = ((pf_t)(oclent[OCLCALL_clGetContextInfo].ocl_call))(a0,a1,a2,a3,a4);
	if (a1==CL_CONTEXT_DEVICES) for(j=0;j<a2/sizeof(cl_device_id);j++)
		*(void***)((cl_device_id*)a3)[j] = (void**)oclent;
   return rv;
}


cl_int clWaitForEvents(
	cl_uint a0,const cl_event* a1
)
{
	if (a0<1) return CL_INVALID_VALUE;
	if (!a1) return CL_INVALID_EVENT;
	struct oclent_struct* oclent = *(struct oclent_struct**)(*a1);
	typedef cl_int (*pf_t) (cl_uint,const cl_event*);
	cl_int rv 
		= ((pf_t)(oclent[OCLCALL_clWaitForEvents].ocl_call))(a0,a1);
	return rv;
}


cl_int clCreateKernelsInProgram(
	cl_program a0,cl_uint a1,cl_kernel* a2,cl_uint* a3
)
{
	struct oclent_struct* oclent = *(struct oclent_struct**)a0;
	typedef cl_int (*pf_t) (cl_program,cl_uint,cl_kernel*,cl_uint*);
	cl_int rv = ((pf_t)(oclent[OCLCALL_clCreateKernelsInProgram].ocl_call))(
		a0,a1,a2,a3);
	int n=0;
	for(n=0;n<a1;n++,a2++) *(void***)(*a2) = (void**)oclent;
	return rv;
}


cl_int clUnloadCompiler(void)
{ return (cl_int)CL_SUCCESS; }

