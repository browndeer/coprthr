/* clmalloc.h
 *
 * Copyright (c) 2009 Brown Deer Technology, LLC.  All Rights Reserved.
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


#ifndef _CLMALLOC_H
#define _CLMALLOC_H

#include "stdcl.h"

#define CL_MEM_WRITE 0x200

#define CLMEM_MAGIC     0x19661971
struct _memd_struct {
   union {
      struct {
         LIST_ENTRY(_memd_struct) memd_list;
         const cl_uint magic;
         cl_uint flags;
         size_t sz;
         cl_mem clbuf;
      };
      char __pad[64];
   };
};

#ifdef __cplusplus
extern "C" {
#endif

static __inline__ 
size_t clsizeofmem(void* ptr) 
{
	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;
	return(memd->sz);
}


void* clmalloc(CONTEXT* cp, size_t size, int flag);
void clfree( void* ptr );

cl_event clmsync(CONTEXT* cp, unsigned int devnum, void* ptr, int flags);


#ifdef __cplusplus
}
#endif


#endif

