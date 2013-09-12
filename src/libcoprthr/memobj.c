/* memobj.c 
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
#include "memobj.h"

void __do_create_memobj(cl_mem memobj) {}

void __do_release_memobj(cl_mem memobj) 
{

	int i;
	cl_context ctx = memobj->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = memobj->ctx->devices;

	for(i=0;i<ndev;i++) {

		if (
			__resolve_devid_devinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_CPU
		) {

			if (memobj->imp->res[i]) free(memobj->imp->res[i]);

		} else if (
			__resolve_devid_devinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_GPU
		) {

			printcl( CL_WARNING "device unsupported, how did you get here?");

		} else {

		}

	}

}

void __do_create_buffer(cl_mem memobj) 
{
	int i;
	cl_context ctx = memobj->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = memobj->ctx->devices;

	memobj->imp->res = (void**)calloc(ndev,sizeof(void*));
	memobj->imp->resmap = (void**)calloc(ndev,sizeof(void*));

	printcl( CL_DEBUG "using static resource allocation across devices");

	for(i=0;i<ndev;i++) {

		if (
			__resolve_devid_devinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_CPU
		) {

//			memobj->imp->res[i] = malloc(memobj->sz);
			printcl("XXX memalloc %p",__resolve_devid_devops(ctx->devices[i],memalloc));
			memobj->imp->res[i] 
				= __resolve_devid_devops(ctx->devices[i],memalloc)(memobj->sz,0);

			if (memobj->sz > 0 && memobj->imp->res[i] == 0) {

				printcl( CL_WARNING "malloc failed");

			} else {

				printcl( CL_DEBUG "malloc'd %d bytes",memobj->sz);

			}

			if (memobj->flags&CL_MEM_COPY_HOST_PTR)
				memcpy(memobj->imp->res[i],memobj->host_ptr,memobj->sz);

			if (memobj->flags&CL_MEM_USE_HOST_PTR) {
				printcl( CL_WARNING 
					"workaround: CL_MEM_USE_HOST_PTR => CL_MEM_COPY_HOST_PTR,"
					" fix this");
				memcpy(memobj->imp->res[i],memobj->host_ptr,memobj->sz);
			}

		} else if (
			__resolve_devid_devinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_GPU
		) {

			printcl( CL_WARNING "device unsupported, how did you get here?");

		} else {

		}

	}

}


void __do_create_image2d(cl_mem memobj) 
{
	int i;
	cl_context ctx = memobj->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = memobj->ctx->devices;

	memobj->imp->res = (void**)calloc(ndev,sizeof(void*));
	memobj->imp->resmap = (void**)calloc(ndev,sizeof(void*));

	if (CL_MEM_READ_WRITE&memobj->flags) 
		printcl( CL_DEBUG "CL_MEM_READ_WRITE set");

	if (CL_MEM_READ_ONLY&memobj->flags) 
		printcl( CL_DEBUG "CL_MEM_READ_ONLY set");

	if (CL_MEM_WRITE_ONLY&memobj->flags) 
		printcl( CL_DEBUG "CL_MEM_WRITE_ONLY set");

	if (CL_MEM_USE_HOST_PTR&memobj->flags) 
		printcl( CL_DEBUG "CL_MEM_USE_HOST_PTR set");

	if (CL_MEM_ALLOC_HOST_PTR&memobj->flags) 
		printcl( CL_DEBUG "CL_MEM_ALLOC_HOST_PTR set");

	if (CL_MEM_COPY_HOST_PTR&memobj->flags) 
		printcl( CL_DEBUG "CL_MEM_COPY_HOST_PTR set");


	for(i=0;i<ndev;i++) {

		if (
			__resolve_devid_devinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_CPU
		) {

			/* XXX the 128 would take too long to explain -DAR */

			memobj->imp->res[i] = (void**)malloc(128 + memobj->sz);
			size_t* p = (size_t*)memobj->imp->res[i];
			p[0] = memobj->width;
			p[1] = memobj->height;

			if (memobj->sz > 0 && memobj->imp->res[i] == 0) {

				printcl( CL_WARNING "malloc failed");

			} else {

				printcl( CL_DEBUG "malloc'd %d bytes",memobj->sz);

			}

		} else if (
			__resolve_devid_devinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_GPU
		) {

			printcl( CL_WARNING "device unsupported, how did you get here?");

		} else {

		}

	}

}


// device mem operations

void* __coprthr_memalloc( size_t sz, int flags )
{ return malloc(sz); }

void* __coprthr_memrealloc( void** p_memptr, size_t sz, int flags)
{ return realloc(p_memptr,sz); }

void __coprthr_memfree( void* memptr, int flags )
{ free(memptr); }

size_t __coprthr_memread( void* memptr, void* buf, size_t sz )
{ memcpy(memptr,buf,sz); return sz; }

size_t __coprthr_memwrite( void* memptr, void* buf, size_t sz )
{ memcpy(buf,memptr,sz); return sz; }

size_t __coprthr_memcopy( void* memptr_src, void* memptr_dst, size_t sz)
{ memcpy(memptr_dst,memptr_src,sz); return sz; }

