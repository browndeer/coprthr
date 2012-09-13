/* ocl_platform.c 
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
#include <CL/cl_ext.h>

#include "xcl_structs.h"
#include "printcl.h"
#include "platform.h"

#define min(a,b) ((a<b)?a:b)


//void** __icd_call_vector;


// Platform API Calls


cl_int 
_clGetPlatformIDs(
	cl_uint nplatforms,
	cl_platform_id* platforms,
	cl_uint* nplatforms_ret
)
{
	if (nplatforms == 0 && platforms) return(CL_INVALID_VALUE);

	if (!platforms && !nplatforms_ret) return(CL_INVALID_VALUE);

	cl_uint nplatforms_avail;

	__do_discover_platforms();

	__do_get_nplatforms_avail(&nplatforms_avail);

	printcl( CL_DEBUG "nplatforms_avail %d\n",nplatforms_avail);

	if (nplatforms) nplatforms = min(nplatforms,nplatforms_avail);
	else nplatforms = nplatforms_avail;

	if (platforms) __do_get_platforms(nplatforms,platforms);

	if (nplatforms_ret) *nplatforms_ret = nplatforms;

	return(CL_SUCCESS);
}

char __suffix_str[] = "_coprthr";

cl_int 
_clGetPlatformInfo(
	cl_platform_id platformid, 
	cl_platform_info param_name,
	size_t param_sz, 
	void* param_val,
	size_t* param_sz_ret
) 
{
	fprintf(stderr,"IMP _clGetPlatformInfo\n"); fflush(stderr);
	
	if (__invalid_platform_id(platformid)) return(CL_INVALID_PLATFORM);

	char* p;
	size_t sz;

	switch (param_name) {

		case CL_PLATFORM_PROFILE:

			__do_get_platform_profile(platformid,&p);

			if (p) __case_get_param( strnlen(p,__CLMAXSTR_BUFSZ)+1,p);

			break;

		case CL_PLATFORM_VERSION:

			__do_get_platform_version(platformid,&p);

			if (p) __case_get_param( strnlen(p,__CLMAXSTR_BUFSZ)+1,p);

			break;

		case CL_PLATFORM_NAME:

			__do_get_platform_name(platformid,&p);

			if (p) __case_get_param( strnlen(p,__CLMAXSTR_BUFSZ)+1,p);

			break;

		case CL_PLATFORM_VENDOR:

			__do_get_platform_vendor(platformid,&p);

			if (p) __case_get_param( strnlen(p,__CLMAXSTR_BUFSZ)+1,p);

			break;

		case CL_PLATFORM_EXTENSIONS:

			__do_get_platform_extensions(platformid,&p);

			if (p) __case_get_param( strnlen(p,__CLMAXSTR_BUFSZ)+1,p);

			break;

		case CL_PLATFORM_ICD_SUFFIX_KHR:

			if (p) p = (void*)__suffix_str;
			break;

		default:

			return(CL_INVALID_VALUE);
	}

	return(CL_SUCCESS);
}



// Aliased Platform API Calls

cl_int
clGetPlatformIDs( cl_uint nplatforms, cl_platform_id* platforms,
   cl_uint* nplatforms_ret)
	__attribute__((alias("_clGetPlatformIDs")));

cl_int
clGetPlatformInfo( cl_platform_id platformid, cl_platform_info param_name,
   size_t param_sz, void* param_val, size_t* param_sz_ret)
	__attribute__((alias("_clGetPlatformInfo")));


