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

#include <string.h>

#include "printcl.h"
#include "memobj.h"

#include "device.h"
#include "coprthr.h"


// device mem operations

/*
void* __coprthr_memalloc( size_t sz, int flags )
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
*/


void* coprthr_devmemalloc( 
	struct coprthr_device* dev,
	void* addr, size_t nmemb, size_t size,
	int flags
)
{
	printcl( CL_DEBUG "dev->devops=%p",dev->devops);
	printcl( CL_DEBUG "dev->devops->memalloc=%p",dev->devops->memalloc);

	if ( flags & COPRTHR_DEVMEM_TYPEMASK & dev->devinfo->memsup )
		return dev->devops->memalloc(nmemb*size,flags);
	else
		return 0;
}

void* coprthr_dmalloc( int dd, size_t sizeb, int flags )
{
	if (dd < 256 && __ddtab[dd]) 
		return
			coprthr_devmemalloc(__ddtab[dd],0,sizeb,1,COPRTHR_DEVMEM_TYPE_BUFFER);
	else 
		return(0);
}

void coprthr_devmemfree( struct coprthr_device* dev, struct coprthr1_mem* mem1 )
{
	if (dev) 
		dev->devops->memfree(mem1,0);
}


void coprthr_dfree( int dd, void* ptr )
{
	if (dd < 256 && __ddtab[dd]) 
		coprthr_devmemfree(__ddtab[dd],ptr);
}

void* coprthr_drealloc( int dd, void* dptr, size_t sizeb, int flags )
{
	if (dd < 256 && __ddtab[dd]) 
		return(__ddtab[dd]->devops->memrealloc(dptr,sizeb,flags));
	else 
		return(0);
}

void* coprthr_dmmap( int dd, void* dptr, size_t sizeb, int flags, void* ptr )
{
}

int coprthr_dmunmap( void* dptr, size_t sizeb )
{
}

int coprthr_dmsync( void* ptr, size_t sizeb, int flags )
{
}


