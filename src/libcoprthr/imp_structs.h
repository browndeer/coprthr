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


#define CLARG_KIND_UNDEFINED	0x000
#define CLARG_KIND_VOID       0x001
#define CLARG_KIND_DATA       0x002
#define CLARG_KIND_GLOBAL     0x004
#define CLARG_KIND_LOCAL      0x008
#define CLARG_KIND_CONSTANT   0x010
#define CLARG_KIND_SAMPLER    0x020
#define CLARG_KIND_IMAGE2D    0x040
#define CLARG_KIND_IMAGE3D    0x080


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


/* platform */

struct _imp_platform {
   char* profile;
   char* version;
   char* name;
   char* vendor;
   char* extensions;
   unsigned int ndevices;
   struct _cl_device_id* dtab;
   struct _strtab_entry* dstrtab;
};

#define __imp_init_platform(imp) do { \
	bzero(&imp,sizeof(struct _imp_platform)); \
	} while(0)

#define __imp_free_platform(imp) do {} while(0)

#define __resolve_platformid(p,m) ((p)->imp.m)


/* memobj */

struct _imp_mem {
	int devnum;
	void** res;
	void** resmap;
	unsigned int host_cached;
	void* cache_ptr;
};

#define __imp_init_memobj(imp) do { \
	imp.res = 0; \
	imp.resmap = 0; \
	} while(0)

#define __imp_free_memobj(imp) do { \
	__free(imp.res); \
	__free(imp.resmap); \
	} while(0)


struct _elf_data {
	char filename[256];
	void* dlh;
	void* map;
};

#define __init_elf_data(ed) do { \
   (ed).filename[0] = '\0'; \
   (ed).dlh = 0; \
   (ed).map = 0; \
   } while(0)


extern void* __icd_call_vector;

#endif

