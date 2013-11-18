/* cmdcall_x86_64_sl.c 
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

//#include "cmdcall.h"
//#include "workp.h"
//#include "sl_engine.h"

#include "coprthr_device.h"
#include "coprthr_mem.h"
#include "coprthr.h"

/***
 *** low-level device memory operations
 ***/

static void* memalloc( size_t size, int flags )
{
	printcl( CL_DEBUG "arch/x86_64: memalloc %ld 0x%x",size,flags);

   struct coprthr1_mem* mem1 = (struct coprthr1_mem*)
      malloc(sizeof(struct coprthr1_mem));

	switch( flags&COPRTHR_DEVMEM_TYPEMASK) {

		case COPRTHR_DEVMEM_TYPE_BUFFER:
			mem1->res = malloc(size);
			if (!mem1->res) 
				goto failed;
			mem1->type = COPRTHR_DEVMEM_TYPE_BUFFER;
			mem1->size = size;
			printcl( CL_DEBUG "memalloc type=BUFFER res=%p", mem1->res);
			break;

		case COPRTHR_DEVMEM_TYPE_MUTEX:
			printcl( CL_DEBUG "COPRTHR_DEVMEM_TYPE_MUTEX");
			if (size != 0) 
				goto failed;
			mem1->res = malloc(sizeof(pthread_mutex_t));
			printcl( CL_DEBUG "res=%p",mem1->res);
			if (!mem1->res) 
				goto failed;
			mem1->type = COPRTHR_DEVMEM_TYPE_MUTEX;
			mem1->size = sizeof(pthread_mutex_t);
			pthread_mutex_init( (pthread_mutex_t*)mem1->res, 0);
			printcl( CL_DEBUG "memalloc type=MUTEX res=%p", mem1->res);
			break;

		default:
			printcl( CL_ERR "unsupported devmem type 0x%x requested",
				flags&COPRTHR_DEVMEM_TYPEMASK);
			goto failed;
	}

   return(mem1);

failed:
	free(mem1);
	return(0);
}

static void* memrealloc( void* ptr, size_t sz, int flags)
{ return realloc(ptr,sz); }

static void memfree( void* dptr, int flags )
{
   struct coprthr1_mem* mem1 = (struct coprthr1_mem*)dptr;

   if (mem1) {

		switch( mem1->type&COPRTHR_DEVMEM_TYPEMASK) {

			case COPRTHR_DEVMEM_TYPE_MUTEX:
				pthread_mutex_init( (pthread_mutex_t*)mem1->res, 0);
				if (mem1->res) free(mem1->res);
				break;

			default:
				if (mem1->res) free(mem1->res);

		}

      free(mem1);
   }
}

static size_t memread( void* dptr, void* buf, size_t sz )
//{ memcpy(memptr,buf,sz); return sz; }
{ 
	struct coprthr1_mem* mem1 = (struct coprthr1_mem*)dptr;
	memcpy(buf,mem1->res,sz); 
	printcl( CL_DEBUG "memread res=%p",mem1->res);
	return sz; 
}

static size_t memwrite( void* dptr, void* buf, size_t sz )
{ 
	struct coprthr1_mem* mem1 = (struct coprthr1_mem*)dptr;
	memcpy(mem1->res,buf,sz); 
	printcl( CL_DEBUG "memwrite res=%p",mem1->res);
	return sz; 
}

static size_t memcopy( void* dptr_src, void* dptr_dst, size_t sz)
{ 
	struct coprthr1_mem* mem1_src = (struct coprthr1_mem*)dptr_src;
	struct coprthr1_mem* mem1_dst = (struct coprthr1_mem*)dptr_dst;
	memcpy(mem1_dst->res,mem1_src->res,sz); 
	return sz; 
}


static int mtxlock( void* mtxmem )
{
	printcl( CL_DEBUG "arch/x86_64: mtxlock %p",mtxmem);

	struct coprthr1_mem* mem = (struct coprthr1_mem*)mtxmem;

	pthread_mutex_t* p_mtx = (pthread_mutex_t*)mem->res;

	return pthread_mutex_lock(p_mtx);
}


static int mtxunlock( void* mtxmem )
{
	printcl( CL_DEBUG "arch/x86_64: mtxunlock %p",mtxmem);

	struct coprthr1_mem* mem = (struct coprthr1_mem*)mtxmem;

	pthread_mutex_t* p_mtx = (pthread_mutex_t*)mem->res;

	return pthread_mutex_unlock(p_mtx);
}


struct coprthr_device_operations devops_x86_64 = {
	.memalloc = memalloc,
   .memrealloc = memrealloc,
   .memfree = memfree,
   .memread = memread,
   .memwrite = memwrite,
   .memcopy = memcopy,
	.mtxlock = mtxlock,
	.mtxunlock = mtxunlock
};


