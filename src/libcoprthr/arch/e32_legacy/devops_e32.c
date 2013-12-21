/* cmdcall_e32.c 
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

#include "printcl.h"
//#include "cmdcall.h"
//#include "workp.h"
//#include "sl_engine.h"

#include "coprthr_device.h"
#include "coprthr_mem.h"
#include "coprthr.h"

#include "xxx.h"

/***
 *** low-level device memory operations
 ***/

static void* memalloc( size_t size, int flags )
{
	printcl( CL_DEBUG "arch/e32: memalloc %ld 0x%x",size,flags);

   struct coprthr1_mem* mem1 = (struct coprthr1_mem*)
      malloc(sizeof(struct coprthr1_mem));
//   mem1->res = malloc(sz);

/*
	if ( (flags&COPRTHR_DEVMEM_TYPE_BUFFER) )
   	mem1->res = dmalloc(0,sz);
	else 
		mem1->res = 0;
*/
	switch( flags&COPRTHR_DEVMEM_TYPEMASK) {

		case COPRTHR_DEVMEM_TYPE_BUFFER:
			mem1->res = dmalloc(0,size);
			if (!mem1->res)
				goto failed;
			mem1->type = COPRTHR_DEVMEM_TYPE_BUFFER;
			mem1->size = size;
			printcl( CL_DEBUG "memalloc type=BUFFER res=%p", mem1->res);
			break;

		case COPRTHR_DEVMEM_TYPE_MUTEX:
			{
			printcl( CL_DEBUG "COPRTHR_DEVMEM_TYPE_MUTEX");
			if (size != 0)
				goto failed;
			unsigned int map = devmtx_alloc_map;

//			intptr_t addr = (intptr_t)0x80800000 + 32768 - 120;
			intptr_t addr = (intptr_t)0x80807f90;
			int b;
			for(b=0;b<12;b++,addr+=8) 
				if ( (map & (1<<b)) == 0) 
					break;

			if (b == 12) 
				goto failed;

			map |= (1<<b);

			devmtx_alloc_map = map;
	
//			mem1->res = 0x80807fa0; //0x80800000+32768-128;
			mem1->res = (void*)addr;

			mem1->type = COPRTHR_DEVMEM_TYPE_MUTEX;
//			mem1->size = sizeof(e_mutex_t);
			mem1->size = 8;
			printcl( CL_DEBUG "memalloc type=MUTEX res=%p", mem1->res);
			struct ss_struct { int a, b; };
			struct ss_struct ss = { 0,0 };
//			size_t s = e_write_word( &e_epiphany, 0, mem1->res, 0);
//			s = e_write_word( &e_epiphany, 0, ((int*)mem1->res)+1, 0);

//			size_t s = e_write_word( &e_epiphany, 0, 0x80807fa0, 0);
//			s = e_write_word( &e_epiphany, 0, 0x80807fa4, 0);
//			printcl( CL_DEBUG "e_write_word returned %ld",s);

			xxx_e_write_dram((void*)addr,&ss,8);			

			break;
			}

		default:
			printcl( CL_ERR "unsupported devmem type 0x%x requested",
				flags&COPRTHR_DEVMEM_TYPEMASK);
			goto failed;

	}

//	if (mem1->res) 
//		printcl( CL_DEBUG "dmalloc %d bytes",sz);
//	else
//		printcl( CL_DEBUG "dmalloc failed");

   return(mem1);

failed:
	free(mem1);
	return;

}

/* XXX this should be eliminated */
static void* memrealloc( void* ptr, size_t sz, int flags)
{ return realloc(ptr,sz); }

static void memfree( void* dptr, int flags )
{

	printcl( CL_DEBUG "arch/e32: memfree %p 0x%x",dptr,flags);
	
   struct coprthr1_mem* mem1 = (struct coprthr1_mem*)dptr;

   if (mem1) {

		switch( mem1->type&COPRTHR_DEVMEM_TYPEMASK) {

			case COPRTHR_DEVMEM_TYPE_MUTEX:

				printcl( CL_DEBUG "arch/e32: memfree: MUTEX %p",mem1->res);

				if (mem1->res) {
					intptr_t addr = (intptr_t)mem1->res;
//					intptr_t offset = addr - ((intptr_t)0x80800000 + 32768 - 128);
					intptr_t offset = addr - ((intptr_t)0x80807f90);
					int b = offset/8;
					unsigned int map = devmtx_alloc_map;
					printcl( CL_DEBUG "arch/e32: memfree: MUTEX: orig map 0x%x",map);
					map &= ~(1<<b);
					printcl( CL_DEBUG "arch/e32: memfree: MUTEX: mod map 0x%x",map);
					devmtx_alloc_map = map;
				}
				break;

			default:
      		if (mem1->res) 
					dfree(0,mem1->res);

		}

      free(mem1);

   }
	
	printcl( CL_DEBUG "arch/e32: memfree: return");

}

static size_t memread( void* dptr, void* buf, size_t sz )
{ 
	struct coprthr1_mem* mem1 = (struct coprthr1_mem*)dptr;
	xxx_e_read_dram(mem1->res,buf,sz);
	return sz; 
}

static size_t memwrite( void* dptr, void* buf, size_t sz )
{ 
	struct coprthr1_mem* mem1 = (struct coprthr1_mem*)dptr;
	xxx_e_write_dram(mem1->res,buf,sz);
	return sz; 
}

static size_t memcopy( void* dptr_src, void* dptr_dst, size_t sz)
{ 
	struct coprthr1_mem* mem1_src = (struct coprthr1_mem*)dptr_src;
	struct coprthr1_mem* mem1_dst = (struct coprthr1_mem*)dptr_dst;
	void* ptmp = malloc(sz);
	xxx_e_read_dram(mem1_src->res,ptmp,sz);
	xxx_e_write_dram(mem1_dst->res,ptmp,sz);
	return sz; 
}

static 
__attribute__((noinline)) int __read_h( int* mtx ) 
{ return e_read_word(&e_epiphany, 0, (off_t)(mtx+1)); }

static 
__attribute__((noinline)) void __write_h( int* mtx, int val ) 
{ e_write_word(&e_epiphany, 0, (off_t)(mtx+1), val); }

static 
__attribute__((noinline)) int __read_m( int* mtx ) 
{ return e_read_word(&e_epiphany, 0, (off_t)mtx); }

static 
__attribute__((noinline)) void __write_m( int* mtx, int val ) 
{ e_write_word(&e_epiphany, 0, (off_t)mtx, val); }

static int mtxlock( void* mtxmem )
{
	printcl( CL_DEBUG "arch/e32: mtxlock %p",mtxmem);

	struct coprthr1_mem* mem = (struct coprthr1_mem*)mtxmem;

//	pthread_mutex_t* p_mtx = (pthread_mutex_t*)mem->res;
	unsigned int* p_mtx = (unsigned int*)mem->res;

//	return pthread_mutex_lock(p_mtx);
	printcl( CL_DEBUG "arch/e32: mtxunlock: writing 0xa5a5 to %p + 1",p_mtx);
	__write_h( p_mtx, 0xa5a5);
	while(__read_m(p_mtx));
	while(__read_m(p_mtx));
	return 0;
}

static int mtxunlock( void* mtxmem )
{
	printcl( CL_DEBUG "arch/e32: mtxunlock %p",mtxmem);

	struct coprthr1_mem* mem = (struct coprthr1_mem*)mtxmem;

//	pthread_mutex_t* p_mtx = (pthread_mutex_t*)mem->res;
	unsigned int* p_mtx = (unsigned int*)mem->res;

//	return pthread_mutex_unlock(p_mtx);
	printcl( CL_DEBUG "arch/e32: mtxunlock: writing 0 to %p + 1",p_mtx);

	__write_h(p_mtx,0);
	return 0;
}

struct coprthr_device_operations devops_e32 = {
	.memalloc = memalloc,
   .memrealloc = 0,
   .memfree = memfree,
   .memread = memread,
   .memwrite = memwrite,
   .memcopy = memcopy,
	.mtxlock = mtxlock,
	.mtxunlock = mtxunlock
};


