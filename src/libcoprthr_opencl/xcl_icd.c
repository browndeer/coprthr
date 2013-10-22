/* xcl_icd.c 
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

#include "xcl_structs.h"
#include "printcl.h"

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif


// ICD stuff 


cl_int 
clIcdGetPlatformIDsKHR( 
	cl_uint nplatforms, 
	cl_platform_id *platforms, 
	cl_uint *nplatforms_ret
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


void*
clGetExtensionFunctionAddress( const char* funcname )
{
	if (!funcname) return 0;

	if (!strcmp("clIcdGetPlatformIDsKHR",funcname) )
		return &clIcdGetPlatformIDsKHR;

	return 0;
}


