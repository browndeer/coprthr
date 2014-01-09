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

void* coprthr_devmemalloc( 
	struct coprthr_device* dev,
	void* addr, size_t nmemb, size_t size,
	int flags
)
{
	printcl( CL_DEBUG "dev->devops=%p",dev->devops);
	printcl( CL_DEBUG "dev->devops->memalloc=%p",dev->devops->memalloc);

	printcl( CL_DEBUG "memsup 0x%x",
		COPRTHR_DEVMEM_TYPEMASK & dev->devinfo->memsup);

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

void coprthr_devmemfree( struct coprthr_device* dev, struct coprthr_mem* mem1 )
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

size_t coprthr_memsize( struct coprthr_mem* mem )
{ 
	return mem->size; 
}

void* coprthr_memptr( struct coprthr_mem* mem, int flags )
{ 
	return mem->res; 
}


