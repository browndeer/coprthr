/* e32pth_engine_needham.c 
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
#include <sys/time.h>

#define __CL_MEM_OBJECT_ALLOCATION_FAILURE -4
#define __CL_INVALID_VALUE -30
#define __CL_OUT_OF_RESOURCES -5

#include "xcl_config.h"

#include "printcl.h"
#include "program.h"
#include "cmdcall.h"
#include "workp.h"

#include "coprthr_mem.h"

#include "e32_config_needham.h"
#include "e32pth_engine_needham.h"
#include "device.h"
#include "xxx.h"

#include "epiphany_api.h"

#include "e32pth_mem_if_needham.h"
#include "e32pth_if_needham.h"

#include "epiphany_api.h"

#define XXX_RUN 0x7f90
#define XXX_DEBUG 0x7f94
#define XXX_INFO 0x7f98

struct coord { int row,col; };

struct coord xxx_cores[] = {
	{0,0}, {0,1}, {0,2}, {0,3},
	{1,0}, {1,1}, {1,2}, {1,3},
	{2,0}, {2,1}, {2,2}, {2,3},
	{3,0}, {3,1}, {3,2}, {3,3}
};

unsigned int xxx_ncores = (sizeof(xxx_cores)/sizeof(struct coord));

extern void* loaded_srec;
extern int e_opened;

/* 
 * Note: this code is a reduction of the explicit engines used to control
 * execution on a co-processor device.  As such, it is not a true engine,
 * but follows the code structure for consistency of the API. -DAR 
 */

static nengines = 0;
static engine_is_started = 0;
static struct coprthr_device* dev = 0;
static size_t core_local_mem_size = 0;
static void* core_base_addr = 0;

int e32pth_engine_ready_needham() 
{ return((engine_is_started)? 1:0); }


void* e32pth_engine_startup_needham( void* p )
{
	int i;

	printcl( CL_INFO "e32_engine_startup: engine is path-through");

	/* XXX could put a test here to check if e-server is running -DAR */

	nengines = 1;

	engine_is_started = 1;

	dev = (struct coprthr_device*)p;
	core_local_mem_size = dev->devstate->e32.core_local_mem_size;
	core_base_addr = dev->devstate->e32.core_base_addr;

	printcl( CL_DEBUG "core_local_mem_size = %ld",core_local_mem_size);
	printcl( CL_DEBUG "core_base_addr = %p",core_base_addr);

	if (dev->devstate->e32.array_ncol != E32_COLS_IN_CHIP
		|| dev->devstate->e32.array_nrow != E32_ROWS_IN_CHIP) {

		printcl( CL_ERR "compare %d %d",dev->devstate->e32.array_ncol,
			E32_COLS_IN_CHIP);
		printcl( CL_ERR "compare %d %d",dev->devstate->e32.array_nrow,
			E32_ROWS_IN_CHIP);
		printcl( CL_ERR "e32 array layout mismatch, this is a fatal error");
		return((void*)(-1));

	}

	return(0);
}


#define foreach_core(r,c) for(icore=0,r=xxx_cores[icore].row,c=xxx_cores[icore].col; icore<xxx_ncores;icore++,r=xxx_cores[icore].row,c=xxx_cores[icore].col) 


void __dump_registers() 
{
	unsigned int r_config,r_status,r_pc,r_imask,r_ipend;
	int r,c,icore;
//	for(r=0;r<4;r++) for(c=0;c<4;c++) {
	foreach_core(r,c) {
		size_t sz = sizeof(unsigned int);
		size_t rc1 = e_read( &e_epiphany,r,c, E_REG_CONFIG, &r_config, sz);
		size_t rc2 = e_read( &e_epiphany,r,c, E_REG_STATUS, &r_status, sz);
		size_t rc3 = e_read( &e_epiphany,r,c, E_REG_PC, &r_pc, sz);
		size_t rc4 = e_read( &e_epiphany,r,c, E_REG_IMASK, &r_imask, sz);
		size_t rc5 = e_read( &e_epiphany,r,c, E_REG_IPEND, &r_ipend, sz);
		fprintf( stderr,
			"(%d,%d) %d %d %d %d %d reg:"
			" config=0x%x status=0x%x pc=0x%x imask=0x%x ipend=0x%x" "\n",
			r,c,rc1,rc2,rc3,rc4,rc5,r_config,r_status,r_pc,r_imask,r_ipend);
	}
}

int e32pth_engine_klaunch_needham( int engid_base, int ne, struct workp* wp,
	struct cmdcall_arg* argp )
{
	static int printcl_level = -1;
	if (printcl_level < 0) {
		char* envset = getenv("COPRTHR_CLMESG_LEVEL");
		printcl_level = (envset)? atoi(envset) : DEFAULT_CLMESG_LEVEL;
	}

//	e_set_host_verbosity(H_D3);
	e_set_host_verbosity(H_D0);

	int i;
	int icore,irow,icol;

   struct program_info_struct* proginfo
      = (struct program_info_struct*)(argp->k.krn->prg1->info);

	int knum = argp->k.krn->knum;

	printcl( CL_DEBUG "knum=%d",knum);

   int nthr = 1;
   switch(workp_get_entry(wp,0)->ndr_dim) {
      case 3: nthr *= workp_get_entry(wp,0)->ndr_ltdsz[2];
      case 2: nthr *= workp_get_entry(wp,0)->ndr_ltdsz[1];
      case 1: nthr *= workp_get_entry(wp,0)->ndr_ltdsz[0];
   }

   printcl( CL_DEBUG "threads per thread block %d",nthr);

   /* XXX hardcoded protection, fix to return insufficient resources -DAR */
   if (nthr > E32_NCORES) {
      printcl( CL_ERR "exceeded maximum thread block size");
      return(__CL_OUT_OF_RESOURCES);
   }

	printcl( CL_DEBUG "kernel prg1 %p knum %d",argp->k.krn->prg1,knum);
	printcl( CL_DEBUG "kernel su %d",argp->k.krn->prg1->ksu[knum]);

	size_t core_local_mem_hi = core_local_mem_size - 0x200;

	if (argp->k.krn->prg1->ksu[knum] > core_local_mem_hi ) {
		printcl( CL_DEBUG "ksu=%d > core_local_mem_hi=%d",
			argp->k.krn->prg1->ksu[knum],core_local_mem_hi);
		printcl( CL_ERR "exceeded maximum stack size");
		return(__CL_OUT_OF_RESOURCES);
	}

	size_t stack_base = core_local_mem_hi - argp->k.krn->prg1->ksu[knum];

	if ( stack_base < proginfo->core_local_data + 0xb0 ) {
		printcl( CL_ERR "compare %ld %ld",stack_base,
			proginfo->core_local_data+0xb0);
		printcl( CL_ERR "exceeded maximum stack size");
		return(__CL_OUT_OF_RESOURCES);
	}

	size_t repacked_arg_buf_sz = 0;

   unsigned int narg =  argp->k.krn->prg1->knarg[knum];

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

	e32_ptr_t device_mem_lo = (e32_ptr_t)devmemlo;
	e32_ptr_t device_mem_hi = (e32_ptr_t)getdbrk(0);
	int device_mem_fault = 0;

   printcl( CL_DEBUG "fix global ptrs %p", argp->k.arg_kind);

	size_t local_mem_free = proginfo->core_local_data + 0xe0;

	local_mem_free += (local_mem_free%16)? 16-local_mem_free%16 : 0;

	if (local_mem_free > stack_base) {
		printcl( CL_ERR "insufficient core-local memory");
		return(__CL_OUT_OF_RESOURCES);
	}

	size_t local_mem_sz =  stack_base - local_mem_free;

	printcl( CL_DEBUG "local_mem_base = %p local_mem_sz = %p",
      local_mem_free,local_mem_sz);

  	for(i=0;i<argp->k.krn->prg1->knarg[knum];i++) {

      printcl( CL_DEBUG  "fix global ptrs %d",i);

      printcl( CL_DEBUG "arg_kind=%d", argp->k.arg_kind[i]);

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

				struct coprthr_mem* mem1 = *(struct coprthr_mem**)p;

            printcl( CL_DEBUG  "mem1=%p", mem1);

            /* XXX this is a hack, redesign devnum/devid issue -DAR */

            *(void**)p = mem1->res;
            e32_ptr_t ptr = (e32_ptr_t)mem1->res;

            printcl( CL_DEBUG  "*(void**)p=%p", *(void**)p );

				if (ptr < device_mem_lo || ptr >= device_mem_hi) 
					device_mem_fault = 1;

            }

            break;

         case CLARG_KIND_LOCAL:

            {

            size_t sz = *(size_t*)p;

            size_t align;

            if (sz & 8) align = 8;
            else align = 16;

            local_mem_free
               += (local_mem_free%align)? align-local_mem_free%align : 0;

            printcl( CL_DEBUG "argn %d alloc local mem %p %d (%d)",
               i,local_mem_free,sz,align);

            if (sz > local_mem_sz) {

               *(void**)p = 0;
               printcl( CL_ERR "insuficient local memory");
               return(__CL_OUT_OF_RESOURCES);

            } else { /* XXX local mem allocated from core0 -DAR */

               *(void**)p = (void*)(local_mem_free|(intptr_t)core_base_addr);
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
		return(__CL_MEM_OBJECT_ALLOCATION_FAILURE);
	}

	e_set_host_verbosity(H_D0);
	e_set_loader_verbosity(0);

	if (printcl_level>6) __dump_registers();

	printcl( CL_DEBUG "BEFORE LOAD");

	if (1) {
		printcl( CL_DEBUG "need to load srec");
		printcl( CL_WARNING "hardcoded to devnum=0");


		printcl( CL_DEBUG "will try to load the srec file '%s'",
			argp->k.krn->prg1->kbinfile);

//		printcl( CL_CRIT "XXX attempt e_loaad");

		printcl( CL_DEBUG "send reset");
		e_reset_system();
		e_set_loader_verbosity(0);
		int err = e_load_group(argp->k.krn->prg1->kbinfile,
			&e_epiphany,0,0,4,4,0);

//		printcl( CL_CRIT "XXX e_loader returned %d",err);
 
		if (!err)
			loaded_srec = argp->k.krn->prg1->kbin;

		e_opened = 1;
	

	} else {
	
		printcl( CL_DEBUG "srec file '%s' already loaded",
			argp->k.krn->prg1->kbinfile);

		printcl( CL_DEBUG "sending reset and ILAT only");
		

	}

	printcl( CL_DEBUG "AFTER LOAD");

	if (printcl_level>6) __dump_registers();


	int corenum;
	struct core_control_struct core_control_data;

	unsigned int addr;
	size_t sz;


	struct timeval t0,t1;


	/***
	 *** mapping thread block to physical cores
	 ***/

	printcl( CL_DEBUG 
		"mapping thread block to physical cores (%d) %d %d %d = %d",
		workp_get_entry(wp,0)->ndr_dim,workp_get_entry(wp,0)->ndr_ltdsz[0],
		workp_get_entry(wp,0)->ndr_ltdsz[1], workp_get_entry(wp,0)->ndr_ltdsz[2],
		nthr);

	unsigned char coremap[E32_NCORES];
	unsigned char threadmap[E32_NCORES];

	for(i=0; i<E32_NCORES;  i++) {
		coremap[i] = (e32_uchar_t)255;
		threadmap[i] = (e32_uchar_t)255;
	}

	switch(workp_get_entry(wp,0)->ndr_dim) {
		
		case 1:
			{
				int m;
				for(m=0; m<nthr; m++) {
					coremap[m] = threadmap[m] = (e32_uchar_t)m;
				}
			}
			break;

		case 2:
			{
				int ltdsz0 = workp_get_entry(wp,0)->ndr_ltdsz[0];
				int ltdsz1 = workp_get_entry(wp,0)->ndr_ltdsz[1];
		
				if ( ltdsz1 == 1 || ltdsz0 <= nthr ) { 

					/* XXX added for testing -DAR */

					int m;
					for(m=0; m<nthr; m++) {
						coremap[m] = threadmap[m] = (e32_uchar_t)m;
					}
	
				} else if (ltdsz0 > E32_COLS_IN_CHIP || ltdsz1 > E32_ROWS_IN_CHIP) {

					printcl( CL_ERR "unsupported thread block layout");
      			return(__CL_INVALID_VALUE);

				} else { 

					/* thread block layout conforms to chip */

					int m;
					for(m=0; m<nthr; m++) {

						int j = m / ltdsz0;
						int i = m % ltdsz0;
						int r = j;
						int c = i;
						int n = r * E32_COLS_IN_CHIP + c;

						coremap[m] = (e32_uchar_t)n;
						threadmap[n] = (e32_uchar_t)m;

					}

				}

			}
			break;

		case 3:
		default:
			printcl( CL_ERR "invalid thread block dim");
      	return(__CL_INVALID_VALUE);
			break;

	} 

	for(i=0; i<E32_NCORES; i++) 
		printcl( CL_DEBUG "coremap[%d]=%d",i,coremap[i]);	

	for(i=0; i<E32_NCORES; i++) 
		printcl( CL_DEBUG "threadmap[%d]=%d",i,threadmap[i]);	

	e32_write_coremap(coremap);
	e32_write_threadmap(threadmap);


	printcl( CL_DEBUG "repacking workp data");
	e32_workp_entry_t we_e32[E32_NCORES];
	e32_init_we(we_e32,workp_get_entry(wp,0),1);
	e32_write_we(we_e32);
	printcl( CL_DEBUG "dim %d", we_e32[0][0] );
	for(i=0;i<E32_NCORES;i++) 
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

	/* XXX this hack will force alignment -DAR */
	unsigned int alignment;
	alignment = device_ptr_arg_off & 0x7;
	if (alignment) device_ptr_arg_off += (8-alignment);

	e32_ptr_t device_ptr_arg_buf = device_ptr_arg_off + narg*sizeof(e32_int_t);
	/* XXX this hack will force alignment -DAR */
	alignment = device_ptr_arg_buf & 0x7;
	if (alignment) device_ptr_arg_buf += (8-alignment);


	printcl( CL_DEBUG "write kernel arg data");
	e32_write_kdata_ptr_arg_off(device_ptr_arg_off);
	e32_write_kdata_ptr_arg_buf(device_ptr_arg_buf);

	printcl( CL_DEBUG "device_ptr_arg_off 0x%x",device_ptr_arg_off);
	printcl( CL_DEBUG "device_ptr_arg_buf 0x%x",device_ptr_arg_buf);

	xxx_e_write_zeropage(device_ptr_arg_off,e32_arg_off,narg*sizeof(e32_int_t));
	xxx_e_write_zeropage(device_ptr_arg_buf,e32_arg_buf,(intptr_t)(p-e32_arg_buf));

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

//	printcl( CL_DEBUG "readback kcall3 entry:");
//	e32_read_ctrl_callp(core_control_data.callp);
//	for (corenum=0; corenum<E32_NCORES;  corenum++) 
//		printcl( CL_DEBUG "readback kcall3 entry: %d 0x%x",
//			corenum,core_control_data.callp[corenum]);

	int fail=0;
	int retry_count=0;

retry:

//	printcl( CL_DEBUG "clear core ready state");
//	for (corenum=0; corenum<E32_NCORES;  corenum++) 
//   	core_control_data.ready[corenum] = 7;
//	e32_write_ctrl_ready(core_control_data.ready); 

	printcl( CL_DEBUG "clear core return values");
	for (corenum=0; corenum<E32_NCORES;  corenum++) 
   	core_control_data.retval[corenum] = 16;
	e32_write_ctrl_retval(core_control_data.retval); 


//	printcl( CL_DEBUG "write kcall3 entry 0x%x",argp->k.ksyms->kcall3);
//	for (corenum=0; corenum<E32_NCORES;  corenum++) 
//   	core_control_data.callp[corenum] = (e32_ptr_t)argp->k.ksyms->kcall3;
//	e32_write_ctrl_callp(core_control_data.callp);

//	printcl( CL_DEBUG "readback kcall3 entry:");
//	e32_read_ctrl_callp(core_control_data.callp);
//	for (corenum=0; corenum<E32_NCORES;  corenum++) 
//		printcl( CL_DEBUG "readback kcall3 entry: %d 0x%x",
//			corenum,core_control_data.callp[corenum]);


//#define foreach_core(r,c) for(r=0;r<2;r++) for(c=0;c<2;c++)
#define foreach_core(r,c) for(icore=0,r=xxx_cores[icore].row,c=xxx_cores[icore].col; icore<xxx_ncores;icore++,r=xxx_cores[icore].row,c=xxx_cores[icore].col) 

	int count = 0;
	int run_state = 1;
	int debug_state;
	int info_state;
	foreach_core(irow,icol) 
		e_write( &e_epiphany, irow, icol, XXX_RUN, &run_state, sizeof(int));
//	int zero = 0;
//	foreach_core(irow,icol) 
//		e_write( &e_epiphany, irow, icol, XXX_DEBUG, &zero, sizeof(int));
//	foreach_core(irow,icol) 
//		e_write( &e_epiphany, irow, icol, XXX_INFO, &zero, sizeof(int));

	if (printcl_level>6) __dump_registers();


/*
	while(run_state != 1) 	{
		e_read( &e_epiphany, 0, 0, XXX_RUN, &run_state, sizeof(int) );
		e_read( &e_epiphany, 0, 0, XXX_DEBUG, &debug_state, sizeof(int) );
		 e_read( &e_epiphany, 0, 0, XXX_INFO, &info_state, sizeof(int) );
		printcl( CL_DEBUG "run_state=%d debug_state=%d info=%d (0x%x)",
			run_state,debug_state,info_state,info_state);
	}
*/

//	int rc = e_start_group(&e_epiphany);
	foreach_core(irow,icol) {
		unsigned int r_pc = 0;
		int count = 0;
		do {
			int rc = e_start(&e_epiphany,irow,icol);
//			fprintf( stderr,"(%d)e_start returned %d\n",count,rc);
			printcl( CL_DEBUG "(%d)e_start returned %d",count,rc);
			e_read( &e_epiphany,irow,icol, E_REG_PC, &r_pc, sizeof(unsigned int));
		} while(!r_pc && ++count < 100);
		if (count == 100) {
			printcl( CL_ERR "core %d,%d failed to start",irow,icol);
		}
	}

	for(i=0;i<10;i++)	
		if (printcl_level>6) __dump_registers();

//	foreach_core(irow,icol) {
//		e_read( &e_epiphany, irow, icol, XXX_RUN, &run_state, sizeof(int) );
//		while(run_state == 1) 	{
//			e_read( &e_epiphany, irow, icol, XXX_RUN, &run_state, sizeof(int));
//			e_read( &e_epiphany, irow, icol, XXX_DEBUG, &debug_state, sizeof(int));
//			e_read( &e_epiphany, irow, icol, XXX_INFO, &info_state, sizeof(int));
//			printcl( CL_DEBUG "(%d,%d) run_state=%d debug_state=%d info=%d (0x%x)",
//				irow,icol,run_state,debug_state,info_state,info_state);
//		}
//	}

//	run_state = 3;
//	foreach_core(irow,icol) 
//		e_write( &e_epiphany, irow, icol, XXX_RUN, &run_state, sizeof(int));

	foreach_core(irow,icol) {
		e_read( &e_epiphany, irow, icol, XXX_RUN, &run_state, sizeof(int) );
		e_read( &e_epiphany, irow, icol, XXX_DEBUG, &debug_state, sizeof(int));
		e_read( &e_epiphany, irow, icol, XXX_INFO, &info_state, sizeof(int));
		printcl( CL_DEBUG "(%d,%d) run_state=%d debug_state=%d info=%d (0x%x)",
			irow,icol,run_state,debug_state,info_state,info_state);
		while(run_state > 0) 	{
			e_read( &e_epiphany, irow, icol, XXX_RUN, &run_state, sizeof(int));
			e_read( &e_epiphany, irow, icol, XXX_DEBUG, &debug_state, sizeof(int));
			e_read( &e_epiphany, irow, icol, XXX_INFO, &info_state, sizeof(int));
			printcl( CL_DEBUG "(%d,%d) run_state=%d debug_state=%d info=%d (0x%x)",
				irow,icol,run_state,debug_state,info_state,info_state);
			if (printcl_level>6) __dump_registers();
		}
	}

	if (printcl_level>6) __dump_registers();

/*
	printcl( CL_DEBUG "readback kcall3 entry:");
	e32_read_ctrl_callp(core_control_data.callp);
	for (corenum=0; corenum<E32_NCORES;  corenum++) 
		printcl( CL_DEBUG "readback kcall3 entry: %d 0x%x",
			corenum,core_control_data.callp[corenum]);
*/

	if (printcl_level>6) __dump_registers();

	printcl( CL_DEBUG "read back core return values");
	e32_read_ctrl_retval(core_control_data.retval);
   for(i=0;i<E32_NCORES;i++)
      printcl( CL_DEBUG " core %d retval %d (%x)",
			i,*(int*)&core_control_data.retval[i],
			*(int*)&core_control_data.retval[i] );

	if (printcl_level>6) __dump_registers();

//	printcl( CL_DEBUG "readback kcall3 entry:");
//	e32_read_ctrl_callp(core_control_data.callp);
//	for (corenum=0; corenum<E32_NCORES;  corenum++) 
//		printcl( CL_DEBUG "readback kcall3 entry (%p): %d 0x%x",
//			&core_control_data.callp[corenum],
//			corenum,core_control_data.callp[corenum]);


//	for(irow=0;irow<4;irow++) for(icol=0;icol<4;icol++) {
//	foreach_core(irow,icol) {
//		e_read( &e_epiphany, irow, icol, XXX_RUN, &run_state, sizeof(int) );
//		e_read( &e_epiphany, irow, icol, XXX_DEBUG, &debug_state, sizeof(int));
//		e_read( &e_epiphany, irow, icol, XXX_INFO, &info_state, sizeof(int));
//		printcl( CL_DEBUG "(%d,%d) run_state=%d debug_state=%d info=%d (0x%x)",
//			irow,icol,run_state,debug_state,info_state,info_state);
//	}

	return(0);

}

