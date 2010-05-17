/* memobj.c 
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
#include "memobj.h"

void __do_create_memobj(cl_mem memobj) {}

void __do_release_memobj(cl_mem memobj) {}

void __do_create_buffer(cl_mem memobj) {

	unsigned int ndev = memobj->ctx->ndev;

	memobj->imp.res = (void**)calloc(ndev,sizeof(void*));
	memobj->imp.resmap = (void**)calloc(ndev,sizeof(void*));

}

void __do_create_image2d(cl_mem memobj) {

	unsigned int ndev = memobj->ctx->ndev;

	memobj->imp.res = (void**)calloc(ndev,sizeof(void*));
	memobj->imp.resmap = (void**)calloc(ndev,sizeof(void*));

	if (CL_MEM_READ_WRITE&memobj->flags) 
		DEBUG(__FILE__,__LINE__,"CL_MEM_READ_WRITE set");

	if (CL_MEM_READ_ONLY&memobj->flags) 
		DEBUG(__FILE__,__LINE__,"CL_MEM_READ_ONLY set");

	if (CL_MEM_WRITE_ONLY&memobj->flags) 
		DEBUG(__FILE__,__LINE__,"CL_MEM_WRITE_ONLY set");

	if (CL_MEM_USE_HOST_PTR&memobj->flags) 
		DEBUG(__FILE__,__LINE__,"CL_MEM_USE_HOST_PTR set");

	if (CL_MEM_ALLOC_HOST_PTR&memobj->flags) 
		DEBUG(__FILE__,__LINE__,"CL_MEM_ALLOC_HOST_PTR set");

	if (CL_MEM_COPY_HOST_PTR&memobj->flags) 
		DEBUG(__FILE__,__LINE__,"CL_MEM_COPY_HOST_PTR set");

}

