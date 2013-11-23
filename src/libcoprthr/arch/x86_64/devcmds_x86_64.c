/* devcmds_x86_64.c 
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

#undef _FORTIFY_SOURCE

#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmdcall.h"
#include "workp.h"
#include "sl_engine.h"

#include "coprthr_device.h"
#include "coprthr_mem.h"


/***
 *** asynchronous device commands
 ***/

static void* 
exec_ndrange_kernel(struct coprthr_device* dev, void* p)
{
	printcl( CL_DEBUG "cmdcall_x86_64:exec_ndrange_kernel");

	int i;

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	int vec_argp = 0;

	if (argp->k.nxt_argp) {
		printcl( CL_DEBUG "arch/x86_64 exec_ndrange_kernel: multiple argp");
		vec_argp = 1;

		/* XXX for now pre-check if we have a problem with too many argp -DAR */
//		int count = 1;
//		struct cmdcall_arg* a = argp;
//		while( a = a->k.nxt_argp ) {
//			if ( ++count > 16 ) {
//				printcl( CL_ERR "multiple argp exceeds limit");
//			}
//		}
	}


	/* initialization stuff */

	if (!sl_engine_ready()) sl_engine_startup(0);
	
	int base = dev->devstate->cpu.veid_base;
	int nve = dev->devstate->cpu.nve;
	printcl( CL_DEBUG "cpu.nve = %d", nve );

	struct workp* wp = workp_alloc( nve );
	workp_init( wp );


	struct cmdcall_arg* a = argp;
	int count = 0;

	do { 

		if (count >= nve) {
				printcl( CL_ERR "multiple argp exceeds limit");
				free(wp);
				return((void*)(-1));
		}

		printcl( CL_DEBUG "argp->flags %x (%d)",a->flags,count);
		printcl( CL_DEBUG "argp->k.krn %p (%d)",a->k.krn,count);

		printcl( CL_DEBUG "argp->k.word_dim %d (%d)",a->k.work_dim,count);
		printcl( CL_DEBUG "argp->k.global_work_offset[] %d %d %d (%d)",
			a->k.global_work_offset[0],
			a->k.global_work_offset[1],
			a->k.global_work_offset[2],count);
		printcl( CL_DEBUG "argp->k.global_work_size[] %d %d %d (%d)",
			a->k.global_work_size[0],
			a->k.global_work_size[1],
			a->k.global_work_size[2],count);
		printcl( CL_DEBUG "argp->k.local_work_size[] %d %d %d (%d)",
			a->k.local_work_size[0],
			a->k.local_work_size[1],
			a->k.local_work_size[2],count);

//	int base = dev->devstate->cpu.veid_base;
//	int nve = dev->devstate->cpu.nve;
//
//	printcl( CL_DEBUG "cpu.nve = %d", nve );

#define safe_div(a,b) ((b==0)? 0 : a/b)

		struct workp_entry e0 = { 
			a->k.work_dim,
			{ 
				a->k.global_work_offset[0], 
				a->k.global_work_offset[1], 
				a->k.global_work_offset[2]
			},
			{
				a->k.global_work_size[0],
				a->k.global_work_size[1],
				a->k.global_work_size[2]
			},
			{
				a->k.local_work_size[0],
				a->k.local_work_size[1],
				a->k.local_work_size[2]
			},
			{ 0,0,0 },
			{
				safe_div(a->k.global_work_size[0],a->k.local_work_size[0]),
				safe_div(a->k.global_work_size[1],a->k.local_work_size[1]),
				safe_div(a->k.global_work_size[2],a->k.local_work_size[2])
			},
			{ 0,0,0 }

		};

//	if (!sl_engine_ready()) sl_engine_startup(0);
//	
//	struct workp* wp = workp_alloc( nve );
//
//	workp_init( wp );

		report_workp_entry(CL_DEBUG,&e0);

		if (vec_argp)
			workp_set_entry( wp, count, &e0);
		else
			workp_genpart( wp, &e0 );

		count++;

	} while ( a = a->k.nxt_argp );

	
	/* if vec_argp set flag and adjust wp->n */

	if (vec_argp) {
		wp->flags = 1;
		wp->n = count;
	}


	/* report workp */

	workp_reset(wp);
	struct workp_entry* e;

	while (e = workp_nxt_entry(wp)) 
		report_workp_entry(CL_DEBUG,e);


	/* klaunch */

	if (vec_argp)
		sl_engine_klaunch(base,count,wp,argp);
	else
		sl_engine_klaunch(base,nve,wp,argp);


	workp_free(wp);

	return(0); 
}


static void* task( struct coprthr_device* dev, void* argp)
{
	printcl( CL_WARNING "cmdcall_x86_64:task: unsupported");
	return(0); 
}


static void* native_kernel( struct coprthr_device* dev, void* argp) 
{
	printcl( CL_WARNING "cmdcall_x86_64:native_kernel: unsupported");
	return(0); 
}


static void* read_buffer_safe(struct coprthr_device* dev, void* p) 
{
	printcl( CL_DEBUG "cmdcall_x86_64:read_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	void* dst = argp->m.dst;
	void* src = ((struct coprthr1_mem*)argp->m.src)->res;
	size_t offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst==src+offset) return(0);

	else if (src+offset < dst+len || dst < src+offset+len) 
		memmove(dst,src+offset,len);
	
	else memcpy(dst,src+offset,len);

	return(0);
}

static void* read_buffer( struct coprthr_device* dev, void* p) 
{
	printcl( CL_DEBUG "cmdcall_x86_64:read_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	void* dst = argp->m.dst;
	void* src = ((struct coprthr1_mem*)argp->m.src)->res;
	printcl( CL_DEBUG "cmdcall_x86_64:read_buffer m.src %p",argp->m.src);
	size_t offset = argp->m.src_offset;
	size_t len = argp->m.len;

	printcl( CL_DEBUG "%p %p %ld",dst,src,offset);

	if (dst==src+offset) return(0);
	else {
		printcl( CL_DEBUG "memcpy(%p,%p,%ld)",dst,src+offset,len);
		memcpy(dst,src+offset,len);
//      float* xxx = (float*)(dst);
//      int i;
//      for(i=0;i<10;i++) printcl( CL_DEBUG "xxx [%d] %f",i,xxx[i]);

	}

	return(0);
}


static void* write_buffer_safe(struct coprthr_device* dev, void* p) 
{
	printcl( CL_DEBUG "cmdcall_x86_64:write_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	void* dst = ((struct coprthr1_mem*)argp->m.dst)->res;
	void* src = argp->m.src;
	size_t offset = argp->m.dst_offset;
	size_t len = argp->m.len;

	if (dst+offset == src) return(0);

	else if (src < dst+offset+len || dst+offset < src+len) 
		memmove(dst,src+offset,len);
	
	else memcpy(dst+offset,src,len);

	return(0); 
}

static void* write_buffer(struct coprthr_device* dev, void* p) 
{
	printcl( CL_DEBUG "cmdcall_x86_64:write_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	void* dst = ((struct coprthr1_mem*)argp->m.dst)->res;
	void* src = argp->m.src;
	size_t offset = argp->m.dst_offset;
	size_t len = argp->m.len;

	if (dst+offset == src) return(0);
	else {
		printcl( CL_DEBUG "memcpy(%p,%p,%ld)",dst+offset,src,len);
		memcpy(dst+offset,src,len);
//		float* xxx = (float*)(dst+offset);
//		int i;
//		for(i=0;i<10;i++) printcl( CL_DEBUG "xxx [%d] %f",i,xxx[i]);
	}

	return(0); 
}


static void* copy_buffer_safe(struct coprthr_device* dev, void* p)
{
	printcl( CL_DEBUG "cmdcall_x86_64:copy_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	void* dst = ((struct coprthr1_mem*)argp->m.dst)->res;
	void* src = ((struct coprthr1_mem*)argp->m.src)->res;
	size_t dst_offset = argp->m.dst_offset;
	size_t src_offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst+dst_offset == src+src_offset) return(0);

	else if (src+src_offset < dst+dst_offset+len 
		|| dst+dst_offset < src+src_offset+len) 
			memmove(dst+dst_offset,src+src_offset,len);
	
	else memcpy(dst+dst_offset,src+src_offset,len);

	return(0); 
}

static void* copy_buffer(struct coprthr_device* dev, void* p)
{
	printcl( CL_DEBUG "cmdcall_x86_64:copy_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	void* dst = ((struct coprthr1_mem*)argp->m.dst)->res;
	void* src = ((struct coprthr1_mem*)argp->m.src)->res;
	size_t dst_offset = argp->m.dst_offset;
	size_t src_offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst+dst_offset == src+src_offset) return(0);
	else memcpy(dst+dst_offset,src+src_offset,len);

	return(0); 
}


static void* read_image(struct coprthr_device* dev, void* p) 
{
	printcl( CL_DEBUG "cmdcall_x86_64:read_image");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	void* dst = argp->m.dst;
	void* src = ((struct coprthr1_mem*)argp->m.src)->res;
	size_t offset = argp->m.src_offset + 128;

	size_t esz = 4 * sizeof(float);
	size_t w = *(size_t*)src;

	if (argp->m.region[0] == w) {

		size_t len = argp->m.region[1] * argp->m.region[0] * esz;
		memcpy(dst,src+offset,len);

	} else {

		int n;
		size_t len = argp->m.region[0] * esz;
		for(n=0;n<argp->m.region[1];n++) memcpy(dst+n*w,src+offset+n*w,len);

	}
	

	return(0);
}


static void* write_image(struct coprthr_device* dev, void* p) 
{
	printcl( CL_DEBUG "cmdcall_x86_64:write_image");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	void* dst = ((struct coprthr1_mem*)argp->m.dst)->res;
	void* src = argp->m.src;
	size_t offset = argp->m.dst_offset + 128;

	size_t esz = 4 * sizeof(float);
	size_t w = *(size_t*)dst;

	if (argp->m.region[0] == w) {

		size_t len = argp->m.region[1] * argp->m.region[0] * esz;
		memcpy(dst+offset,src,len);

	} else {

		int n;
		size_t len = argp->m.region[0] * esz;
		for(n=0;n<argp->m.region[1];n++) memcpy(dst+offset+n*w,src+n*w,len);

	}
	

	return(0); 
}


static void* copy_image(struct coprthr_device* dev, void* argp) 
{
	printcl( CL_WARNING "cmdcall_x86_64:copy_image: unsupported");
	return(0); 
}


static void* copy_image_to_buffer(struct coprthr_device* dev, void* argp) 
{
	printcl( CL_WARNING "cmdcall_x86_64:copy_image_to_buffer: unsupported");
	return(0); 
}


static void* copy_buffer_to_image(struct coprthr_device* dev, void* argp)
{
	printcl( CL_WARNING "cmdcall_x86_64:copy_buffer_to_image: unsupported");
	return(0); 
}


static void* map_buffer(struct coprthr_device* dev, void* p) 
{
	printcl( CL_DEBUG "cmdcall_x86_64:map_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

   size_t offset = argp->m.src_offset;
	size_t len = argp->m.len;

	void* ptr0 = malloc(len+2*sizeof(size_t));

	((size_t*)ptr0)[0] = offset;
	((size_t*)ptr0)[1] = len;

	void* ptr = ptr0 + 2*sizeof(size_t);

	*(void**)argp->m.dst = ptr;

   void* src = ((struct coprthr1_mem*)argp->m.src)->res;

   if (ptr==src+offset) return(0);
   else memcpy(ptr,src+offset,len);

	return(0);
}


static void* map_image(struct coprthr_device* dev, void* argp) 
{
	printcl( CL_WARNING "cmdcall_x86_64:map_image: unsupported");
	return(0); 
}


static void* unmap_mem_object(struct coprthr_device* dev, void* p) 
{

	printcl( CL_DEBUG "cmdcall_x86_64:unmap_mem_object");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	void* ptr = argp->m.dst;

	void* ptr0 = ptr - 2*sizeof(size_t);

	if ( __test_flags(argp->flags,__CL_MAP_WRITE) ) {

		void* src = ((struct coprthr1_mem*)argp->m.src)->res;

		size_t offset = ((size_t*)ptr0)[0] = offset;
		size_t len = ((size_t*)ptr0)[1] = len;

		if (src+offset == ptr) return(0);
		else memcpy(src+offset,ptr,len);

	}

	free( ptr0 );

	return(0);
}


static void* marker(struct coprthr_device* dev, void* p) 
{
	printcl( CL_DEBUG "cmdcall_x86_64:marker");
	return(0); 
}


static void* acquire_gl_objects(struct coprthr_device* dev, void* argp)
{
	printcl( CL_WARNING "cmdcall_x86_64:acquire_gl_objects: unsupported");
	return(0); 
}


static void* release_gl_objects(struct coprthr_device* dev, void* argp) 
{
	printcl( CL_WARNING "cmdcall_x86_64:acquire_gl_objects: unsupported");
	return(0); 
}



/* 
 * XXX The *_safe versions of read/write/copy_buffer should be used for
 * XXX careful treatment of memory region overlap.  This can be added as
 * XXX a runtime option that simply modifies the cmdcall table. -DAR
 */


struct coprthr_device_commands devcmds_x86_64 = {
   .cmd_ndrange_kernel = exec_ndrange_kernel,
   .cmd_task = task,
   .cmd_native_kernel = native_kernel,
   .cmd_read_buffer = read_buffer,
   .cmd_write_buffer = write_buffer,
   .cmd_copy_buffer = copy_buffer,
   .cmd_read_image = read_image,
   .cmd_write_image = write_image,
   .cmd_copy_image = copy_image,
   .cmd_copy_image_to_buffer = copy_image_to_buffer,
   .cmd_copy_buffer_to_image = copy_buffer_to_image,
   .cmd_map_buffer = map_buffer,
   .cmd_map_image = map_image,
   .cmd_unmap_mem_object = unmap_mem_object,
   .cmd_marker = marker,
   .cmd_acquire_gl_objects = acquire_gl_objects,
   .cmd_release_gl_objects = release_gl_objects
};


