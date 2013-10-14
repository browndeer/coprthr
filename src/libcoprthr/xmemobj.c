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

#include "xdevice.h"

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

			printcl( CL_DEBUG "__do_release_memobj: %p",memobj->mem1[i]);
			printcl( CL_DEBUG "__do_release_memobj: %p",memobj->mem1[i]->res);

			if (memobj->mem1[i]->res) free(memobj->mem1[i]->res);

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

	memobj->mem1 = (struct coprthr1_mem**)
		malloc(ndev*sizeof(struct coprthr1_mem*));

	printcl( CL_DEBUG "using static resource allocation across devices");

	for(i=0;i<ndev;i++) {

		if (
			__resolve_devid_devinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_CPU
		) {

			printcl("XXX memalloc %p",
				__resolve_devid_devops(ctx->devices[i],memalloc));
/*
			memobj->mem1[i] = (struct coprthr1_mem*)
				malloc(sizeof(struct coprthr1_mem));
			memobj->mem1[i]->res 
				= __resolve_devid_devops(ctx->devices[i],memalloc)(memobj->sz,0);
*/
			memobj->mem1[i] = (struct coprthr1_mem*)
				__resolve_devid_devops(ctx->devices[i],memalloc)(memobj->sz,0);
			memobj->mem1[i]->sz = memobj->sz;

			if (memobj->sz > 0 && memobj->mem1[i]->res == 0) {

				printcl( CL_WARNING "malloc failed");

			} else {

				printcl( CL_DEBUG "malloc'd %d bytes",memobj->sz);

			}

			if (memobj->flags&CL_MEM_COPY_HOST_PTR)
				memcpy(memobj->mem1[i]->res,memobj->host_ptr,memobj->sz);

			if (memobj->flags&CL_MEM_USE_HOST_PTR) {
				printcl( CL_WARNING 
					"workaround: CL_MEM_USE_HOST_PTR => CL_MEM_COPY_HOST_PTR,"
					" fix this");
				memcpy(memobj->mem1[i]->res,memobj->host_ptr,memobj->sz);
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

			memobj->mem1[i]->res = (void**)malloc(128 + memobj->sz);
			size_t* p = (size_t*)memobj->mem1[i]->res;
			p[0] = memobj->width;
			p[1] = memobj->height;
			memobj->mem1[i]->sz = memobj->sz + 128;

			if (memobj->sz > 0 && memobj->mem1[i]->res == 0) {

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
//{ return malloc(sz); }
{
	struct coprthr1_mem* mem1 = (struct coprthr1_mem*)
		malloc(sizeof(struct coprthr1_mem));
	mem1->res = malloc(sz);
	return(mem1); 
}

void* __coprthr_memrealloc( void* ptr, size_t sz, int flags)
{ return realloc(ptr,sz); }

void __coprthr_memfree( void* dptr, int flags )
{
	struct coprthr1_mem* mem1 = (struct coprthr1_mem*)dptr; 
	if (mem1) {
		if (mem1->res) free(mem1->res);
		free(mem1);
	}
}

size_t __coprthr_memread( void* memptr, void* buf, size_t sz )
{ memcpy(memptr,buf,sz); return sz; }

size_t __coprthr_memwrite( void* memptr, void* buf, size_t sz )
{ memcpy(buf,memptr,sz); return sz; }

size_t __coprthr_memcopy( void* memptr_src, void* memptr_dst, size_t sz)
{ memcpy(memptr_dst,memptr_src,sz); return sz; }



void* coprthr_dmalloc( int dd, size_t sizeb, int flags )
{
	if (dd < 256 && __ddtab[dd]) 
		return(__ddtab[dd]->devops->memalloc(sizeb,flags));
	else 
		return(0);
}

void coprthr_dfree( int dd, void* ptr )
{
	if (dd < 256 && __ddtab[dd]) 
		__ddtab[dd]->devops->memfree(ptr,0);
}

void* coprthr_drealloc( int dd, void* dptr, size_t sizeb, int flags )
{
	if (dd < 256 && __ddtab[dd]) 
		return(__ddtab[dd]->devops->memrealloc(dptr,sizeb,flags));
	else 
		return(0);
}

/*
size_t coprthr_dmwrite( int dd, void* dptr, void* buf, size_t sizeb)
{
	if (dd < 256 && __ddtab[dd]) 
		return(__ddtab[dd]->devops->memwrite(dptr,buf,sizeb));
	else 
		return(0);
}
*/

/*
size_t coprthr_dmread( int dd, void* dptr, void* buf, size_t sizeb)
{
	if (dd < 256 && __ddtab[dd]) 
		return(__ddtab[dd]->devops->memread(dptr,buf,sizeb));
	else 
		return(0);
}
*/

void* coprthr_dmmap( int dd, void* dptr, size_t sizeb, int flags, void* ptr )
{
}

int coprthr_dmunmap( void* dptr, size_t sizeb )
{
}

int coprthr_dmsync( void* ptr, size_t sizeb, int flags )
{
}


