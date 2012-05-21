/* cmdcall_x86_64_k.c 
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

#include <CL/cl.h>

#include "xcl_structs.h"
#include "cmdcall.h"
#include "workp.h"
#include "util.h"

#include <setjmp.h>
#define __vc_setjmp  setjmp
#define __vc_longjmp longjmp
#define __vc_jmp_buf jmp_buf


static void* ndrange_kernel(cl_device_id devid, void* p)
{
	xclreport( XCL_DEBUG "cmdcall_x86_64:ndrange_kernel");

	int i;

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;


	xclreport( XCL_DEBUG "argp->flags %x\n",argp->flags);
	xclreport( XCL_DEBUG "argp->k.krn %p\n",argp->k.krn);
	xclreport( XCL_DEBUG "argp->k.krn->narg %d\n",argp->k.krn->narg);
	xclreport( XCL_DEBUG "argp->k.krn->narg %d\n",argp->k.krn->narg);
	

	xclreport( XCL_DEBUG "argp->k.word_dim %d\n",argp->k.work_dim);
	xclreport( XCL_DEBUG "argp->k.global_work_offset[] %d %d %d\n",
		argp->k.global_work_offset[0],
		argp->k.global_work_offset[1],
		argp->k.global_work_offset[2]);
	xclreport( XCL_DEBUG "argp->k.global_work_size[] %d %d %d\n",
		argp->k.global_work_size[0],
		argp->k.global_work_size[1],
		argp->k.global_work_size[2]);
	xclreport( XCL_DEBUG "argp->k.local_work_size[] %d %d %d\n",
		argp->k.local_work_size[0],
		argp->k.local_work_size[1],
		argp->k.local_work_size[2]);

	int base = __resolve_devid(devid,cpu.veid_base);
	int nve = __resolve_devid(devid,cpu.nve);
	vcproc_cmd(base,nve,argp);

	return(0); 
}

/****************************************************************/

#define VCORE_NC	64
#define VCORE_STACK_SZ     16384
#define VCORE_LOCAL_MEM_SZ 32768
#define VCORE_STACK_MASK (~(VCORE_STACK_SZ-1))

struct vcengine_data {
   int veid;
   void* funcp;
   void* callp;
   uint32_t* pr_arg_off;
   void* pr_arg_buf;
   int vc_runc;
};

unsigned int ne = 0;

static pthread_t* engine_td = 0;
static pthread_mutex_t* engine_mtx = 0;
static pthread_cond_t* engine_sig1 = 0;
static pthread_cond_t* engine_sig2 = 0;
static int* engine_ready = 0;
static int* engine_shutdown = 0;
static struct cmdcall_arg** engine_cmd_argp = 0;
static struct workp* common_engine_workp = 0;
static struct vcengine_data* engine_data = 0;
static char** engine_local_mem_base = 0;
static char** engine_local_mem_free = 0;
static size_t* engine_local_mem_sz = 0;

char* engine_stack_storage = 0;
char* engine_local_mem = 0;

#if defined(__x86_64__)
#define __callsp(sp,pf,argp) __asm volatile ( \
   "movq %0,%%rdi\n\t"  \
   "movq %1,%%rsp\n\t"  \
   "call *%2\n"   \
   : : "m" (argp), "m" (sp), "m" (pf)  \
   );
#elif defined(__arm__)
#define __callsp(sp,pf,argp) __asm volatile ( \
   "mov ip,sp\n\t" \
   "ldr %%sp,%1\n\t"  \
   "stmfd sp!,{ip}\n\t" \
   "ldr %%r0,%0\n\t"  \
   "mov lr,pc\n\t" \
   "bx %2\n"   \
   "ldmfd sp,{sp}\n\t" \
   : : "m" (argp), "m" (sp), "r" (pf)  \
   );
#else
#error unsupported architecture
#endif


static void* engine( void* p );

static void* engine_startup( void* p )
{
	xclreport( XCL_INFO "engine_startup");

	int i;

	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);

	ne = ncore;

	if (getenv("COPRTHR_VCORE_NE")) 
		ne = min(ne,atoi(getenv("COPRTHR_VCORE_NE")));

	xclreport( XCL_DEBUG "ne = %d",ne);

	engine_td = (pthread_t*)calloc(ne,sizeof(pthread_t));
	engine_mtx = (pthread_mutex_t*)calloc(ne,sizeof(pthread_mutex_t));
	engine_sig1 = (pthread_cond_t*)calloc(ne,sizeof(pthread_cond_t));
	engine_sig2 = (pthread_cond_t*)calloc(ne,sizeof(pthread_cond_t));
	engine_ready = (int*)calloc(ne,sizeof(int));
	engine_shutdown = (int*)calloc(ne,sizeof(int));
	engine_cmd_argp 
		= (struct cmdcall_arg**)calloc(ne,sizeof(struct cmdcall_arg*));
	engine_data 
		= (struct vcengine_data*)calloc(ne,sizeof(struct vcengine_data));
	engine_local_mem_base = (char**)calloc(ne,sizeof(char*));
	engine_local_mem_free = (char**)calloc(ne,sizeof(char*));
	engine_local_mem_sz = (size_t*)calloc(ne,sizeof(size_t));

	xclreport( XCL_DEBUG "alloc stack_storage: calloc(%d,1)"
		,(VCORE_NC+1)*VCORE_STACK_SZ*ne);

	engine_stack_storage = (char*)calloc((VCORE_NC+1)*VCORE_STACK_SZ*ne,1);
	mprotect(engine_stack_storage,
		(VCORE_NC+1)*VCORE_STACK_SZ*ne,PROT_READ|PROT_WRITE);

	engine_local_mem = (char*)calloc(VCORE_LOCAL_MEM_SZ*ne,1);


	pthread_attr_t td_attr;

	pthread_attr_init(&td_attr);
	pthread_attr_setdetachstate(&td_attr,PTHREAD_CREATE_JOINABLE);

	cpu_set_t mask;

	for(i=0;i<ne;i++) {

		pthread_mutex_init(&engine_mtx[i],0);
		pthread_cond_init(&engine_sig1[i],0);
		pthread_cond_init(&engine_sig2[i],0);

		engine_ready[i] = 0;
		engine_shutdown[i] = 0;
		engine_cmd_argp[i] = 0;
		engine_data[i].veid = i;
		engine_data[i].vc_runc = 0;

		xclreport( XCL_DEBUG "pthread_create %d",i); 

		pthread_create(&engine_td[i],&td_attr,engine,
			(void*)&engine_data[i]);

		CPU_ZERO(&mask);
		CPU_SET(i%ncore,&mask);
		if (pthread_setaffinity_np(engine_td[i],sizeof(cpu_set_t),&mask)) {
			xclreport( XCL_WARNING "engine: pthread_setaffinity_np failed");
		}

		pthread_yield();

		xclreport( XCL_DEBUG "wait until engine ready %d",i); 

		while(!engine_ready[i]) pthread_yield();

		xclreport( XCL_DEBUG "ready[%d] %d",i,engine_ready[i]);
	}

	pthread_attr_destroy(&td_attr);

	return(0);
}

struct work_struct {
   unsigned int tdim;
   size_t ltsz[3];
   size_t gsz[3];
   size_t gtsz[3];
   size_t gid[3];
   size_t gtid[3];
};

struct vc_data {
   int vcid;
   __vc_jmp_buf* vcengine_jbufp;
   __vc_jmp_buf* this_jbufp;
   __vc_jmp_buf* next_jbufp; 
   struct work_struct* workp;
   size_t ltid[3];
};

static void* engine( void* p )
{

	int i;
	struct vcengine_data* edata = (struct vcengine_data*)p;
	int veid = edata->veid;

	__vc_jmp_buf vcengine_jbuf;
	__vc_jmp_buf vc_jbuf[VCORE_NC];

	struct work_struct work;

	xclreport( XCL_DEBUG "engine-attempt-lock");
	pthread_mutex_lock(&engine_mtx[veid]);


	/* setup stacks and jbufs */

	size_t offset = veid*(VCORE_NC+1)*VCORE_STACK_SZ + VCORE_STACK_SZ;
	char* vc_stack_base 
		= (char*) (((intptr_t)engine_stack_storage+offset)&VCORE_STACK_MASK);

	char* vc_stack[VCORE_NC];

	xclreport( XCL_DEBUG "vc_stack_base %p\n",vc_stack_base);


	for(i=0;i<VCORE_NC;i++) {
	
		int vcid = i + VCORE_NC*veid;	

		vc_stack[i] = vc_stack_base + (i+1)*VCORE_STACK_SZ;

		struct vc_data* data = (struct vc_data*)(vc_stack[i]-VCORE_STACK_SZ);

		*data = (struct vc_data){vcid,&vcengine_jbuf,vc_jbuf+i,0,&work};

		xclreport( XCL_DEBUG "sp %p data %p\n",vc_stack[i],data);
	}


	xclreport( XCL_DEBUG "[%d] vcengine_jbuf %p\n",veid,&vcengine_jbuf);
	for(i=0;i<VCORE_NC;i++) 
		xclreport( XCL_DEBUG "vc_jbufp[%d] %p\n",i,vc_jbuf+i);


	engine_local_mem_free[veid] = engine_local_mem_base[veid] 
		= engine_local_mem + veid*VCORE_LOCAL_MEM_SZ;

	engine_local_mem_sz[veid] = VCORE_LOCAL_MEM_SZ;

	xclreport( XCL_DEBUG "engine_local_mem=%p",
		engine_local_mem);

	xclreport( XCL_DEBUG "engine_local_mem_free[%d]=%p",
		veid,engine_local_mem_free[veid]);

	xclreport( XCL_DEBUG "vcengine-set-ready");
	engine_ready[veid] = 1;


	int d,g;
	int nc,ng;
	int ii,gg;
	div_t qr;
	struct cmdcall_arg* cmd_argp;

	xclreport( XCL_DEBUG "[%d] ENTER BIG LOOP FOR VCENGINE\n",veid);

	do {

		xclreport( XCL_DEBUG "vcengine-wait1");

		pthread_cond_wait(&engine_sig1[veid],&engine_mtx[veid]);

		if (cmd_argp = engine_cmd_argp[veid]) {

			/* propagate info so that vcore() can execute specified kernel */

			xclreport( XCL_DEBUG "vcengine[%d]: krn %p",veid,cmd_argp->k.krn);
			edata->funcp = cmd_argp->k.ksym;
			edata->callp = cmd_argp->k.kcall;
			edata->pr_arg_buf = cmd_argp->k.pr_arg_buf;
			edata->pr_arg_off = cmd_argp->k.pr_arg_off;

			struct workp_entry* e = workp_get_entry( common_engine_workp, veid );

			report_workp_entry( XCL_DEBUG, e );

			work.tdim = e->ndr_dim;
			nc = ng = 1;
			size_t gid_offset[3];
			size_t gtid_offset[3];
			for(d = 0; d<work.tdim; d++) {

				work.gsz[d] = e->ndp_blk_end[d] - e->ndp_blk_first[d];
				gid_offset[d] =  e->ndp_blk_first[d];
				work.ltsz[d] = e->ndr_ltdsz[d];
				work.gtsz[d] = work.gsz[d] * work.ltsz[d];
				gtid_offset[d] = gid_offset[d] * work.ltsz[d];

				xclreport( XCL_DEBUG "vcengine[%d]: %d %d %d %d\n",
					veid,d,gtid_offset[d],work.gtsz[d],work.ltsz[d]);

				nc *= work.ltsz[d];
				ng *= work.gsz[d];
			} 

			xclreport( XCL_DEBUG "vcengine[%d]: nc=%d  ng=%d",veid,nc,ng);

			for(i=0;i<nc;i++) {

				struct vc_data* data =(struct vc_data*)(vc_stack[i]-VCORE_STACK_SZ);
				data->next_jbufp = vc_jbuf + (i+1)%nc;	/*****/


				int ii = i;
				switch(work.tdim) {
					case 3:
						qr = div(ii,(work.ltsz[1]*work.ltsz[0]));
						data->ltid[2] = qr.quot;
						ii = qr.rem;
					case 2:
						qr = div(ii,work.ltsz[0]);
						data->ltid[1] = qr.quot;
						ii = qr.rem;
					case 1:
						data->ltid[0] = ii;
					default: break;
				}

			} 

			edata->vc_runc = 0;

			xclreport( XCL_DEBUG "vcengine[%d]: num_of_groups %d",veid,ng);

			for(g=0;g<ng;g++) { /* loop over thread groups */

				/* set gid[], gtid[] */

				gg = g;
				switch(work.tdim) {
					case 3:
						qr = div(gg,(work.gsz[1]*work.gsz[0]));
						work.gid[2] = qr.quot + gid_offset[2];
						work.gtid[2] = qr.quot*work.ltsz[2] + gtid_offset[2];
						gg = qr.rem;
					case 2:
						qr = div(gg,work.gsz[0]);
						work.gid[1] = qr.quot + gid_offset[1];
						work.gtid[1] = qr.quot*work.ltsz[1] + gtid_offset[1];
						gg = qr.rem;
					case 1:
						work.gid[0] = gg + gid_offset[0];
						work.gtid[0] = gg*work.ltsz[0] + gtid_offset[0];
					default: break;
				} 

	
				switch(work.tdim) {
					case 3:
						xclreport( XCL_DEBUG 
							"vcengine[%d]: gid[]={%d,%d,%d} gtid[]={%d,%d,%d}",veid,
							work.gid[0],work.gid[1],work.gid[2],
							work.gtid[0],work.gtid[1],work.gtid[2]);
						break;
					case 2:
						xclreport( XCL_DEBUG 
							"vcengine[%d]: gid[]={%d,%d} gtid[]={%d,%d}",veid,
							work.gid[0],work.gid[1],
							work.gtid[0],work.gtid[1]);
						break;
					case 1:
						xclreport( XCL_DEBUG 
							"vcengine[%d]: gid[]={%d} gtid[]={%d}",veid,
							work.gid[0],
							work.gtid[0]);
						break;
					default: break;
				} 


				/* XXX temporary soln to the global size bookkeeping issue -DAR */
				for(d=0;d<work.tdim;d++) {
//					work.gtsz[d] = cmd_argp->k.global_work_size0[d];
					work.gtsz[d] = e->ndr_gtdsz[d];
					work.gsz[d] = work.gtsz[d]/work.ltsz[d];
				}

				xclreport( XCL_DEBUG "launching vcores (%d)",nc);

				char* sp;
				for(i=0;i<nc;i++) {
					if (!(__vc_setjmp(vcengine_jbuf))) {
						sp = vc_stack[i];
						xclreport( XCL_DEBUG "[%d] sp %p callp %p edata %p",
							i,sp,edata->callp,edata);
						__callsp(sp,edata->callp,edata);
					}
				}

				for(i=0;i<nc;i++) {

					if (!(__vc_setjmp(vcengine_jbuf))) __vc_longjmp(vc_jbuf[i],i+1);
				} 

				if (edata->vc_runc) {
					xclreport( XCL_DEBUG "vcengine[%d]: unterminated vcore",veid);
					exit(-1);
				} else {
					xclreport( XCL_DEBUG 
						"vcengine[%d]: all vcores completed",veid);
				} 

			} 

			engine_cmd_argp[veid] = 0;
			
		} 

		xclreport( XCL_DEBUG "vcengine-sig2");
		pthread_cond_signal(&engine_sig2[veid]);

	} while(!engine_shutdown[veid]);

	engine_shutdown[veid] = 2;

	pthread_mutex_unlock(&engine_mtx[veid]);
	xclreport( XCL_DEBUG "vcengine-unlock");

}



void*
engine_proc_cmd( int veid_base, int nve, struct cmdcall_arg* argp )
{
	int i,j;
	int e;

	int veid_end = veid_base + nve;

	xclreport( XCL_DEBUG "veid_base,nve %d,%d",veid_base,nve);

	struct cmdcall_arg* subcmd_argp 
		= (struct cmdcall_arg*)malloc(nve*sizeof(struct cmdcall_arg));


	for(e=veid_base;e<veid_end;e++) {

		/* must spin until engine is read to ensure valid local cache is set */
		/* XXX it would be better to spin once at initialization to take this
		/* XXX step out of the execution code -DAR */

		xclreport( XCL_DEBUG "ve[%d] vcproc_cmd-spin-until-ready",e);
		while(!engine_ready[e]) pthread_yield(); 

	}


	/* first apply correction to global ptrs */

	xclreport( XCL_DEBUG "cmdcall_x86_64:ndrange_kernel: fix global ptrs %p",
		argp->k.arg_kind);

	for(i=0;i<argp->k.krn->narg;i++) {

		xclreport( XCL_DEBUG  "fix global ptrs %d",i);

		xclreport( XCL_DEBUG "arg_kind=%d", argp->k.arg_kind[i]);

   cl_context ctx;
   unsigned int ndev;
   cl_device_id* devices;
   unsigned int n;

		void* p = (void*)(argp->k.pr_arg_buf + argp->k.pr_arg_off[i]);

		xclreport( XCL_DEBUG "XXX %d %p %p", 
			argp->k.pr_arg_off[i],
			argp->k.pr_arg_buf,
			p);

		switch(argp->k.arg_kind[i]) {

			case CLARG_KIND_CONSTANT:
			case CLARG_KIND_GLOBAL:

				{

				xclreport( XCL_DEBUG  "argp->k.pr_arg_off[%d]=%p",
					i,argp->k.pr_arg_off[i]);

				xclreport( XCL_DEBUG  "*cl_mem=%p",
					(*(cl_mem*)p));

				ctx = (*(cl_mem*)p)->ctx;
				ndev = ctx->ndev;
				devices = ctx->devices;
				n = 0;

				/* XXX this is a hack, redesign devnum/devid issue -DAR */

				*(void**)p =(*(cl_mem*)p)->imp.res[n];

				}

				break;

//			case CLARG_KIND_UNDEFINED:
//			case CLARG_KIND_VOID:
//			case CLARG_KIND_DATA:
//			case CLARG_KIND_LOCAL:
//			case CLARG_KIND_CONSTANT:
//			case CLARG_KIND_SAMPLER:
//			case CLARG_KIND_IMAGE2D:
//			case CLARG_KIND_IMAGE3D:

			default: break;
		}
	}


	/* make copies of *argp for each engine and allocate local mem */

	xclreport( XCL_DEBUG "make copies of *argp for each engine and allocate local mem");

	for(e=veid_base,i=0;e<veid_end;e++,i++) {

		memcpy(subcmd_argp+i,argp,sizeof(struct cmdcall_arg));

		__clone(subcmd_argp[i].k.pr_arg_off,argp->k.pr_arg_off,
			argp->k.narg,uint32_t);

   	__clone(subcmd_argp[i].k.pr_arg_buf,argp->k.pr_arg_buf,
			argp->k.arg_buf_sz,void);

		xclreport( XCL_DEBUG "ve[%d] arg_buf %p %p",
			e,argp->k.pr_arg_buf,subcmd_argp[i].k.pr_arg_buf);

   	for(j=0;j<subcmd_argp[i].k.narg;j++) 
			xclreport( XCL_DEBUG "ve[%d] arg_off[%d] %p",
				e,j,subcmd_argp[i].k.pr_arg_off[j]);

	}

	size_t sz;

	for(e=veid_base,i=0;e<veid_end;e++,i++) {

		xclreport( XCL_DEBUG "%p",&subcmd_argp[i].k.krn->narg);

		for(j=0;j<subcmd_argp[i].k.krn->narg;j++) {

			void* p = (intptr_t)subcmd_argp[i].k.pr_arg_buf
				+ subcmd_argp[i].k.pr_arg_off[j];

			switch(subcmd_argp[i].k.arg_kind[j]) {

				case CLARG_KIND_LOCAL:

					sz = *(size_t*)p;

					if (engine_local_mem_sz[e] < sz) {
						ERROR(__FILE__,__LINE__,"out of local mem");
						return((void*)-1);						
					}

					xclreport( XCL_DEBUG "ve[%d] argn %d alloc local mem %p %d",
						e,j,engine_local_mem_free[e],sz);

					*(void**)p = (void*)engine_local_mem_free[e];
					
					engine_local_mem_free[e] += sz;
					engine_local_mem_sz[e] -= sz;

					xclreport( XCL_DEBUG "ve[%d] local mem sz free %d",
						e,engine_local_mem_sz[e]);

					break;

//				case CLARG_KIND_UNDEFINED:
//				case CLARG_KIND_VOID:
//				case CLARG_KIND_DATA:
//				case CLARG_KIND_GLOBAL:
//				case CLARG_KIND_CONSTANT:
//				case CLARG_KIND_SAMPLER:
//				case CLARG_KIND_IMAGE2D:
//				case CLARG_KIND_IMAGE3D:

				default: break;
			}
		}

	}


	for(e=veid_base,i=0;e<veid_end;e++,i++) {

		xclreport( XCL_DEBUG "ve[%d] vcproc_cmd-spin-until-ready",e);
		while(!engine_ready[e]) pthread_yield(); 

		xclreport( XCL_DEBUG "ve[%d] vcproc_cmd-attempt-lock",e);
		pthread_mutex_lock(&engine_mtx[e]);

		engine_cmd_argp[e] = subcmd_argp+i;

		xclreport( XCL_DEBUG "ve[%d] vcproc_cmd-sig1",e);
		pthread_cond_signal(&engine_sig1[e]);

		xclreport( XCL_DEBUG "ve[%d] vcproc_cmd-unlock",e);
		pthread_mutex_unlock(&engine_mtx[e]);

	}


	pthread_yield(); 


	for(e=veid_base;e<veid_end;e++) {

		xclreport( XCL_DEBUG "ve[%d] vcproc_cmd-attempt-lock",e);
		pthread_mutex_lock(&engine_mtx[e]);

		xclreport( XCL_DEBUG "ve[%d] vcproc_cmd-wait2",e);
		if (engine_cmd_argp[e]) pthread_cond_wait(&engine_sig2[e],&engine_mtx[e]);

		xclreport( XCL_DEBUG "ve[%d] vcproc_cmd complete",e);

		engine_local_mem_free[e] = engine_local_mem_base[e];
		engine_local_mem_sz[e] = VCORE_LOCAL_MEM_SZ;

		xclreport( XCL_DEBUG "ve[%d] vcproc_cmd-unlock",e);
		pthread_mutex_unlock(&engine_mtx[e]);

	}

}



static void* 
exec_ndrange_kernel(cl_device_id devid, void* p)
{
	xclreport( XCL_DEBUG "cmdcall_x86_64:exec_ndrange_kernel");

	int i;

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;


	xclreport( XCL_DEBUG "argp->flags %x\n",argp->flags);
	xclreport( XCL_DEBUG "argp->k.krn %p\n",argp->k.krn);
	xclreport( XCL_DEBUG "argp->k.krn->narg %d\n",argp->k.krn->narg);
	xclreport( XCL_DEBUG "argp->k.krn->narg %d\n",argp->k.krn->narg);
	

	xclreport( XCL_DEBUG "argp->k.word_dim %d\n",argp->k.work_dim);
	xclreport( XCL_DEBUG "argp->k.global_work_offset[] %d %d %d\n",
		argp->k.global_work_offset[0],
		argp->k.global_work_offset[1],
		argp->k.global_work_offset[2]);
	xclreport( XCL_DEBUG "argp->k.global_work_size[] %d %d %d\n",
		argp->k.global_work_size[0],
		argp->k.global_work_size[1],
		argp->k.global_work_size[2]);
	xclreport( XCL_DEBUG "argp->k.local_work_size[] %d %d %d\n",
		argp->k.local_work_size[0],
		argp->k.local_work_size[1],
		argp->k.local_work_size[2]);

	int base = __resolve_devid(devid,cpu.veid_base);
	int nve = __resolve_devid(devid,cpu.nve);

#define safe_div(a,b) ((b==0)? 0 : a/b)
	struct workp_entry e0 = { 
		argp->k.work_dim,
		{ 
			argp->k.global_work_offset[0], 
			argp->k.global_work_offset[1], 
			argp->k.global_work_offset[2]
		},
		{
			argp->k.global_work_size[0],
			argp->k.global_work_size[1],
			argp->k.global_work_size[2]
		},
		{
			argp->k.local_work_size[0],
			argp->k.local_work_size[1],
			argp->k.local_work_size[2]
		},
		{ 0,0,0 },
		{
			safe_div(argp->k.global_work_size[0],argp->k.local_work_size[0]),
			safe_div(argp->k.global_work_size[1],argp->k.local_work_size[1]),
			safe_div(argp->k.global_work_size[2],argp->k.local_work_size[2])
		},
		{ 0,0,0 }

	};

	ne = 6;

	struct workp* wp = workp_alloc( ne );

	workp_init( wp );

	report_workp_entry(XCL_DEBUG,&e0);

	workp_genpart( wp, &e0 );

	workp_reset(wp);
	struct workp_entry* e;

	while (e = workp_nxt_entry(wp)) 
		report_workp_entry(XCL_DEBUG,e);

	if (!engine_td) {

		engine_startup(0);

	}

//	engine_klaunch();

	common_engine_workp = wp;

	engine_proc_cmd(base,nve,argp);

	workp_free(wp);

	common_engine_workp = 0;

//exit(-999);

	return(0); 
}




/********************************************************************/

static void* task(cl_device_id devid, void* argp)
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:task: unsupported");
	return(0); 
}


static void* native_kernel(cl_device_id devid, void* argp) 
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:native_kernel: unsupported");
	return(0); 
}


static void* read_buffer_safe(cl_device_id devid, void* p) 
{
	xclreport( XCL_DEBUG "cmdcall_x86_64:read_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.src)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = argp->m.dst;
//	void* src = ((cl_mem)argp->m.src)->host_ptr;
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst==src+offset) return(0);

	else if (src+offset < dst+len || dst < src+offset+len) 
		memmove(dst,src+offset,len);
	
	else memcpy(dst,src+offset,len);

	return(0);
}

static void* read_buffer(cl_device_id devid, void* p) 
{
	xclreport( XCL_DEBUG "cmdcall_x86_64:read_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.src)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = argp->m.dst;
//	void* src = ((cl_mem)argp->m.src)->host_ptr;
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst==src+offset) return(0);
	else memcpy(dst,src+offset,len);

	return(0);
}


static void* write_buffer_safe(cl_device_id devid, void* p) 
{
	xclreport( XCL_DEBUG "cmdcall_x86_64:write_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

//	void* dst = ((cl_mem)argp->m.dst)->host_ptr;
	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = argp->m.src;
	size_t offset = argp->m.dst_offset;
	size_t len = argp->m.len;

	if (dst+offset == src) return(0);

	else if (src < dst+offset+len || dst+offset < src+len) 
		memmove(dst,src+offset,len);
	
	else memcpy(dst+offset,src,len);

	return(0); 
}

static void* write_buffer(cl_device_id devid, void* p) 
{
	xclreport( XCL_DEBUG "cmdcall_x86_64:write_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

//	void* dst = ((cl_mem)argp->m.dst)->host_ptr;
	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = argp->m.src;
	size_t offset = argp->m.dst_offset;
	size_t len = argp->m.len;

	if (dst+offset == src) return(0);
	else memcpy(dst+offset,src,len);

	return(0); 
}


static void* copy_buffer_safe(cl_device_id devid, void* p)
{
	xclreport( XCL_DEBUG "cmdcall_x86_64:copy_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

//	void* dst = ((cl_mem)argp->m.dst)->host_ptr;
//	void* src = ((cl_mem)argp->m.src)->host_ptr;
	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t dst_offset = argp->m.dst_offset;
	size_t src_offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst+dst_offset == src+src_offset) return(0);

	else if (src+src_offset < dst+dst_offset+len 
		|| dst+dst_offset < src+src_offset+len) 
			memmove(dst+dst_offset,src+src_offset,len);
	
	else memcpy(dst+dst_offset,src+src_offset,len);

	return(0); 
}

static void* copy_buffer(cl_device_id devid, void* p)
{
	xclreport( XCL_DEBUG "cmdcall_x86_64:copy_buffer");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

//	void* dst = ((cl_mem)argp->m.dst)->host_ptr;
//	void* src = ((cl_mem)argp->m.src)->host_ptr;
	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t dst_offset = argp->m.dst_offset;
	size_t src_offset = argp->m.src_offset;
	size_t len = argp->m.len;

	if (dst+dst_offset == src+src_offset) return(0);
	else memcpy(dst+dst_offset,src+src_offset,len);

	return(0); 
}


static void* read_image(cl_device_id devid, void* p) 
{
//	WARN(__FILE__,__LINE__,"cmdcall_x86_64:read_image: unsupported");
//	return(0); 

	xclreport( XCL_DEBUG "cmdcall_x86_64:read_image");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.src)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

	void* dst = argp->m.dst;
//	void* src = ((cl_mem)argp->m.src)->host_ptr;
	void* src = ((cl_mem)argp->m.src)->imp.res[n];
	size_t offset = argp->m.src_offset + 128;

/*
	size_t len = argp->m.len;

	if (dst==src+offset) return(0);
	else memcpy(dst,src+offset,len);
*/

	/* XXX here we should check for 3D image, ignore for now -DAR */

	size_t esz = 4 * sizeof(float);
	size_t w = *(size_t*)src;

	if (argp->m.region[0] == w) {

		size_t len = argp->m.region[1] * argp->m.region[0] * esz;
		memcpy(dst,src+offset,len);

	} else {

		size_t len = argp->m.region[0] * esz;
		for(n=0;n<argp->m.region[1];n++) memcpy(dst+n*w,src+offset+n*w,len);

	}
	

	return(0);
}


static void* write_image(cl_device_id devid, void* p) 
{
//	WARN(__FILE__,__LINE__,"cmdcall_x86_64:write_image: unsupported");
//	return(0); 

	xclreport( XCL_DEBUG "cmdcall_x86_64:write_image");

	struct cmdcall_arg* argp = (struct cmdcall_arg*)p;

	cl_context ctx = ((cl_mem)argp->m.dst)->ctx;
	unsigned int ndev = ctx->ndev;
	cl_device_id* devices = ctx->devices;
	unsigned int n = 0;
	while (n < ndev && devices[n] != devid) ++n;

//	void* dst = ((cl_mem)argp->m.dst)->host_ptr;
	void* dst = ((cl_mem)argp->m.dst)->imp.res[n];
	void* src = argp->m.src;
	size_t offset = argp->m.dst_offset + 128;

/*
	size_t len = argp->m.len;

	xclreport( XCL_DEBUG "cmdcall_x86_64:write_image: XXX %p %p %d",dst+offset,src,len);

	if (dst+offset == src) return(0);
	else memcpy(dst+offset,src,len);

	size_t* sp = (size_t*)dst;
xclreport( XCL_DEBUG "cmdcall_x86_64:write_image: XXX %d %d %d",sp[0],sp[1],sp[16]);
*/

	/* XXX here we should check for 3D image, ignore for now -DAR */

	size_t esz = 4 * sizeof(float);
	size_t w = *(size_t*)dst;

	if (argp->m.region[0] == w) {

		size_t len = argp->m.region[1] * argp->m.region[0] * esz;
		memcpy(dst+offset,src,len);

	} else {

		size_t len = argp->m.region[0] * esz;
		for(n=0;n<argp->m.region[1];n++) memcpy(dst+offset+n*w,src+n*w,len);

	}
	

	return(0); 
}


static void* copy_image(cl_device_id devid, void* argp) 
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:copy_image: unsupported");
	return(0); 
}


static void* copy_image_to_buffer(cl_device_id devid, void* argp) 
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:copy_image_to_buffer: unsupported");
	return(0); 
}


static void* copy_buffer_to_image(cl_device_id devid, void* argp)
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:copy_buffer_to_image: unsupported");
	return(0); 
}


static void* map_buffer(cl_device_id devid, void* argp) 
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:map_buffer: unsupported");
	return(0); 
}


static void* map_image(cl_device_id devid, void* argp) 
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:map_image: unsupported");
	return(0); 
}


static void* unmap_mem_object(cl_device_id devid, void* argp) 
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:unmap_mem_object: unsupported");
	return(0); 
}


static void* marker(cl_device_id devid, void* p) 
{
	xclreport( XCL_DEBUG "cmdcall_x86_64:marker");
	return(0); 
}


static void* acquire_gl_objects(cl_device_id devid, void* argp)
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:acquire_gl_objects: unsupported");
	return(0); 
}


static void* release_gl_objects(cl_device_id devid, void* argp) 
{
	WARN(__FILE__,__LINE__,"cmdcall_x86_64:acquire_gl_objects: unsupported");
	return(0); 
}



/* 
 * XXX The *_safe versions of read/write/copy_buffer should be used for
 * XXX careful treatment of memory region overlap.  This can be added as
 * XXX a runtime option that simply modifies the cmdcall table. -DAR
 */

cmdcall_t cmdcall_x86_64_k[] = {
	0,
	exec_ndrange_kernel,
	task,
	native_kernel,
	read_buffer,	
	write_buffer,	
	copy_buffer,	
	read_image,
	write_image,
	copy_image,
	copy_image_to_buffer,
	copy_buffer_to_image,
	map_buffer,
	map_image,
	unmap_mem_object,
	marker,
	acquire_gl_objects,
	release_gl_objects
};


