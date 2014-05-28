/* ocl_enqueue.c
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
#include "event.h"
#include <pthread.h>

void __do_release_event(cl_event ev);
void __do_set_cmd_read_buffer( cl_event ev, cl_mem src, size_t src_offset, 
	size_t len, void* dst);

void __do_set_cmd_write_buffer( cl_event ev, cl_mem dst, size_t dst_offset,
	size_t len, const void* src);

void __do_set_cmd_copy_buffer( cl_event ev, cl_mem src, cl_mem dst, 
	size_t src_offset, size_t dst_offset, size_t len );

void __do_set_cmd_read_image(cl_event ev, cl_mem src, const size_t* src_origin,
	const size_t* region, size_t row_pitch, size_t slice_pitch, void* dst);

void __do_set_cmd_write_image( cl_event ev, cl_mem dst, 
	const size_t* dst_origin, const size_t* region, size_t row_pitch, 
	size_t slice_pitch, const void* src);

void __do_set_cmd_copy_image( cl_event ev, cl_mem src, cl_mem dst, 
	const size_t* src_origin, const size_t* dst_origin, const size_t* region);

void __do_set_cmd_copy_image_to_buffer( cl_event ev, cl_mem src, cl_mem dst, 
   const size_t* src_origin, const size_t* region, size_t dst_offset);

void __do_set_cmd_copy_buffer_to_image( cl_event ev, cl_mem src, cl_mem dst, 
   size_t src_offset, const size_t* dst_origin, const size_t* region);

void __do_set_cmd_map_buffer( cl_event ev, cl_mem membuf, cl_map_flags flags,
	size_t offset, size_t len, void* pp);

void __do_set_cmd_map_image( cl_event ev, cl_mem image, cl_map_flags flags, 
	const size_t* origin, const size_t* region, size_t* row_pitch, 
	size_t* slice_pitch, void* p);

void __do_set_cmd_unmap_memobj( cl_event ev, cl_mem memobj, void* p);

void __do_set_cmd_ndrange_kernel( cl_command_queue cmdq, cl_event ev, 
	cl_kernel krn, cl_uint work_dim, const size_t* global_work_offset,
   const size_t* global_work_size, const size_t* local_work_size);

void __do_set_cmd_task( cl_event ev, cl_kernel krn);
void __do_wait_for_events( cl_uint nev, const cl_event* evlist);


#define FORCE_BLOCKING

#define __wait(ev) do { \
		printcl( CL_DEBUG "blocking on event %p",ev); \
		__lock_event(ev); \
      while (ev->ev1->cmd_stat != CL_COMPLETE) { \
         printcl( CL_DEBUG "wait-sleep on event %p\n",ev); \
         __wait_event(ev); \
         printcl( CL_DEBUG "wait-wake on event %p",ev); \
      } \
      printcl( CL_DEBUG "event %p complete\n",ev); \
		__unlock_event(ev); \
	} while(0)


// Enqueue Command API Calls

cl_int 
_clEnqueueReadBuffer(
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
	printcl( CL_DEBUG "clEnqueueReadBuffer");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	printcl( CL_DEBUG "clEnqueueReadBuffer: membuf=%p",membuf);
	printcl( CL_DEBUG "clEnqueueReadBuffer: membuf->type=%d",membuf->type);
	printcl( CL_DEBUG "clEnqueueReadBuffer: __invalid_membuf()=%d",__invalid_membuf(membuf));
	if (__invalid_membuf(membuf)) return(CL_INVALID_MEM_OBJECT);

	printcl( CL_DEBUG "clEnqueueReadBuffer: membuf->sz=%d",membuf->sz);
	if (membuf->sz < (offset+cb) || !ptr) return(CL_INVALID_VALUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(membuf->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != membuf->ctx) return(CL_INVALID_CONTEXT);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		printcl( CL_DEBUG "test lock");
		__lock_event(ev);
		__unlock_event(ev);

		ev->cmdq = cmdq;
		__set_cmd_read_buffer(ev);

		__do_set_cmd_read_buffer(ev,membuf,offset,cb,ptr);
	
		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

/*
	if (block) {

		printcl( CL_DEBUG "clEnqueueReadBuffer blocking");

		__lock_event(ev);

      while (ev->cmd_stat != CL_COMPLETE) {
         printcl( CL_DEBUG "clEnqueueReadBuffer: wait-sleep\n");
         __wait_event(ev);
         printcl( CL_DEBUG "clEnqueueReadBuffer: wait-wake\n");
      }

      printcl( CL_DEBUG "clEnqueueReadBuffer: event %p complete\n",ev);

		__unlock_event(ev);

	}
*/
#ifdef FORCE_BLOCKING
	__wait(ev);
#else
	if (block) __wait(ev);
#endif


	return(CL_SUCCESS);
}

                            
cl_int 
_clEnqueueWriteBuffer(
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
	printcl( CL_DEBUG "clEnqueueWriteBuffer");

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

		ev->cmdq = cmdq;
		__set_cmd_write_buffer(ev);

		__do_set_cmd_write_buffer(ev,membuf,offset,cb,ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

/*
	if (block) {

		printcl( CL_DEBUG "clEnqueueWriteBuffer blocking");

		__lock_event(ev);

      while (ev->cmd_stat != CL_COMPLETE) {
         printcl( CL_DEBUG "wait-sleep\n");
         __wait_event(ev);
         printcl( CL_DEBUG "wait-wake\n");
      }

      printcl( CL_DEBUG "event %p complete\n",ev);

		__unlock_event(ev);

	}
*/
#ifdef FORCE_BLOCKING
	__wait(ev);
#else
	if (block) __wait(ev);
#endif


	return(CL_SUCCESS);
}

                            
cl_int 
_clEnqueueCopyBuffer(
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
	printcl( CL_WARNING "clEnqueueCopyBuffer: warning: unsupported");

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

		ev->cmdq = cmdq;
		__set_cmd_copy_buffer(ev);

		__do_set_cmd_copy_buffer(ev,
			src_membuf,dst_membuf,src_offset,dst_offset,cb);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

#ifdef FORCE_BLOCKING	
	__wait(ev);
#endif

	return(CL_SUCCESS);
}

                            
cl_int 
_clEnqueueReadImage(
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
	printcl( CL_WARNING "clEnqueueReadImage: warning: unsupported");

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

		ev->cmdq = cmdq;
		__set_cmd_read_image(ev);

		__do_set_cmd_read_image(ev,image,origin,region,row_pitch,slice_pitch,ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


#ifdef FORCE_BLOCKING
	__wait(ev);
#else
	if (block) __wait(ev);
#endif


	return(CL_SUCCESS);
}

                            

cl_int 
_clEnqueueWriteImage(
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
	printcl( CL_WARNING "clEnqueueWriteImage: warning: unsupported");

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

		ev->cmdq = cmdq;
		__set_cmd_write_image(ev);

		__do_set_cmd_write_image(ev,image,origin,region,
			row_pitch,slice_pitch,ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);


#ifdef FORCE_BLOCKING
	__wait(ev);
#else
	if (block) __wait(ev);
#endif

	return(CL_SUCCESS);
}



cl_int 
_clEnqueueCopyImage(
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
	printcl( CL_WARNING "clEnqueueCopyImage: warning: unsupported");

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

		ev->cmdq = cmdq;
		__set_cmd_copy_image(ev);

		__do_set_cmd_copy_image(ev,src_image,dst_image,
			src_origin,dst_origin,region);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

#ifdef FORCE_BLOCKING
	__wait(ev);
#endif

	return(CL_SUCCESS);
}



cl_int 
_clEnqueueCopyImageToBuffer(
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
	printcl( CL_WARNING "clEnqueueCopyImageToBuffer: warning: unsupported");

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

		ev->cmdq = cmdq;
		__set_cmd_copy_image_to_buffer(ev);

		__do_set_cmd_copy_image_to_buffer(ev,src_image,dst_membuf,
			src_origin,region,dst_offset);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

#ifdef FORCE_BLOCKING
	__wait(ev);
#endif


	return(CL_SUCCESS);
}



cl_int 
_clEnqueueCopyBufferToImage(
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
	printcl( CL_WARNING "clEnqueueCopyBufferToImage: warning: unsupported");

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

		ev->cmdq = cmdq;
		__set_cmd_copy_buffer_to_image(ev);

		__do_set_cmd_copy_buffer_to_image(ev,src_membuf,dst_image,
			src_offset,dst_origin,region);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

#ifdef FORCE_BLOCKING
	__wait(ev);
#endif


	return(CL_SUCCESS);
}



void* 
_clEnqueueMapBuffer(
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
	printcl( CL_WARNING "clEnqueueMapBuffer: warning: unsupported");

	void* ptr = 0;

	if (__invalid_command_queue(cmdq)) 
		__error_return(CL_INVALID_COMMAND_QUEUE,void*);

	if (__invalid_membuf(membuf)) __error_return(CL_INVALID_MEM_OBJECT,void*);

	if (membuf->sz < (offset+cb)) __error_return(CL_INVALID_VALUE,void*);

	if (__invalid_context(cmdq->ctx)) __error_return(CL_INVALID_CONTEXT,void*);

	if (__invalid_context(membuf->ctx)) __error_return(CL_INVALID_CONTEXT,void*);

	if (cmdq->ctx != membuf->ctx) __error_return(CL_INVALID_CONTEXT,void*);

	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);

	printcl( CL_WARNING "clEnqueueMapBuffer: passed initial checks");

	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));

	if (ev) {

		__init_event(ev);

		ev->cmdq = cmdq;
		__set_cmd_map_buffer(ev);

		printcl( CL_DEBUG "calling __do_set_cmd_map_buffer");

		__do_set_cmd_map_buffer(ev,membuf,map_flags,offset,cb,&ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,void*);


/*
	if (block) {

		printcl( CL_WARNING "clEnqueueMapBuffer blocking ev=%p",ev);

		__lock_event(ev);

      while (ev->cmd_stat != CL_COMPLETE) {
         printcl( CL_DEBUG "wait-sleep\n");
         __wait_event(ev);
         printcl( CL_DEBUG "wait-wake\n");
      }

      printcl( CL_DEBUG "event %p complete\n",ev);

		__unlock_event(ev);

	} else {
		printcl( CL_DEBUG "clEnqueueMapBuffer non-blocking");
	}
*/

#ifdef FORCE_BLOCKING
	__wait(ev);
#else
	if (block) __wait(ev);
#endif

	__success();

	return(ptr);
}


void* 
_clEnqueueMapImage(
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
	printcl( CL_WARNING "clEnqueueMapImage: warning: unsupported");

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

		ev->cmdq = cmdq;
		__set_cmd_map_image(ev);

		__do_set_cmd_map_image(ev,image,map_flags,origin,region,
			row_pitch,slice_pitch,&ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else __error_return(CL_OUT_OF_HOST_MEMORY,void*);


#ifdef FORCE_BLOCKING
	__wait(ev);
#else
	if (block) __wait(ev);
#endif

	__success();

	return(ptr);
}


cl_int 
_clEnqueueUnmapMemObject(
	cl_command_queue cmdq,
	cl_mem memobj,
	void* mapped_ptr,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	printcl( CL_WARNING "clEnqueueUnmapMemObject: warning: unsupported");

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

		ev->cmdq = cmdq;
		__set_cmd_unmap_mem_object(ev);

		__do_set_cmd_unmap_memobj(ev,memobj,mapped_ptr);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

#ifdef FORCE_BLOCKING
	__wait(ev);
#endif

	return(CL_SUCCESS);
}


cl_int 
_clEnqueueNDRangeKernel(
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
	
	printcl( CL_DEBUG "clEnqueueNDRangeKernel gwo=%p",global_work_offset);

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_kernel(krn)) return(CL_INVALID_KERNEL);

//	if (__invalid_kernel_args(krn)) return(CL_INVALID_KERNEL_ARGS);

//	if (__invalid_executable(krn,cmdq)) return(CL_INVALID_PROGRAM_EXECUTABLE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);

	if (__invalid_context(krn->ctx)) return(CL_INVALID_CONTEXT);

	if (cmdq->ctx != krn->ctx) return(CL_INVALID_CONTEXT);

	printcl( CL_DEBUG "clEnqueueNDRangeKernel: A %d %p %p",nwaitlist,waitlist,cmdq);
	__check_waitlist(nwaitlist,waitlist,cmdq->ctx);
	printcl( CL_DEBUG "clEnqueueNDRangeKernel: A");

	if (work_dim < 1 || work_dim > 3) return(CL_INVALID_WORK_DIMENSION);

//	if (global_work_offset) return(CL_INVALID_GLOBAL_OFFSET);
	if (global_work_offset && *global_work_offset != 0) 
		printcl( CL_WARNING "clEnqueueNDRangeKernel: ignoring global_work_offset");

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

		ev->cmdq = cmdq;
		__set_cmd_ndrange_kernel(ev);

		__do_set_cmd_ndrange_kernel(cmdq,ev,krn,work_dim,
			global_work_offset,global_work_size,local_work_size);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

#ifdef FORCE_BLOCKING
	__wait(ev);
#endif

	return(CL_SUCCESS);
}


cl_int 
_clEnqueueTask(
	cl_command_queue cmdq,
	cl_kernel krn,
	cl_uint nwaitlist,
	const cl_event* waitlist,
	cl_event* event
)
{
	printcl( CL_WARNING "clEnqueueTask: warning: unsupported");

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

		ev->cmdq = cmdq;
		__set_cmd_task(ev);

		__do_set_cmd_task(ev,krn);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

#ifdef FORCE_BLOCKING
	__wait(ev);
#endif

	return(CL_SUCCESS);
}


cl_int 
_clEnqueueNativeKernel(
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
	printcl( CL_WARNING "clEnqueueNativeKernel: warning: unsupported");

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

#ifdef FORCE_BLOCKING
	__wait(ev);
#endif
*/

	return(CL_ENOTSUP);
}


cl_int 
_clEnqueueMarker(
	cl_command_queue cmdq,
   cl_event* event
)
{
	printcl( CL_DEBUG "clEnqueueMarker");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		ev->cmdq = cmdq;
		__set_cmd_marker(ev);

		__do_enqueue_cmd(cmdq,ev);

		if (event) *event = ev;

	} else return(CL_OUT_OF_HOST_MEMORY);

#ifdef FORCE_BLOCKING
	__wait(ev);
#endif

	return(CL_SUCCESS);
}


cl_int 
_clEnqueueWaitForEvents(
	cl_command_queue cmdq,
	cl_uint nwaitlist,
	const cl_event* waitlist
)
{
	printcl( CL_WARNING "clEnqueueWaitForEvents: warning: unsupported");

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
_clEnqueueBarrier( cl_command_queue cmdq )
{
	printcl( CL_DEBUG "clEnqueueBarrier");

	if (__invalid_command_queue(cmdq)) return(CL_INVALID_COMMAND_QUEUE);

	if (__invalid_context(cmdq->ctx)) return(CL_INVALID_CONTEXT);


	struct _cl_event* ev = (struct _cl_event*)malloc(sizeof(struct _cl_event));


	if (ev) {

		__init_event(ev);

		ev->cmdq = cmdq;
		__set_cmd_barrier(ev);

		__do_enqueue_cmd(ev);

	} else return(CL_OUT_OF_HOST_MEMORY);

#ifdef FORCE_BLOCKING
	__wait(ev);
#endif

	return(CL_SUCCESS);
}




// Aliased Enqueue Command API Calls

cl_int
clEnqueueReadBuffer( cl_command_queue cmdq, cl_mem membuf, cl_bool block,
   size_t offset, size_t cb, void* ptr, cl_uint nwaitlist, 
	const cl_event* waitlist, cl_event* event)
	__attribute__((alias("_clEnqueueReadBuffer")));

cl_int
clEnqueueWriteBuffer( cl_command_queue cmdq, cl_mem membuf, cl_bool block,
   size_t offset, size_t cb, const void* ptr, cl_uint nwaitlist,
   const cl_event* waitlist, cl_event* event)
	__attribute__((alias("_clEnqueueWriteBuffer")));

cl_int
clEnqueueCopyBuffer( cl_command_queue cmdq, cl_mem src_membuf, 
	cl_mem dst_membuf, size_t src_offset, size_t dst_offset, size_t cb,
   cl_uint nwaitlist, const cl_event* waitlist, cl_event* event)
	__attribute__((alias("_clEnqueueCopyBuffer")));

cl_int
clEnqueueReadImage( cl_command_queue cmdq, cl_mem image, cl_bool block,
   const size_t* origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, void* ptr, cl_uint nwaitlist,
   const cl_event* waitlist, cl_event* event)
	__attribute__((alias("_clEnqueueReadImage")));

cl_int
clEnqueueWriteImage( cl_command_queue cmdq, cl_mem image, cl_bool block,
   const size_t* origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, const void* ptr, cl_uint nwaitlist,
   const cl_event* waitlist, cl_event* event)
	__attribute__((alias("_clEnqueueWriteImage")));

cl_int
clEnqueueCopyImage( cl_command_queue cmdq, cl_mem src_image, cl_mem dst_image,
   const size_t* src_origin, const size_t* dst_origin, const size_t* region,
   cl_uint nwaitlist, const cl_event* waitlist, cl_event* event)
	__attribute__((alias("_clEnqueueCopyImage")));

cl_int
clEnqueueCopyImageToBuffer( cl_command_queue cmdq, cl_mem src_image, 
	cl_mem dst_membuf, const size_t* src_origin, const size_t* region, 
   size_t dst_offset, cl_uint nwaitlist, const cl_event* waitlist, 
	cl_event* event)
	__attribute__((alias("_clEnqueueCopyImageToBuffer")));

cl_int
clEnqueueCopyBufferToImage( cl_command_queue cmdq, cl_mem src_membuf,
   cl_mem dst_image, size_t src_offset, const size_t* dst_origin,
   const size_t* region, cl_uint nwaitlist, const cl_event* waitlist, 
	cl_event* event)
	__attribute__((alias("_clEnqueueCopyBufferToImage")));

void*
clEnqueueMapBuffer( cl_command_queue cmdq, cl_mem membuf, cl_bool block,
   cl_map_flags map_flags, size_t offset, size_t cb, cl_uint nwaitlist,
   const cl_event* waitlist, cl_event* event, cl_int* err_ret)
	__attribute__((alias("_clEnqueueMapBuffer")));

void*
clEnqueueMapImage( cl_command_queue cmdq, cl_mem image, cl_bool block,
   cl_map_flags map_flags, const size_t* origin, const size_t* region,
   size_t* row_pitch, size_t* slice_pitch, cl_uint nwaitlist,
   const cl_event* waitlist, cl_event* event, cl_int* err_ret)
	__attribute__((alias("_clEnqueueMapImage")));

cl_int clEnqueueUnmapMemObject( cl_command_queue cmdq, cl_mem memobj,
   void* mapped_ptr, cl_uint nwaitlist, const cl_event* waitlist, 
	cl_event* event)
	__attribute__((alias("_clEnqueueUnmapMemObject")));


cl_int
clEnqueueNDRangeKernel( cl_command_queue cmdq, cl_kernel krn, cl_uint work_dim,
   const size_t* global_work_offset, const size_t* global_work_size,
   const size_t* local_work_size, cl_uint nwaitlist, const cl_event* waitlist,
   cl_event* event)
	__attribute__((alias("_clEnqueueNDRangeKernel")));

cl_int
clEnqueueTask( cl_command_queue cmdq, cl_kernel krn, cl_uint nwaitlist,
   const cl_event* waitlist, cl_event* event)
	__attribute__((alias("_clEnqueueTask")));

cl_int
clEnqueueNativeKernel( cl_command_queue cmdq, void (*user_func)(void *),
   void* args, size_t cb_args, cl_uint nmemobj, const cl_mem* memobj,
   const void**  args_mem_loc, cl_uint nwaitlist, const cl_event* waitlist,
   cl_event* event)
	__attribute__((alias("_clEnqueueNativeKernel")));

cl_int
clEnqueueMarker( cl_command_queue cmdq, cl_event* event)
	__attribute__((alias("_clEnqueueMarker")));

cl_int
clEnqueueWaitForEvents( cl_command_queue cmdq, cl_uint nwaitlist,
   const cl_event* waitlist)
	__attribute__((alias("_clEnqueueWaitForEvents")));

cl_int
clEnqueueBarrier( cl_command_queue cmdq )
	__attribute__((alias("_clEnqueueBarrier")));


/*
 * Internal event implementation calls
 */

void __do_release_event(cl_event ev) 
{
	/* XXX may need to check state of event for controlled release -DAR */

	/* XXX this assumes the ev is on cmds_complete, check this first! -DAR */

	__do_release_event_1(ev->ev1);
	ev->cmdq = 0;
	ev->dev = 0;
}



#define __copy3(dst,src) do { \
	(dst)[0]=(src)[0]; (dst)[1]=(src)[1]; (dst)[2]=(src)[2]; \
	} while(0)


void __do_set_cmd_read_buffer( 
	cl_event ev, cl_mem src, size_t src_offset, size_t len, void* dst
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_read_buffer_1( ev->ev1, src->mem1[n], src_offset, len, dst); 
}


void __do_set_cmd_write_buffer( 
	cl_event ev, cl_mem dst, size_t dst_offset, size_t len, const void* src
)
{ 
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_write_buffer_1( ev->ev1, dst->mem1[n], dst_offset, len, src); 
}


void __do_set_cmd_copy_buffer( 
	cl_event ev, cl_mem src, cl_mem dst, 
	size_t src_offset, size_t dst_offset, size_t len 
)
{ 
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_copy_buffer_1( ev->ev1, src->mem1[n], dst->mem1[n], 
		src_offset, dst_offset, len);
}


void __do_set_cmd_read_image( 
	cl_event ev, cl_mem src, const size_t* src_origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, void* dst
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_read_image_1( ev->ev1, src->mem1[n], src_origin, region, 
		row_pitch, slice_pitch, dst);
}


void __do_set_cmd_write_image( 
	cl_event ev, 
	cl_mem dst, 
	const size_t* dst_origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, 
	const void* src
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_write_image_1( ev->ev1, dst->mem1[n], dst_origin, 
		region, row_pitch, slice_pitch, src);
}


void __do_set_cmd_copy_image( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	const size_t* src_origin, 
	const size_t* dst_origin, const size_t* region
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_copy_image_1(ev->ev1,src->mem1[n],dst->mem1[n],src_origin,
		dst_origin,region);
}


void __do_set_cmd_copy_image_to_buffer( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	const size_t* src_origin, const size_t* region, 
	size_t dst_offset
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_copy_image_to_buffer_1( ev->ev1, src->mem1[n], dst->mem1[n], 
		src_origin, region, dst_offset);
}


void __do_set_cmd_copy_buffer_to_image( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	size_t src_offset, 
	const size_t* dst_origin, const size_t* region
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_copy_buffer_to_image_1( ev->ev1, src->mem1[n], dst->mem1[n],
		src_offset, dst_origin, region);
}


void __do_set_cmd_map_buffer( 
	cl_event ev, 
	cl_mem membuf,
	cl_map_flags flags, size_t offset, size_t len,
	void* pp
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_map_buffer_1(ev->ev1, membuf->mem1[n], flags, offset, len, pp); 
}


void __do_set_cmd_map_image( 
	cl_event ev, 
	cl_mem image,
	cl_map_flags flags, const size_t* origin, const size_t* region,
	size_t* row_pitch, size_t* slice_pitch,
	void* p
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_map_image_1( ev->ev1, image->mem1[n], flags, origin, region, 
		row_pitch, slice_pitch, p);
}


void __do_set_cmd_unmap_memobj( 
	cl_event ev, 
	cl_mem memobj, void* p
)
{
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_unmap_memobj_1( ev->ev1, memobj->mem1[n], p); 
}


void __do_set_cmd_ndrange_kernel(
	cl_command_queue cmdq,
	cl_event ev,
	cl_kernel krn,
	cl_uint work_dim,
	const size_t* global_work_offset,
	const size_t* global_work_size,
	const size_t* local_work_size
)
{
	unsigned int n = ev->cmdq->devnum;
//	__do_set_cmd_ndrange_kernel_1( cmdq, ev->ev1, krn->krn1[n], work_dim, 
	__do_set_cmd_ndrange_kernel_1( ev->ev1, krn->krn1[n], work_dim, 
		global_work_offset, global_work_size, local_work_size);
}

void __do_set_cmd_task( cl_event ev, cl_kernel krn)
{ 
	unsigned int n = ev->cmdq->devnum;
	__do_set_cmd_task_1( ev->ev1, krn->krn1[n]); 
}


void __do_wait_for_events( cl_uint nev, const cl_event* evlist)
{
	int i;
	cl_event ev;

	for(i=0;i<nev;i++) 
		__do_wait_1(evlist[i]->ev1);

}

