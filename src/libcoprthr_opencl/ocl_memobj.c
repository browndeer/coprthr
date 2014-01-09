/* ocl_memobj.c 
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

#include "coprthr.h"

#ifndef max
#define max(a,b) ((a<b)?b:a)
#endif

static void __do_get_max_buffer_size_in_context(cl_context ctx, size_t* sz)
{
   int i;
   size_t tmp = 0;
   for(i=0;i<ctx->ndev;i++)
      tmp = max(tmp,
			__resolve_devid_ocldevinfo(ctx->devices[i],max_mem_alloc_sz));
   *sz = tmp;
}

void __do_create_memobj(cl_mem memobj);
void __do_release_memobj(cl_mem memobj);
void __do_create_buffer(cl_mem memobj);
void __do_create_image2d(cl_mem memobj);


// Memory Object API Calls

cl_mem 
_clCreateBuffer(
	cl_context ctx,
	cl_mem_flags flags,
	size_t size,
	void* host_ptr,
	cl_int* err_ret
) 
{
	printcl( CL_DEBUG "clCreateBuffer");

	if (__invalid_context(ctx)) __error_return(CL_INVALID_CONTEXT,cl_mem);

	/* XXX test flags and return CL_INVALID_VALUE if invalid -DAR */

	int permc = 0;
	if (CL_MEM_READ_WRITE&flags) ++permc;
	if (CL_MEM_READ_ONLY&flags) ++permc;
	if (CL_MEM_WRITE_ONLY&flags) ++permc;
	if (permc > 1) __error_return(CL_INVALID_VALUE,cl_mem);
	if (permc == 0) flags |= CL_MEM_READ_WRITE;


	if (size == 0) __error_return(CL_INVALID_BUFFER_SIZE,cl_mem);

	size_t max_buffer_size;
	__do_get_max_buffer_size_in_context(ctx,&max_buffer_size);
	if (size > max_buffer_size) __error_return(CL_INVALID_BUFFER_SIZE,cl_mem);

	/* XXX if not RO or WO then force default RW */

	cl_mem_flags f_use_host_ptr = flags&CL_MEM_USE_HOST_PTR;
	cl_mem_flags f_copy_host_ptr = flags&CL_MEM_COPY_HOST_PTR;

	if (host_ptr && (!f_use_host_ptr && !f_copy_host_ptr))
		__error_return(CL_INVALID_HOST_PTR,cl_mem);

	if (!host_ptr && (f_use_host_ptr || f_copy_host_ptr))
		__error_return(CL_INVALID_HOST_PTR,cl_mem);


	struct _cl_mem* membuf = (struct _cl_mem*)malloc(sizeof(struct _cl_mem));

	printcl( CL_DEBUG "malloc returned %p",membuf);


	if (membuf) {

		__init_memobj(membuf);
		membuf->mem1 = (struct coprthr_mem**)
			malloc(ctx->ndev*sizeof(struct coprthr_mem*));

		membuf->ctx = ctx;
		membuf->sz = size;
		membuf->host_ptr = host_ptr;
		membuf->type = CL_MEM_OBJECT_BUFFER;
		membuf->flags = flags;
		membuf->refc = 1;
		membuf->mapc = 0;

		__do_create_buffer(membuf);

		__error(CL_SUCCESS);

	} else __error_return(CL_OUT_OF_HOST_MEMORY,cl_mem);

	return(membuf);
}


 
cl_mem 
_clCreateImage2D(
	cl_context ctx,
	cl_mem_flags flags,
	const cl_image_format* img_format,
	size_t img_width,
	size_t img_height,
	size_t img_row_pitch, 
	void* host_ptr,
	cl_int* err_ret
) 
{
	printcl( CL_DEBUG "clCreateImage2D");

	if (__invalid_context(ctx)) __error_return(CL_INVALID_CONTEXT,cl_mem);

	/* XXX test flags and return CL_INVALID_VALUE if invalid -DAR */

	int permc = 0;
	if (CL_MEM_READ_WRITE&flags) ++permc;
	if (CL_MEM_READ_ONLY&flags) ++permc;
	if (CL_MEM_WRITE_ONLY&flags) ++permc;
	if (permc > 1) __error_return(CL_INVALID_VALUE,cl_mem);
	if (permc == 0) flags |= CL_MEM_READ_WRITE;

	if (img_format == 0) 
		__error_return(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,cl_mem);

	/* XXX test img_format is valid */

	if (img_width == 0 || img_height == 0) 
		__error_return(CL_INVALID_IMAGE_SIZE,cl_mem);

	/* XXX check width,height against all devices in ctx */
	//size_t max_buffer_size;
	//__do_get_max_buffer_size_in_context(ctx,&max_buffer_size);
	//if (size > max_buffer_size) __error_return(CL_INVALID_BUFFER_SIZE,cl_mem);

	if (host_ptr == 0 && img_row_pitch != 0) 
		__error_return(CL_INVALID_IMAGE_SIZE,cl_mem);

	/* XXX more tests for img_row_pitch */

	cl_mem_flags f_use_host_ptr = flags&CL_MEM_USE_HOST_PTR;
	cl_mem_flags f_copy_host_ptr = flags&CL_MEM_COPY_HOST_PTR;

	if (host_ptr && (!f_use_host_ptr && !f_copy_host_ptr))
		__error_return(CL_INVALID_HOST_PTR,cl_mem);

	if (!host_ptr && (f_use_host_ptr || f_copy_host_ptr))
		__error_return(CL_INVALID_HOST_PTR,cl_mem);

	/* XXX check if image format not supported */

	struct _cl_mem* membuf = (struct _cl_mem*)malloc(sizeof(struct _cl_mem));

	if (membuf) {

		__init_memobj(membuf);
		membuf->mem1 = (struct coprthr_mem**)
			malloc(ctx->ndev*sizeof(struct coprthr_mem*));

		membuf->ctx = ctx;
		/* XXX for now hardcoded to int4/float4 datatype -DAR */
		membuf->sz = img_width * img_height * 4 * sizeof(float);
		membuf->width = img_width;
		membuf->height = img_height;
		membuf->pitch = img_row_pitch;
		membuf->host_ptr = host_ptr;
		membuf->type = CL_MEM_OBJECT_IMAGE2D;
		membuf->flags = flags;
		membuf->refc = 1;
		membuf->mapc = 0;

		/* XXX alloc res, resmap array over devices in ctx */

		__do_create_image2d(membuf);

		__error(CL_SUCCESS);

	} else __error_return(CL_OUT_OF_HOST_MEMORY,cl_mem);

	return(membuf);

}  
                      
 
cl_mem 
_clCreateImage3D(
	cl_context ctx,
	cl_mem_flags flags,
	const cl_image_format* img_format,
	size_t img_width, 
	size_t img_height,
	size_t img_depth, 
	size_t img_row_pitch, 
	size_t img_slice_pitch, 
	void* host_ptr,
	cl_int* err_ret
) 
{
	printcl( CL_WARNING "clCreateImage3D: warning: unsupported");

	__error_return(CL_ENOTSUP,cl_mem);
}

                        
 
cl_int 
_clRetainMemObject(cl_mem memobj) 
{
	printcl( CL_DEBUG "clRetainMemObject");

	if (!memobj) return(CL_INVALID_MEM_OBJECT);

	__retain_memobj(memobj);

	return(CL_SUCCESS);
}


cl_int 
_clReleaseMemObject(cl_mem memobj)
{
	printcl( CL_DEBUG "clReleaseMemObject");

	if (__invalid_memobj(memobj)) return(CL_INVALID_MEM_OBJECT);

	__release_memobj(memobj);

	return(CL_SUCCESS);
}


cl_int 
_clGetSupportedImageFormats(
	cl_context ctx,
	cl_mem_flags flags,
	cl_mem_object_type imgtype,
	cl_uint nimgfmt,
	cl_image_format* imgfmt,
	cl_uint* nimgfmt_ret)
{
	printcl( CL_WARNING "clGetSupportedImageFormats: warning: unsupported");

	return(CL_ENOTSUP);
}

                                    
cl_int 
_clGetMemObjectInfo(
	cl_mem memobj,
   cl_mem_info param_name, 
	size_t param_sz,
	void* param_val,
	size_t* param_sz_ret
) 
{
	printcl( CL_DEBUG "clGetMemObjectInfo");

	if (__invalid_memobj(memobj)) return(CL_INVALID_MEM_OBJECT);

	size_t sz;

	switch (param_name) {

		case CL_MEM_TYPE:

			__case_get_param(sizeof(cl_mem_object_type),&memobj->type);

			break;

		case CL_MEM_FLAGS:

			__case_get_param(sizeof(cl_mem_flags),&memobj->flags);

			break;

		case CL_MEM_SIZE:

			__case_get_param(sizeof(size_t),&memobj->sz);

			break;

		case CL_MEM_HOST_PTR:

			__case_get_param(sizeof(void*),&memobj->host_ptr);

			break;

		case CL_MEM_MAP_COUNT:

			__case_get_param(sizeof(cl_uint),&memobj->mapc);

			break;

		case CL_MEM_REFERENCE_COUNT:

			__case_get_param(sizeof(cl_uint),&memobj->refc);

			break;

		case CL_MEM_CONTEXT:

			__case_get_param(sizeof(cl_context),&memobj->ctx);

			break;

		default:

			return(CL_INVALID_VALUE);

	}

	return(CL_SUCCESS);
}


cl_int 
_clGetImageInfo(
	cl_mem image,
	cl_image_info param_name, 
	size_t param_sz,
	void* param_val,
	size_t* param_sz_ret
)
{
	printcl( CL_WARNING "clGetImageInfo: warning: unsupported");

	if (__invalid_memobj(image)) return(CL_INVALID_MEM_OBJECT);

	return(CL_ENOTSUP);
}



// Aliased Memory Object API Calls

cl_mem
clCreateBuffer( cl_context ctx, cl_mem_flags flags, size_t size,
   void* host_ptr, cl_int* err_ret)
	__attribute__((alias("_clCreateBuffer")));

cl_mem
clCreateImage2D( cl_context ctx, cl_mem_flags flags, 
	const cl_image_format* img_format, size_t img_width, size_t img_height,
   size_t img_row_pitch, void* host_ptr, cl_int* err_ret)
	__attribute__((alias("_clCreateImage2D")));

cl_mem
clCreateImage3D( cl_context ctx, cl_mem_flags flags, 
	const cl_image_format* img_format, size_t img_width, size_t img_height,
   size_t img_depth, size_t img_row_pitch, size_t img_slice_pitch,
   void* host_ptr, cl_int* err_ret)
	__attribute__((alias("_clCreateImage3D")));

cl_int
clRetainMemObject(cl_mem memobj)
	__attribute__((alias("_clRetainMemObject")));

cl_int
clReleaseMemObject(cl_mem memobj)
	__attribute__((alias("_clReleaseMemObject")));

cl_int
clGetSupportedImageFormats( cl_context ctx, cl_mem_flags flags,
   cl_mem_object_type imgtype, cl_uint nimgfmt, cl_image_format* imgfmt,
   cl_uint* nimgfmt_ret)
	__attribute__((alias("_clGetSupportedImageFormats")));

cl_int
clGetMemObjectInfo( cl_mem memobj, cl_mem_info param_name, size_t param_sz,
   void* param_val, size_t* param_sz_ret) 
	__attribute__((alias("_clGetMemObjectInfo")));

cl_int
clGetImageInfo( cl_mem image, cl_image_info param_name, size_t param_sz,
   void* param_val, size_t* param_sz_ret)
	__attribute__((alias("_clGetImageInfo")));


void __do_create_memobj(cl_mem memobj) {}

void __do_release_memobj(cl_mem memobj) 
{

	int i;
	cl_context ctx = memobj->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = memobj->ctx->devices;

	for(i=0;i<ndev;i++) {

		if (
			__resolve_devid_ocldevinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_CPU
		) {

			printcl( CL_DEBUG "__do_release_memobj: %p",memobj->mem1[i]);
			printcl( CL_DEBUG "__do_release_memobj: %p",memobj->mem1[i]->res);

			if (memobj->mem1[i]->res) free(memobj->mem1[i]->res);

		} else if (
			__resolve_devid_ocldevinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_GPU
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

	memobj->mem1 = (struct coprthr_mem**)
		malloc(ndev*sizeof(struct coprthr_mem*));

	printcl( CL_DEBUG "using static resource allocation across devices");

	for(i=0;i<ndev;i++) {

//		if (
//			__resolve_devid_ocldevinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_CPU
//		) {

			printcl( CL_DEBUG "XXX memalloc %p",
				__resolve_devid_devops(ctx->devices[i],memalloc));
/*
			memobj->mem1[i] = (struct coprthr_mem*)
				malloc(sizeof(struct coprthr_mem));
			memobj->mem1[i]->res 
				= __resolve_devid_devops(ctx->devices[i],memalloc)(memobj->sz,0);
*/
			memobj->mem1[i] = (struct coprthr_mem*)
//				__resolve_devid_devops(ctx->devices[i],memalloc)(memobj->sz,0);
				__resolve_devid_devops(ctx->devices[i],memalloc)
					(memobj->sz,COPRTHR_DEVMEM_TYPE_BUFFER);

			memobj->mem1[i]->size = memobj->sz;

			printcl( CL_DEBUG "__do_create_buffer: mem1[%d]=%p (%ld)",
				i,memobj->mem1[i],memobj->mem1[i]->size);

			if (memobj->sz > 0 && memobj->mem1[i]->res == 0) {

				printcl( CL_WARNING "malloc failed");

			} else {

				printcl( CL_DEBUG "malloc'd %d bytes",memobj->sz);

			}

			if (memobj->flags&CL_MEM_COPY_HOST_PTR) {
				printcl( CL_DEBUG "CL_MEM_COPY_HOST_PTR");
//				memcpy(memobj->mem1[i]->res,memobj->host_ptr,memobj->sz);
				__resolve_devid_devops(ctx->devices[i],memwrite)
					(memobj->mem1[i],memobj->host_ptr,memobj->sz);
			}

			if (memobj->flags&CL_MEM_USE_HOST_PTR) {
				printcl( CL_WARNING 
					"workaround: CL_MEM_USE_HOST_PTR => CL_MEM_COPY_HOST_PTR,"
					" fix this");
//				memcpy(memobj->mem1[i]->res,memobj->host_ptr,memobj->sz);
				__resolve_devid_devops(ctx->devices[i],memwrite)
					(memobj->mem1[i],memobj->host_ptr,memobj->sz);
				
			}

			printcl( CL_DEBUG "past");

//		} else if (
//			__resolve_devid_ocldevinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_GPU
//		) {
//
//			printcl( CL_WARNING "device unsupported, how did you get here?");
//
//		} else {
//
//		}

	}

	printcl( CL_DEBUG "__do_create_buffer return");


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
			__resolve_devid_ocldevinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_CPU
		) {

			/* XXX the 128 would take too long to explain -DAR */

			memobj->mem1[i]->res = (void**)malloc(128 + memobj->sz);
			size_t* p = (size_t*)memobj->mem1[i]->res;
			p[0] = memobj->width;
			p[1] = memobj->height;
			memobj->mem1[i]->size = memobj->sz + 128;

			if (memobj->sz > 0 && memobj->mem1[i]->res == 0) {

				printcl( CL_WARNING "malloc failed");

			} else {

				printcl( CL_DEBUG "malloc'd %d bytes",memobj->sz);

			}

		} else if (
			__resolve_devid_ocldevinfo(ctx->devices[i],devtype)==CL_DEVICE_TYPE_GPU
		) {

			printcl( CL_WARNING "device unsupported, how did you get here?");

		} else {

		}

	}

}

