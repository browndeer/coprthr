/* e32ser_engine.c 
 *
 * Copyright (c) 2009-2012 Brown Deer Technology, LLC.  All Rights Reserved.
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
#include <stdint.h>

#include <CL/cl.h>

#include "xcl_structs.h"
#include "printcl.h"
#include "program.h"
#include "cmdcall.h"
#include "workp.h"
//#include "util.h"

#include "e32_config.h"
#include "e32ser_engine.h"

#define __host__
#include "e32ser_mem_if.h"
#include "e32ser_if.h"
extern void* loaded_srec;
extern int e_opened;
extern char servIP[];
extern const unsigned short eServLoaderPort;

/* 
 * Note: this code is a reduction of the explicit engines used to control
 * execution on a co-processor device.  As such, it is not a true engine,
 * but follows the code structure for consistency of the API. -DAR 
 */

static nengines = 0;
static engine_is_started = 0;

int e32ser_engine_ready() 
{ return((engine_is_started)? 1:0); }

void* e32ser_engine_startup( void* p )
{
	int i;

	nengines = 1;

	printcl( CL_INFO "e32_engine_startup: engine is path-through");

	/* XXX could put a test here to check if e-server is running -DAR */

	engine_is_started = 1;

	return(0);
}


int
e32ser_engine_klaunch( int engid_base, int ne, struct workp* wp,
	struct cmdcall_arg* argp )
{

	int i;

   struct program_info_struct* proginfo
      = (struct program_info_struct*)(argp->k.krn->prg->imp.info);

   int nthr = 1;
   switch(workp_get_entry(wp,0)->ndr_dim) {
      case 3: nthr *= workp_get_entry(wp,0)->ndr_ltdsz[2];
      case 2: nthr *= workp_get_entry(wp,0)->ndr_ltdsz[1];
      case 1: nthr *= workp_get_entry(wp,0)->ndr_ltdsz[0];
   }

   printcl( CL_DEBUG "threads per thread block %d",nthr);

   if (nthr > E32_NCORES) {
      printcl( CL_ERR "exceeded maximum thread block size");
      return(CL_OUT_OF_RESOURCES);
   }

	printcl( CL_DEBUG "kernel su %d",argp->k.krn->imp.ksu);

	/* XXX this is an estimate - assumes we can allow 2048 bytes -DAR */
	if (argp->k.krn->imp.ksu > 0x800) {
		printcl( CL_ERR "exceeded maximum stack size");
		return(CL_OUT_OF_RESOURCES);
	}

	size_t repacked_arg_buf_sz = 0;

   unsigned int narg =  argp->k.krn->narg;

   for(i=0;i<narg;i++) {

      size_t arg_sz = argp->k.arg_sz[i];

      switch(argp->k.arg_kind[i]) {

         case CLARG_KIND_CONSTANT:
         case CLARG_KIND_GLOBAL:
         case CLARG_KIND_LOCAL:
            repacked_arg_buf_sz += sizeof(e32_ptr_t);
            break;

         case CLARG_KIND_IMAGE2D:
         case CLARG_KIND_IMAGE3D:
            { printcl( CL_ERR "image arg not supported"); }

         default:
            repacked_arg_buf_sz += arg_sz;
            break;
      }

   }

	e32_ptr_t device_mem_lo = E32_GLOBAL_MEM_BASE;
	e32_ptr_t device_mem_hi = getdbrk(0);
	int device_mem_fault = 0;

   printcl( CL_DEBUG "fix global ptrs %p", argp->k.arg_kind);

//	size_t local_mem_free = proginfo->core_local_data 
//		+ E32_SZ_CTRL + E32_SZ_WE + E32_SZ_KDATA
//		+ narg*sizeof(e32_int_t) + repacked_arg_buf_sz;
	size_t local_mem_free = proginfo->core_local_data +0x98;

	local_mem_free += (local_mem_free%16)? 16-local_mem_free%16 : 0;

//	if (local_mem_free > 0x7000) {
	if (local_mem_free > E32_CORE_LOCAL_MEM_HI - 0x1000) {
		printcl( CL_ERR "insufficient core-local memory");
		return(CL_OUT_OF_RESOURCES);
	}

//	size_t local_mem_sz =  0x7000 - local_mem_free;
	size_t local_mem_sz =  E32_CORE_LOCAL_MEM_HI - 0x1000 - local_mem_free;

	printcl( CL_DEBUG "local_mem_base = %p local_mem_sz = %p",
      local_mem_free,local_mem_sz);

  	for(i=0;i<argp->k.krn->narg;i++) {

      printcl( CL_DEBUG  "fix global ptrs %d",i);

      printcl( CL_DEBUG "arg_kind=%d", argp->k.arg_kind[i]);

   	cl_context ctx;
   	unsigned int ndev;
   	cl_device_id* devices;
   	unsigned int n;

      void* p = (void*)(argp->k.pr_arg_buf + argp->k.pr_arg_off[i]);

      printcl( CL_DEBUG "XXX %d %p %p",
         argp->k.pr_arg_off[i],
         argp->k.pr_arg_buf,
         p);

      switch(argp->k.arg_kind[i]) {

         case CLARG_KIND_CONSTANT:
         case CLARG_KIND_GLOBAL:

            {

            printcl( CL_DEBUG  "argp->k.pr_arg_off[%d]=%p",
               i,argp->k.pr_arg_off[i]);

            printcl( CL_DEBUG  "*cl_mem=%p", (*(cl_mem*)p));

            ctx = (*(cl_mem*)p)->ctx;
            ndev = ctx->ndev;
            devices = ctx->devices;
            n = 0;

            /* XXX this is a hack, redesign devnum/devid issue -DAR */

            e32_ptr_t ptr = *(void**)p = (*(cl_mem*)p)->imp.res[n];

            printcl( CL_DEBUG  "*(void**)p=%p", *(void**)p );

				if (ptr < device_mem_lo || ptr >= device_mem_hi) 
					device_mem_fault = 1;

            }

            break;

         case CLARG_KIND_LOCAL:

            {

            size_t sz = *(size_t*)p;

				size_t align;
//				if (sz & 1) align = 1;
//				else if (sz & 2) align = 2;
//				else if (sz & 4) align = 4;
//				else if (sz & 8) align = 8;
//				else align = 16;
				if (sz & 8) align = 8;
				else align = 16;

				local_mem_free 
					+= (local_mem_free%align)? align-local_mem_free%align : 0;

            printcl( CL_DEBUG "argn %d alloc local mem %p %d (%d)",
               i,local_mem_free,sz,align);

            if (sz > local_mem_sz) {

               *(void**)p = 0;
               printcl( CL_ERR "insuficient local memory");
               return(CL_OUT_OF_RESOURCES);

            } else {

               *(void**)p = (void*)local_mem_free;
               local_mem_free += sz;
               local_mem_sz -= sz;

            }

            printcl( CL_DEBUG "local mem sz free %d",local_mem_sz);

            }

            break;

//       case CLARG_KIND_UNDEFINED:
//       case CLARG_KIND_VOID:
//       case CLARG_KIND_DATA:
//       case CLARG_KIND_SAMPLER:
//       case CLARG_KIND_IMAGE2D:
//       case CLARG_KIND_IMAGE3D:

         default: break;
      }
   }

	if (device_mem_fault) {
		printcl( CL_ERR "device memory fault");
		return(CL_MEM_OBJECT_ALLOCATION_FAILURE);
	}

	/* XXX alloc local mem skipped -DAR */

	int corenum;
	struct core_control_struct core_control_data;

	printcl( CL_DEBUG "clear cores run state");
	for (corenum=0; corenum<E32_NCORES;  corenum++) 
   	core_control_data.run[corenum] = 0;
	e32_write_ctrl_run(core_control_data.run);

	if (loaded_srec != argp->k.krn->prg->imp.v_kbin[0] ) {
//	if (1) {

		printcl( CL_DEBUG "need to load srec");
		printcl( CL_WARNING "hardcoded to devnum=0");

		if (e_opened) {
			printcl( CL_DEBUG "closing e-server connection");
			if (e_close()) {
      		printcl( CL_ERR "cannot close connection to e-server");
				return(CL_DEVICE_NOT_AVAILABLE);
			}
			e_opened = 0;
		}

		printcl( CL_DEBUG "will try to load the srec file '%s'",
			argp->k.krn->prg->imp.v_kbin_tmpfile[0]);

		char cmd[1024];
		snprintf(cmd,1024,"e-loader -run_target %s\0",
			argp->k.krn->prg->imp.v_kbin_tmpfile[0]);

		printcl( CL_DEBUG "executing system command '%s'",cmd);

		int rc;
		if (rc = system(cmd)) {
			printcl( CL_ERR "e-loader failed, returned %d",rc);
			return(CL_DEVICE_NOT_AVAILABLE);
		}

		loaded_srec = argp->k.krn->prg->imp.v_kbin[0];

		printcl( CL_DEBUG "opening e-server connection");
		if (e_open((char *) servIP, eServLoaderPort)) {
      	printcl( CL_ERR "cannot open connection to e-server");
      	return(CL_DEVICE_NOT_AVAILABLE);
   	}
		e_opened = 1;
	

	} else {
	
		printcl( CL_DEBUG "srec file '%s' already loaded",
			argp->k.krn->prg->imp.v_kbin_tmpfile[0]);

	}

	unsigned int addr;
	size_t sz;

	printcl( CL_DEBUG "clear cores run state");
	for (corenum=0; corenum<E32_NCORES;  corenum++) 
   	core_control_data.run[corenum] = 0;
	e32_write_ctrl_run(core_control_data.run);

	printcl( CL_DEBUG "wait until all cores are ready");
   for (corenum=0; corenum<E32_NCORES;  corenum++) {
		e32_read_ctrl_ready_n(&core_control_data.ready[corenum],corenum);
      while (core_control_data.ready[corenum] == 0)
			e32_read_ctrl_ready_n(&core_control_data.ready[corenum],corenum);
   }


	printcl( CL_DEBUG "repacking workp data");
	e32_workp_entry_t we_e32[E32_NCORES];
	e32_init_we(we_e32,workp_get_entry(wp,0),E32_NCORES);
	e32_write_we(we_e32);
	printcl( CL_DEBUG "dim %d", we_e32[0][0] );
	for(i-0;i<E32_NCORES;i++) 
		printcl( CL_DEBUG "dim %d", we_e32[i][0] );


	/* XXX repack kernel arg data to match device arch -DAR */

	/* XXX note: the only issue addressed is converting pointers from
	 * XXX 64-bit to 32-bit.  all other args are copied exactly as
	 * XXX stored.  specific issue is structs which should not be used
	 * XXX as arguments since they cannot be reliably passed to another
	 * XXX architecture - this is a flaw in the OpenCL standard.  -DAR */

	printcl( CL_DEBUG "repacking kernel arg data");

	e32_int_t* e32_arg_off = (e32_int_t*)malloc(narg*sizeof(e32_int_t));
	/* XXX assume device arg_buf will be no larger than x86_64 -DAR */
	char* e32_arg_buf = malloc(argp->k.arg_buf_sz);

	char* p = e32_arg_buf;

	void* arg_buf = argp->k.pr_arg_buf;

	for(i=0;i<narg;i++) {

		e32_arg_off[i] = p-e32_arg_buf;

		size_t arg_off = argp->k.pr_arg_off[i];
		size_t arg_sz = argp->k.arg_sz[i];

		switch(argp->k.arg_kind[i]) {

         case CLARG_KIND_CONSTANT:
         case CLARG_KIND_GLOBAL:
       	case CLARG_KIND_LOCAL:
				*(e32_ptr_t*)p = (e32_ptr_t)*(void**)(arg_buf + (intptr_t)arg_off);
				p += sizeof(e32_ptr_t);
            break;

       	case CLARG_KIND_IMAGE2D:
       	case CLARG_KIND_IMAGE3D:
				{ printcl( CL_ERR "image arg not supported"); }

         default: 
				memcpy(p,arg_buf + (intptr_t)arg_off,arg_sz);
				p += arg_sz;
				break;
		}	

	}

	e32_ptr_t device_ptr_arg_off = E32_ZERO_PAGE_FREE;
	e32_ptr_t device_ptr_arg_buf = device_ptr_arg_off + narg*sizeof(e32_int_t);


	printcl( CL_DEBUG "write kernel arg data");
	e32_write_kdata_ptr_arg_off(device_ptr_arg_off);
	e32_write_kdata_ptr_arg_buf(device_ptr_arg_buf);

	printcl( CL_DEBUG "device_ptr_arg_off 0x%x",device_ptr_arg_off);
	printcl( CL_DEBUG "device_ptr_arg_buf 0x%x",device_ptr_arg_buf);

	e_write(device_ptr_arg_off,e32_arg_off,narg*sizeof(e32_int_t));
	e_write(device_ptr_arg_buf,e32_arg_buf,(intptr_t)(p-e32_arg_buf));

	for(i=0;i<narg;i++) {
		printcl( CL_DEBUG "e32_arg_off[%d] %d",i,
			((e32_int_t*)e32_arg_off)[i]);
	}
	for(i=0;i<(intptr_t)(p-e32_arg_buf)/sizeof(e32_ptr_t);i++) {
		printcl( CL_DEBUG "e32_arg_buf[%d] 0x%x",i,
			((e32_ptr_t*)e32_arg_buf)[i]);
	}

	printcl( CL_DEBUG "change in arg buf size %d -> %d",
		argp->k.arg_buf_sz,(intptr_t)(p-e32_arg_buf));

	printcl( CL_DEBUG "write kcall3 entry 0x%x",argp->k.ksyms->kcall3);
	for (corenum=0; corenum<E32_NCORES;  corenum++) 
   	core_control_data.callp[corenum] = (e32_ptr_t)argp->k.ksyms->kcall3;
	e32_write_ctrl_callp(core_control_data.callp);

/*
	int buffer[512];
	e_read(0x81004800,buffer,2048);
	for(i=0;i<32;i++) 
		printcl( CL_DEBUG "word[%d] %d",i,buffer[i]);
*/

	printcl( CL_DEBUG "clear core return values");
	for (corenum=0; corenum<E32_NCORES;  corenum++) 
   	core_control_data.retval[corenum] = 0;
	e32_write_ctrl_retval(core_control_data.retval); 

	printcl( CL_DEBUG "set cores to run");
	for (corenum=0; corenum<E32_NCORES;  corenum++) 
   	core_control_data.run[corenum] = 1;
	e32_write_ctrl_run(core_control_data.run);

	printcl( CL_DEBUG "wait until all cores are idle");
	for (corenum=0; corenum<E32_NCORES;  corenum++) {
		e32_read_ctrl_run(core_control_data.run);
		while (core_control_data.run[corenum]) {
			e32_read_ctrl_run(core_control_data.run);
			printcl( CL_DEBUG 
				"run %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
				core_control_data.run[0],
				core_control_data.run[1],
				core_control_data.run[2],
				core_control_data.run[3],
				core_control_data.run[4],
				core_control_data.run[5],
				core_control_data.run[6],
				core_control_data.run[7],
				core_control_data.run[8],
				core_control_data.run[9],
				core_control_data.run[10],
				core_control_data.run[11],
				core_control_data.run[12],
				core_control_data.run[13],
				core_control_data.run[14],
				core_control_data.run[15]);
		}
	}

/*
	printcl( CL_DEBUG "read back core return values");
	e32_read_ctrl_retval(core_control_data.retval);
   for(i=0;i<E32_NCORES;i++)
      printcl( CL_DEBUG " core %d retval %d (%x)",
			i,*(int*)&core_control_data.retval[i],
			*(int*)&core_control_data.retval[i] );
*/

//	e_read(0x81004800,buffer,2048);
//	for(i=0;i<32;i++) 
//		printcl( CL_DEBUG "word[%d] %d",i,buffer[i]);

	return(0);

}

