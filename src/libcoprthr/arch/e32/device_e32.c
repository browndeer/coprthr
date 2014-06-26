/* device_e32.c
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

#define ENABLE_OFFLINE_DEVICE

#define _GNU_SOURCE
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include "printcl.h"
#include "device.h"
#include "cmdcall.h"
//#include "cmdcall_e32_sl.h"
#include "compiler.h"
#include "program.h"
#include "memobj.h"

#include "coprthr_arch.h"
#include "coprthr_device.h"
#include "coprthr.h"

#if defined(__x86_64__) || defined(__arm__)
#ifdef ENABLE_EMEK_BUILD
#include "cmdcall_e32pth.h"
#else
#include "cmdcall_e32pth_needham.h"
#include "cmdcall_e32pth_needhampro.h"
#include "cmdcall_e32pth_blank.h"
#endif
#else
#error unsupported architecture
#endif

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif

#define __CLMAXSTR_LEN 1023
#define __CLMAXSTR_BUFSZ (__CLMAXSTR_LEN+1)

struct coprthr_device_commands devcmds_e32;
struct coprthr_device_operations devops_e32;

int bind_ksyms_default( struct _coprthr_ksyms_struct* ksyms, void* h,
   char* kname );

static char* truncate_ws(char* buf)
{
   size_t sz = strnlen(buf,__CLMAXSTR_LEN);
   char* p = buf + sz - 1;
   while( p>buf && (*p==' '||*p=='\t'||*p=='\n')) *(p--) = '\0';
   p = buf;
   while( p<buf+sz && (*p==' '||*p=='\t'||*p=='\n')) ++p;
   return(p);
}


#include "dmalloc.h"

void* loaded_srec = 0;
int e_opened = 0;


#include "epiphany_api.h"
#include "old_e_platform.h"

e_platform_t e_platform;
e_epiphany_t e_epiphany;
e_mem_t e_dram;

void* devmembase = 0;
void* devmemlo = 0;
void* devmemhi = 0;

unsigned int devmtx_alloc_map = 0;

static void* dlh_compiler = 0;

static int init_device_e32(void)
{
	int err;

	printcl( CL_DEBUG "init_device_e32");

	struct coprthr_device* codev = malloc(sizeof(struct coprthr_device));
	codev->devinfo = malloc(sizeof(struct coprthr_device_info));
	codev->devstate = malloc(sizeof(struct coprthr_device_state));
	codev->devops = malloc(sizeof(struct coprthr_device_operations));
	codev->devcomp = malloc(sizeof(struct coprthr_device_compiler));
	codev->devlink = malloc(sizeof(struct coprthr_device_linker));

/*** initialize epiphany device *****/

//   e_init(EPIPHANY_HDF);
	e_set_host_verbosity(H_D0);
   err = e_init(0);
	e_reset_system();
	printcl( CL_DEBUG "e_init() returned %d",err);

//   struct e_platform_info_struct einfo;
//   old_e_get_platform_info( 0, &einfo );

	/* XXX using new platform call only to check its 16-core parallella */
/*
	err = e_get_platform_info(&e_platform);
	printcl( CL_DEBUG "e_get_platform_info() returned %d",err);
	printcl( CL_DEBUG "platform infor from new call:");
	printcl( CL_DEBUG "version %s",e_platform.version);
	printcl( CL_DEBUG "(row,col) (%d,%d)",e_platform.row,e_platform.col);
	printcl( CL_DEBUG "(rows,cols) (%d,%d)",e_platform.rows,e_platform.cols);
	printcl( CL_DEBUG "num_chips %d", e_platform.num_chips);
	printcl( CL_DEBUG "num_emems %d", e_platform.num_emems);
*/

//   e_open(&e_epiphany, 0, 0, einfo.e_array_nrow, einfo.e_array_ncol );
   e_open(&e_epiphany, 0, 0, 4, 4);

   e_opened = 1;

   printcl( CL_DEBUG "e_alloc using &e_dram %p", &e_dram);

//   err = e_alloc( &e_dram, 0, (8192*4096) );
//   err = e_alloc( &e_dram, 0, (8192*4096 - 0x100000) );
   err = e_alloc( &e_dram, 0x100000, (8192*4096 - 0x100000) );

   if (err)
      printcl( CL_ERR "e_alloc returned %d", err);
   else
      printcl( CL_DEBUG "e_alloc ok, returned %d", err);

   printcl( CL_DEBUG "dram alloc:" );
//   printcl( CL_DEBUG
//      "map_size=%ld map_mask=0x%x phy_base=0x%x mapped_base=%p base=%p"
//      " memfd=%d", e_dram.map_size,e_dram.map_mask,e_dram.phy_base,
//      e_dram.mapped_base,e_dram.base,e_dram.memfd );

   struct old_e_platform_info_struct einfo;
   old_e_get_platform_info( 0, &einfo );

   printcl( CL_DEBUG "epiphany platform info:");
   printcl( CL_DEBUG "\tplatform_name '%s'",einfo.e_platform_name);
   printcl( CL_DEBUG "\tdevice_id %d",einfo.e_device_id);
   printcl( CL_DEBUG "\tglobal_mem_base %p",einfo.e_global_mem_base);
   printcl( CL_DEBUG "\tglobal_mem_size %ld",einfo.e_global_mem_size);
   printcl( CL_DEBUG "\tcore_local_mem_size %ld",einfo.e_core_local_mem_size);
   printcl( CL_DEBUG "\tcore_base_addr %p",einfo.e_core_base_addr);
   printcl( CL_DEBUG "\tarray_ncol %d",einfo.e_array_ncol);
   printcl( CL_DEBUG "\tarray_nrow %d",einfo.e_array_nrow);

   devmembase = einfo.e_global_mem_base;
   devmemlo = devmembase + 0x4000;
   devmemhi = devmemlo + einfo.e_global_mem_size;

//   dmalloc_reset();
	dmalloc_init(devmemlo,devmemhi);


	/***
	 *** set devinfo
	 ***/

	printcl( CL_DEBUG "start set devinfo");

	unsigned int ncpu = 1;

	*codev->devinfo = (struct coprthr_device_info){
		.memsup = COPRTHR_DEVMEM_TYPE_BUFFER | COPRTHR_DEVMEM_TYPE_MUTEX,
		.arch_id = COPRTHR_ARCH_ID_E32,
      .devsup = COPRTHR_DEVSUP_F_RUNTIME
         | COPRTHR_DEVSUP_F_STREAM | COPRTHR_DEVSUP_F_THREAD
         | COPRTHR_DEVSUP_F_MEM_BUFFER | COPRTHR_DEVSUP_F_MEM_MUTEX
	};

	codev->devinfo->name = 0;
	codev->devinfo->vendor = 0;
	codev->devinfo->drv_version = 0;
	codev->devinfo->profile = 0;
	codev->devinfo->version = 0;
	codev->devinfo->extensions = 0;

	
	codev->devinfo->max_compute_units 
		= einfo.e_array_ncol * einfo.e_array_nrow;

	codev->devinfo->max_freq = 1000;


	codev->devinfo->name = strdup(einfo.e_platform_name);
	codev->devinfo->vendor = strdup("Adapteva, Inc.");
	codev->devinfo->vendorid = einfo.e_device_id;
	codev->devinfo->global_mem_sz = einfo.e_global_mem_size;
	codev->devinfo->local_mem_sz = einfo.e_core_local_mem_size;


	#define __terminate(p) do { if (!p) p = strdup("unknown"); } while(0)
	__terminate(codev->devinfo->name);
	__terminate(codev->devinfo->vendor);
	__terminate(codev->devinfo->drv_version);
	__terminate(codev->devinfo->profile);
	__terminate(codev->devinfo->version);
	__terminate(codev->devinfo->extensions);



	/***
	 *** set devcomp, devlink, devops, devcmds
	 ***/

	dlh_compiler = dlopen("libcoprthrcc-e.so",RTLD_LAZY);
	if (!dlh_compiler) 
		printcl( CL_WARNING "no compiler,failed to load libcoprthrcc-e.so: %s",
			dlerror() );

	codev->devcomp->comp = dlsym(dlh_compiler,"compile_e32_needham");
	codev->devcomp->ilcomp = 0;
	codev->devlink->link = 0;
	codev->devlink->bind_ksyms = bind_ksyms_default;
	codev->devops = &devops_e32;
	codev->devcmds = &devcmds_e32;


	/***
	 *** set devstate
	 ***/

	if (!codev->devcomp->comp) {
		printcl( CL_WARNING "no compiler, dlsym failure");
		codev->devstate->compiler_avail = 0;
	} else {
		codev->devstate->compiler_avail = 1;
		codev->devinfo->devsup |= COPRTHR_DEVSUP_F_COMPILER;
	}

	int i;

	codev->devstate->avail = 1;
   codev->devstate->e32.core_local_mem_size = einfo.e_core_local_mem_size;
   codev->devstate->e32.core_base_addr = einfo.e_core_base_addr;
   codev->devstate->e32.array_ncol = einfo.e_array_ncol;
   codev->devstate->e32.array_nrow = einfo.e_array_nrow;
   codev->devstate->e32.ncore = (einfo.e_array_ncol)*(einfo.e_array_nrow);

	codev->devstate->e32.p_mutex_alloc_map = &devmtx_alloc_map;

	codev->devstate->cmdq = 0;

	codev->devstate->locked_pid = 0;


	coprthr_register_device(codev);

	return 0;

}

coprthr_device_init(init_device_e32);

