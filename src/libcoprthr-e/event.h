/* event.h
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

#ifndef _event_h
#define _event_h

#include <CL/cl.h>

#include "xcl_structs.h"



void __do_release_event(cl_event ev);



/*
 * lock and sig
 */

#define __lock_event(ev) do { \
	DEBUG(__FILE__,__LINE__,"__lock_event: attempt %p",ev); \
	pthread_mutex_lock(&ev->imp.mtx); \
	DEBUG(__FILE__,__LINE__,"__lock_event: locked %p",ev); \
	} while(0)

#define __unlock_event(ev) do { \
	pthread_mutex_unlock(&ev->imp.mtx); \
	DEBUG(__FILE__,__LINE__,"__unlock_event: unlocked %p",ev); \
	} while(0)

#define __wait_event(ev) do { \
	DEBUG(__FILE__,__LINE__,"__wait_event: sleep%p",ev); \
	pthread_cond_wait(&ev->imp.sig,&ev->imp.mtx); \
	DEBUG(__FILE__,__LINE__,"__wait_event: wake%p",ev); \
	} while(0)

#define __sig_event(ev) do { \
	DEBUG(__FILE__,__LINE__,"__sig_event: %p",ev); \
	pthread_cond_signal(&ev->imp.sig); \
	} while(0)




/* 
 * set cmd 
 */

void __do_set_cmd_read_buffer( 
	cl_event ev, 
	cl_mem src, size_t src_offset, size_t len, 
	void* dst 
);

void __do_set_cmd_write_buffer( 
	cl_event ev, 
	cl_mem dst, size_t dst_offset, size_t len, 
	const void* src 
);

void __do_set_cmd_copy_buffer( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	size_t src_offset, size_t dst_offset, size_t len 
);

void __do_set_cmd_read_image( 
	cl_event ev, 
	cl_mem src, 
	const size_t* src_origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, 
	void* dst
);

void __do_set_cmd_write_image( 
	cl_event ev, 
	cl_mem dst, 
	const size_t* dst_origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, 
	const void* src
);

void __do_set_cmd_copy_image( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	const size_t* src_origin, 
	const size_t* dst_origin, const size_t* region
);

void __do_set_cmd_copy_image_to_buffer( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	const size_t* src_origin, const size_t* region, 
	size_t dst_offset
);

void __do_set_cmd_copy_buffer_to_image( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	size_t src_offset, 
	const size_t* dst_origin, const size_t* region
);

void __do_set_cmd_map_buffer( 
	cl_event ev, 
	cl_mem membuf, 
	cl_map_flags flags, size_t offset, size_t len,
	void* p
);

void __do_set_cmd_map_image( 
	cl_event ev, 
	cl_mem image, 
	cl_map_flags flags, 
	const size_t* origin, const size_t* region, 
	size_t* row_pitch, size_t* slice_pitch,
	void* p
);

void __do_set_cmd_unmap_memobj( 
	cl_event ev, 
	cl_mem memobj, void* mapped_ptr
);

void __do_set_cmd_ndrange_kernel(
	cl_command_queue cmdq,
	cl_event ev,
	cl_kernel krn,
	cl_uint work_dim,
	const size_t* global_work_offset,
	const size_t* global_work_size,
	const size_t* local_work_size
);

void __do_set_cmd_task(
	cl_event ev,
	cl_kernel krn
);


/*
 *	wait
 */

void __do_wait_for_events(cl_uint nev, const cl_event* evlist );



#endif

