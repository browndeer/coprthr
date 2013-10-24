/* ocl_platform.c 
 *
 * Copyright (c) 2009-2013 Brown Deer Technology, LLC.  All Rights Reserved.
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
#include "device.h"
#include "version.h"
#include "cmdcall.h"

#define min(a,b) ((a<b)?a:b)


void __do_discover_platforms();
static void __do_release_platforms();
void __do_get_nplatforms_avail(cl_uint* n);
void __do_get_platforms(cl_uint n, cl_platform_id* platformid);
void __do_get_default_platformid( cl_platform_id* platformid );
static void __do_get_platform_profile(cl_platform_id platformid, char** p_str);
static void __do_get_platform_version(cl_platform_id platformid, char** p_str);
static void __do_get_platform_name(cl_platform_id platformid, char** p_str);
static void __do_get_platform_vendor(cl_platform_id platformid, char** p_str);

static void __do_get_platform_extensions( 
	cl_platform_id platformid, char** p_str);

static void __do_get_platform_icd_suffix_khr(
	cl_platform_id platformid, char** p_str);


// Platform API Calls


cl_int 
_clGetPlatformIDs(
	cl_uint nplatforms,
	cl_platform_id* platforms,
	cl_uint* nplatforms_ret
)
{
	printcl( CL_DEBUG "clGetPlatformIDs");
	
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
	printcl( CL_DEBUG "clGetPlatformInfo");
	
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



// internal platform implementation calls

/*
struct _strtab_entry {
   size_t alloc_sz;
   size_t sz;
   char* buf;
};
*/

/*
struct _clsymtab_entry {
   cl_uint kind;
   cl_uint type;
};
*/

static struct _cl_platform_id* __ptab = 0;
static unsigned int __nplatforms = 0;

static unsigned int __ndevices = 0;
static struct _cl_device_id* __dtab = 0;


void __do_discover_platforms()
{
   int i;

   if (__nplatforms > 0) return;

   __do_discover_devices(&__ndevices,&__dtab,0);

   __nplatforms = 1;
   __ptab = (struct _cl_platform_id*)malloc(sizeof(struct _cl_platform_id));

   __init_platform_id(__ptab);

	
   __ptab[0] = (struct _cl_platform_id){
		(void*)__icd_call_vector,
		"<profile>",
      COPRTHR_VERSION_STRING,
      "coprthr",
      "Brown Deer Technology, LLC.",
      "cl_khr_icd",
      __ndevices,__dtab
   };

   __init_platform_id(__ptab);

       for(i=0;i<__ndevices;i++)
          __dtab[0].ocldevinfo->platformid = __ptab;

}


static void __do_release_platforms()
{ 
	if (__dtab) free(__dtab);
}


void __do_get_nplatforms_avail(cl_uint* n)
{ *n = __nplatforms; }


void __do_get_platforms(cl_uint n, cl_platform_id* platformid)
{
   int i;
   for(i=0;i<n;i++) platformid[i] = &__ptab[i];
}


void __do_get_default_platformid( cl_platform_id* platformid )
{ *platformid = &__ptab[0]; }

static void __do_get_platform_profile(cl_platform_id platformid, char** p_str)
{ *p_str = __resolve_platformid(platformid,profile); }


static void __do_get_platform_version(cl_platform_id platformid, char** p_str)
{ *p_str = __resolve_platformid(platformid,version); }


static void __do_get_platform_name(cl_platform_id platformid, char** p_str)
{ *p_str = __resolve_platformid(platformid,name); }


static void __do_get_platform_vendor(cl_platform_id platformid, char** p_str)
{ *p_str = __resolve_platformid(platformid,vendor); }


static void __do_get_platform_extensions(cl_platform_id platformid, char** p_str)
{ *p_str = __resolve_platformid(platformid,extensions); }

static char __vendor_icd_ext_suffix[] = "\0";

static void __do_get_platform_icd_suffix_khr(
	cl_platform_id platformid, char** p_str)
{ *p_str = __vendor_icd_ext_suffix; }

