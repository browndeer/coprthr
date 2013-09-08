/* imp_structs.h
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

#ifndef _imp_structs_h
#define _imp_structs_h

#include <sys/queue.h>
#include <pthread.h>

#include "cpuset_type.h"
#include "cmdcall.h"

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif
#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

struct _strtab_entry {
   size_t alloc_sz;
   size_t sz;
   char* buf;
};

struct _clsymtab_entry {
	cl_uint kind;
	cl_uint type;
};


#define CLSYM_KIND_


//#define __resolve_platformid(p,m) ((p)->m)

//extern void* __icd_call_vector;

#endif

