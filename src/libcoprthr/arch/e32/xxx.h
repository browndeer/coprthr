/* device.h 
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

#ifndef _xxx_h
#define _xxx_h

/*
#include <CL/cl.h>

#include "xcl_structs.h"

void __do_discover_devices( unsigned int* ndevices, 
	struct _cl_device_id**, struct _strtab_entry* );

void __do_release_devices(struct _cl_device_id*, struct _strtab_entry*);

void __do_get_ndevices(cl_platform_id, cl_device_type, cl_uint*);

void __do_get_devices(cl_platform_id, cl_device_type, cl_uint, cl_device_id*);
*/

/*** 
 *** Epiphany stuff
 ***/

#include "epiphany_api.h"
#include "dmalloc.h"

#warning NEWAPI

#define xxx_e_read_dram( src, dst, len) do { \
   printcl( CL_DEBUG "xxx_e_read_dram &e_dram=%p src=%p (src-devmembase=%p)", \
		&e_dram,src,src-devmembase); \
   e_read( &e_dram,0,0, (src-devmembase), dst, len); \
   } while(0)

#define xxx_e_write_dram( dst, src, len) do { \
   printcl( CL_DEBUG "xxx_e_write_dram &e_dram=%p dst=%p (dst-devmembase=%p)", \
		&e_dram,dst,dst-devmembase); \
   e_write( &e_dram,0,0, (dst-devmembase), src, len); \
   } while(0)


extern void* devmembase;
extern void* devmemlo;

extern unsigned int devmtx_alloc_map;

#endif

