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


/***
 *** low-level device memory operations
 ***/

static void* memalloc( size_t sz, int flags )
{
   struct coprthr1_mem* mem1 = (struct coprthr1_mem*)
      malloc(sizeof(struct coprthr1_mem));
   mem1->res = malloc(sz);
	printcl( CL_DEBUG "memalloc res=%p", mem1->res);
   return(mem1);
}

static void* memrealloc( void* ptr, size_t sz, int flags)
{ return realloc(ptr,sz); }

static void memfree( void* dptr, int flags )
{
   struct coprthr1_mem* mem1 = (struct coprthr1_mem*)dptr;
   if (mem1) {
      if (mem1->res) free(mem1->res);
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

struct coprthr_device_operations devops_x86_64 = {
	.memalloc = memalloc,
   .memrealloc = memrealloc,
   .memfree = memfree,
   .memread = memread,
   .memwrite = memwrite,
   .memcopy = memcopy
};


