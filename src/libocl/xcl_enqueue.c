/* xcl_enqueue.c
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
#include "event.h"
#include <pthread.h>

// Enqueued Commands APIs

cl_int 
clEnqueueReadBuffer(
	cl_command_queue cmdq,
	cl_mem membuf,
	cl_bool block,
	size_t offset,
	size_t cb, 
	void* ptr,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	DEBUG(__FILE__,__LINE__,"clEnqueueReadBuffer");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	DEBUG(__FILE__,__LINE__,"clEnqueueReadBuffer: membuf=%p",membuf);
	DEBUG(__FILE__,__LINE__,"clEnqueueReadBuffer: membuf->type=%d",membuf->type);
	DEBUG(__FILE__,__LINE__,"clEnqueueReadBuffer: __invalid_membuf()=%d",__invalid_membuf(membuf));
	if (__invalid_membuf(membuf)) return(CL_INVALID_MEM_OBJECT);

	DEBUG(__FILE__,__LINE__,"clEnqueueReadBuffer: membuf->sz=%d",membuf->sz);
	if (membuf->sz < (offset+cb) || !ptr) return(CL_INVALID_VALUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(membuf->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != membuf->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		DEBUG(__FILE__,__LINE__,"test lock");
		__lock_event(ev);
		__unlock_event(ev);

		__set_cmd_read_buffer(ev);

		__do_set_cmd_read_buffer(ev,membuf,offset,cb,ptr);
	
		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


/*
	if (block) {

		DEBUG(__FILE__,__LINE__,"clEnqueueReadBuffer blocking");

		__lock_event(ev);

      while (ev->cmd_stat != CL_COMPLETE) {
         DEBUG(__FILE__,__LINE__,"clEnqueueReadBuffer: wait-sleep\n");
         __wait_event(ev);
         DEBUG(__FILE__,__LINE__,"clEnqueueReadBuffer: wait-wake\n");
      }

      DEBUG(__FILE__,__LINE__,"clEnqueueReadBuffer: event %p complete\n",ev);

		__unlock_event(ev);

	}
*/


	return(CL_SUCCESS);
}

                            
cl_int 
clEnqueueWriteBuffer(
	cl_command_queue cmdq, 
	cl_mem membuf, 
   cl_bool block, 
   size_t offset, 
   size_t cb, 
   const void* ptr, 
   cl_uint nwaitlist, 
   const cl_event* waitlist, 
   cl_event* event
)
{
	DEBUG(__FILE__,__LINE__,"clEnqueueWriteBuffer");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_membuf(membuf)) return(CL_INVALID_MEM_OBJECT);

	if (membuf->sz < (offset+cb) || !ptr) return(CL_INVALID_VALUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(membuf->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != membuf->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_write_buffer(ev);

		__do_set_cmd_write_buffer(ev,membuf,offset,cb,ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	if (block) {

		DEBUG(__FILE__,__LINE__,"clEnqueueWriteBuffer blocking");

		__lock_event(ev);

      while (ev->cmd_stat != CL_COMPLETE) {
         DEBUG(__FILE__,__LINE__,"wait-sleep\n");
         __wait_event(ev);
         DEBUG(__FILE__,__LINE__,"wait-wake\n");
      }

      DEBUG(__FILE__,__LINE__,"event %p complete\n",ev);

		__unlock_event(ev);

	}


	return(CL_SUCCESS);
}

                            
cl_int 
clEnqueueCopyBuffer(
	cl_command_queue cmdq, 
	cl_mem src_membuf,
	cl_mem dst_membuf, 
	size_t src_offset,
	size_t dst_offset,
	size_t cb, 
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event 
)
{
	WARN(__FILE__,__LINE__,"clEnqueueCopyBuffer: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_membuf(src_membuf)) return(CL_INVALID_MEM_OBJECT);

	if (__invalid_membuf(dst_membuf)) return(CL_INVALID_MEM_OBJECT);

	if (src_membuf->sz < (src_offset+cb)) return(CL_INVALID_VALUE);

	if (dst_membuf->sz < (dst_offset+cb)) return(CL_INVALID_VALUE);

	if (
		src_membuf == dst_membuf
		&& ( (src_offset+cb) > dst_offset || (dst_offset+cb) > src_offset)
	) return(CL_MEM_COPY_OVERLAP);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(src_membuf->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(dst_membuf->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != src_membuf->ctx) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != dst_membuf->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_copy_buffer(ev);

		__do_set_cmd_copy_buffer(ev,
			src_membuf,dst_membuf,src_offset,dst_offset,cb);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
}

                            
cl_int 
clEnqueueReadImage(
	cl_command_queue cmdq,
	cl_mem image,
	cl_bool block,
	const size_t* origin, // [3]
	const size_t* region, // [3]
	size_t row_pitch,
	size_t slice_pitch, 
	void* ptr,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	WARN(__FILE__,__LINE__,"clEnqueueReadImage: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_image(image)) return(CL_INVALID_MEM_OBJECT);

//	if (image->sz < (offset+cb) || !ptr) return(CL_INVALID_VALUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(image->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != image->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_read_image(ev);

		__do_set_cmd_read_image(ev,image,origin,region,row_pitch,slice_pitch,ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	if (block) {

	}


	return(CL_SUCCESS);
}

                            

cl_int 
clEnqueueWriteImage(
	cl_command_queue cmdq,
	cl_mem image,
	cl_bool block,
	const size_t* origin, // [3]
	const size_t* region, // [3] 
	size_t row_pitch,
	size_t slice_pitch, 
	const void* ptr,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	WARN(__FILE__,__LINE__,"clEnqueueWriteImage: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_image(image)) return(CL_INVALID_MEM_OBJECT);

//	if (membuf->sz < (offset+cb) || !ptr) return(CL_INVALID_VALUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(image->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != image->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_write_image(ev);

		__do_set_cmd_write_image(ev,image,origin,region,
			row_pitch,slice_pitch,ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	if (block) {

	}


	return(CL_SUCCESS);
}



cl_int 
clEnqueueCopyImage(
	cl_command_queue cmdq,
	cl_mem src_image,
	cl_mem dst_image, 
	const size_t* src_origin, // [3] 
	const size_t* dst_origin, // [3] 
	const size_t* region, // [3]
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	WARN(__FILE__,__LINE__,"clEnqueueCopyImage: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_image(src_image)) return(CL_INVALID_MEM_OBJECT);

	if (__invalid_image(dst_image)) return(CL_INVALID_MEM_OBJECT);

//	if (membuf->sz < (offset+cb) || !ptr) return(CL_INVALID_VALUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(src_image->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(dst_image->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != src_image->ctx) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != dst_image->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_copy_image(ev);

		__do_set_cmd_copy_image(ev,src_image,dst_image,
			src_origin,dst_origin,region);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
}



cl_int 
clEnqueueCopyImageToBuffer(
	cl_command_queue cmdq,
	cl_mem src_image,
	cl_mem dst_membuf,
	const size_t* src_origin, // [3]
	const size_t* region, // [3] 
	size_t dst_offset,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	WARN(__FILE__,__LINE__,"clEnqueueCopyImageToBuffer: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_image(src_image)) return(CL_INVALID_MEM_OBJECT);

	if (__invalid_membuf(dst_membuf)) return(CL_INVALID_MEM_OBJECT);

//	if (membuf->sz < (offset+cb) || !ptr) return(CL_INVALID_VALUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(src_image->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(dst_membuf->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != src_image->ctx) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != dst_membuf->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_copy_image_to_buffer(ev);

		__do_set_cmd_copy_image_to_buffer(ev,src_image,dst_membuf,
			src_origin,region,dst_offset);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
}



cl_int 
clEnqueueCopyBufferToImage(
	cl_command_queue cmdq,
	cl_mem src_membuf,
	cl_mem dst_image, 
	size_t src_offset,
	const size_t* dst_origin, // [3]
	const size_t* region, // [3] 
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event 
)
{
	WARN(__FILE__,__LINE__,"clEnqueueCopyBufferToImage: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_membuf(src_membuf)) return(CL_INVALID_MEM_OBJECT);

	if (__invalid_image(dst_image)) return(CL_INVALID_MEM_OBJECT);

//	if (membuf->sz < (offset+cb) || !ptr) return(CL_INVALID_VALUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(src_membuf->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(dst_image->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != src_membuf->ctx) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != dst_image->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_copy_buffer_to_image(ev);

		__do_set_cmd_copy_buffer_to_image(ev,src_membuf,dst_image,
			src_offset,dst_origin,region);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
}



void* 
clEnqueueMapBuffer(
	cl_command_queue cmdq,
	cl_mem membuf,
	cl_bool block,
	cl_map_flags map_flags,
	size_t offset,
	size_t cb,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event,
	cl_int* err_ret
)
{
	WARN(__FILE__,__LINE__,"clEnqueueMapBuffer: warning: unsupported");

	void* ptr = 0;

	if (__invalid_command_queue(cmdq)) 
		__error_return(CL_INVALID_COMMAND_QUEUE,void*);

	if (__invalid_membuf(membuf)) __error_return(CL_INVALID_MEM_OBJECT,void*);

	if (membuf->sz < (offset+cb) || !ptr) __error_return(CL_INVALID_VALUE,void*);

	if (__invalid_context(cmdq->ctx)) __error_return(CL_INVALID_CONTEXT,void*);

	if (__invalid_context(membuf->ctx)) __error_return(CL_INVALID_CONTEXT,void*);

	if (cmdq->ctx != membuf->ctx) __error_return(CL_INVALID_CONTEXT,void*);

//	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));

	/* need to create mapped ptr */


	if (ev) {

		__init_event(ev);

		__set_cmd_map_buffer(ev);

		__do_set_cmd_map_buffer(ev,membuf,map_flags,offset,cb,&ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,void*);



	if (block) {

	}


	__success();

	return(ptr);
}


void* 
clEnqueueMapImage(
	cl_command_queue cmdq,
	cl_mem image,
	cl_bool block,
	cl_map_flags map_flags, 
	const size_t* origin, // [3]
	const size_t* region, // [3]
	size_t* row_pitch,
	size_t* slice_pitch,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event,
	cl_int* err_ret
)
{
	WARN(__FILE__,__LINE__,"clEnqueueMapImage: warning: unsupported");

	void* ptr = 0;

	if (__invalid_command_queue(cmdq)) 
		__error_return(CL_INVALID_COMMAND_QUEUE,void*);

	if (__invalid_image(image)) __error_return(CL_INVALID_MEM_OBJECT,void*);

//	if (membuf->sz < (offset+cb) || !ptr) return(CL_INVALID_VALUE,void*);

	if (__invalid_context(cmdq->ctx)) __error_return(CL_INVALID_CONTEXT,void*);

	if (__invalid_context(image->ctx)) __error_return(CL_INVALID_CONTEXT,void*);

	if (cmdq->ctx != image->ctx) __error_return(CL_INVALID_CONTEXT,void*);

//	__check_waitlist_error_return(nwaitlist,waitlist,cmdq->ctx,void*);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));

	/* need to create mapped ptr */


	if (ev) {

		__init_event(ev);

		__set_cmd_map_image(ev);

		__do_set_cmd_map_image(ev,image,map_flags,origin,region,
			row_pitch,slice_pitch,&ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,void*);


	if (block) {

	}


	__success();

	return(ptr);
}


cl_int 
clEnqueueUnmapMemObject(
	cl_command_queue cmdq,
	cl_mem memobj,
	void* mapped_ptr,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	WARN(__FILE__,__LINE__,"clEnqueueUnmapMemObject: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_memobj(memobj)) return(CL_INVALID_MEM_OBJECT);

	if (__invalid_mapped_ptr(mapped_ptr)) return(CL_INVALID_VALUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(memobj->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != memobj->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_unmap_mem_object(ev);

		__do_set_cmd_unmap_memobj(ev,memobj,mapped_ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
}


cl_int 
clEnqueueNDRangeKernel(
	cl_command_queue cmdq,
	cl_kernel krn,
	cl_uint work_dim,
	const size_t* global_work_offset,
	const size_t* global_work_size,
	const size_t* local_work_size,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	int i;
	
	DEBUG(__FILE__,__LINE__,"clEnqueueNDRangeKernel gwo=%p",global_work_offset);

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_kernel(krn)) return(CL_INVALID_KERNEL);

//	if (__invalid_kernel_args(krn)) return(CL_INVALID_KERNEL_ARGS);

//	if (__invalid_executable(krn,cmdq)) return(CL_INVALID_PROGRAM_EXECUTABLE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(krn->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != krn->ctx) return(CL_INVALID_CONTEXT);

	DEBUG(__FILE__,__LINE__,"clEnqueueNDRangeKernel: A %d %p %p",nwaitlist,waitlist,cmdq);
	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);
	DEBUG(__FILE__,__LINE__,"clEnqueueNDRangeKernel: A");

	if (work_dim < 1 || work_dim > 3) return(CL_INVALID_WORK_DIMENSION);

//	if (global_work_offset) return(CL_INVALID_GLOBAL_OFFSET);
	if (global_work_offset) WARN(__FILE__,__LINE__,"clEnqueueNDRangeKernel: ignoring global_work_offset");

	if (!global_work_size) return(CL_INVALID_VALUE);

	if (!local_work_size) return(CL_INVALID_VALUE); /* 1.0 std ambiguity */

	if (local_work_size) for(i=0;i<work_dim;i++) 
		if (global_work_size[i]%local_work_size[i]) 
			return(CL_INVALID_WORK_GROUP_SIZE);

	/* check against CL_DEVICE_MAX_WORK_GROUP_SIZE */

	/* check against CL_DEVICE_MAX_WORK_ITEM_SIZES[] */

	/* XXX no attempt is made to check kernel for attirbutes -DAR */


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_ndrange_kernel(ev);

		__do_set_cmd_ndrange_kernel(cmdq,ev,krn,work_dim,
			global_work_offset,global_work_size,local_work_size);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
}


cl_int 
clEnqueueTask(
	cl_command_queue cmdq,
	cl_kernel krn,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	WARN(__FILE__,__LINE__,"clEnqueueTask: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_kernel(krn)) return(CL_INVALID_KERNEL);

//	if (__invalid_kernel_args(krn)) return(CL_INVALID_KERNEL_ARGS);

//	if (__invalid_executable(krn,cmdq)) return(CL_INVALID_PROGRAM_EXECUTABLE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(krn->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != krn->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_task(ev);

		__do_set_cmd_task(ev,krn);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
}


cl_int 
clEnqueueNativeKernel(
	cl_command_queue cmdq,
	void (*user_func)(void *), 
	void* args,
	size_t cb_args, 
	cl_uint nmemobj,
	const cl_mem* memobj,
	const void**  args_mem_loc,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	WARN(__FILE__,__LINE__,"clEnqueueNativeKernel: warning: unsupported");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	/* XXX check consistency of contexts w/ cmdq,membuf,waitlist */

/* XXX not supported 
	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));

	if (ev) {

		__init_event(ev);

		__set_cmd_native_kernel(ev);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);
*/

	return(CL_ENOTSUP);
}


cl_int 
clEnqueueMarker(
	cl_command_queue cmdq,
   cl_event* event
)
{
	DEBUG(__FILE__,__LINE__,"clEnqueueMarker");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_marker(ev);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
}


cl_int 
clEnqueueWaitForEvents(
	cl_command_queue cmdq,
	cl_uint nwaitlist,
	const cl_event* waitlist
)
{
	WARN(__FILE__,__LINE__,"clEnqueueWaitForEvents: warning: unsupported");

/*
	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_wait_for_events(ev);

//		__do_enqueue_cmd(ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
*/
	return(CL_ENOTSUP);

}


cl_int 
clEnqueueBarrier( cl_command_queue cmdq )
{
	DEBUG(__FILE__,__LINE__,"clEnqueueBarrier");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		__set_cmd_barrier(ev);

		__do_enqueue_cmd(ev);

	} else return(CL_OUT_OF_HOST_MEMORY);


	return(CL_SUCCESS);
}



