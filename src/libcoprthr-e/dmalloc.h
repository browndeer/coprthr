/* dmalloc.h
 *
 * Copyright (c) 2012 Brown Deer Technology, LLC.  All Rights Reserved.
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


#ifndef _dmalloc_h
#define _dmalloc_h

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void* devmembase;
extern void* devmemlo;
extern void* devmemhi;

void dmalloc_reset( void );

void* getdbrk(int devnum); 

void* dmalloc(int devnum, size_t size);

int dposix_memalign(int devnum, void** memptr, size_t alignment, size_t size);

void* dcalloc(int devnum, size_t num, size_t size);

void dfree(int devnum, void* ptr);

void* drealloc(int devnum, void* ptr, size_t size);

#ifdef __cplusplus
}
#endif

#endif


