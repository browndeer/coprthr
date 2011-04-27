/* vcore.c
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

#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <signal.h>

#include "xcl_structs.h"	/* XXX this should be temporary! -DAR */

#include "vcore.h"
#include "cmdcall.h"
#include "util.h"


//#define NCPU 2 /* XXX this is a hack for testing, fix it -DAR */

extern void* vcore_v[];


//static pthread_t engine_td[VCORE_NE];
//static pthread_mutex_t engine_mtx[VCORE_NE];
//static pthread_cond_t engine_sig1[VCORE_NE];
//static pthread_cond_t engine_sig2[VCORE_NE];
//static int engine_ready[VCORE_NE];
//static int engine_shutdown[VCORE_NE];
//static struct cmdcall_arg* engine_cmd_argp[VCORE_NE];
//static struct vcengine_data engine_data[VCORE_NE];
//static char* engine_local_mem_base[VCORE_NE];
//static char* engine_local_mem_free[VCORE_NE];
//static size_t engine_local_mem_sz[VCORE_NE];
//
//char vc_stack_storage[(VCORE_NC+1)*VCORE_STACK_SZ*VCORE_NE];
//char ve_local_mem[VCORE_LOCAL_MEM_SZ*VCORE_NE];

unsigned int vcore_ne = 0;

static pthread_t* engine_td = 0;
static pthread_mutex_t* engine_mtx = 0;
static pthread_cond_t* engine_sig1 = 0;
static pthread_cond_t* engine_sig2 = 0;
static int* engine_ready = 0;
static int* engine_shutdown = 0;
static struct cmdcall_arg** engine_cmd_argp = 0;
static struct vcengine_data* engine_data = 0;
static char** engine_local_mem_base = 0;
static char** engine_local_mem_free = 0;
static size_t* engine_local_mem_sz = 0;

char* vc_stack_storage = 0;
char* ve_local_mem = 0;


#define __callsp(sp,pf,argp) __asm volatile ( \
	"movq %0,%%rdi\n\t"	\
	"movq %1,%%rsp\n\t" 	\
	"call *%2\n"	\
	: : "m" (argp), "m" (sp), "m" (pf) 	\
	);


static void* 
vcengine( void* p )
{
	int i;
	struct vcengine_data* edata = (struct vcengine_data*)p;
	int veid = edata->veid;

//	jmp_buf vcengine_jbuf;
//	jmp_buf vc_jbuf[VCORE_NC];
	__vc_jmp_buf vcengine_jbuf;
	__vc_jmp_buf vc_jbuf[VCORE_NC];
//	char vc_stack_storage[(VCORE_NC+1)*VCORE_STACK_SZ];
//	char ve_local_mem[VCORE_LOCAL_MEM_SZ];

	struct work_struct work;

//	cpu_set_t mask;
//	if (sched_getaffinity(0,sizeof(cpu_set_t),&mask)) {
//		WARN(__FILE__,__LINE__,"vcengine: sched_getaffinity failed");
//	} else {
//		CPU_ZERO(&mask);
//		CPU_SET(veid%NCPU,&mask);
//		pid_t td = syscall(SYS_gettid);
//		if (sched_setaffinity(td,sizeof(cpu_set_t),&mask)) {
//			WARN(__FILE__,__LINE__,"vcengine: sched_setaffinity failed");
//		}
//	}



	DEBUG(__FILE__,__LINE__,"vcengine-attempt-lock");
	pthread_mutex_lock(&engine_mtx[veid]);


	/* setup stacks and jbufs */

	size_t offset = veid*(VCORE_NC+1)*VCORE_STACK_SZ + VCORE_STACK_SZ;
	char* vc_stack_base 
//		= (char*) (((intptr_t)vc_stack_storage+VCORE_STACK_SZ)&VCORE_STACK_MASK);
		= (char*) (((intptr_t)vc_stack_storage+offset)&VCORE_STACK_MASK);

	char* vc_stack[VCORE_NC];

	DEBUG(__FILE__,__LINE__,"vc_stack_base %p\n",vc_stack_base);


	for(i=0;i<VCORE_NC;i++) {
	
		int vcid = i + VCORE_NC*veid;	

		vc_stack[i] = vc_stack_base + (i+1)*VCORE_STACK_SZ;

		struct vc_data* data = (struct vc_data*)(vc_stack[i]-VCORE_STACK_SZ);

		*data = (struct vc_data){vcid,&vcengine_jbuf,vc_jbuf+i,0,&work};

		DEBUG(__FILE__,__LINE__,"sp %p data %p\n",vc_stack[i],data);
	}


	DEBUG(__FILE__,__LINE__,"[%d] vcengine_jbuf %p\n",veid,&vcengine_jbuf);
	for(i=0;i<VCORE_NC;i++) 
		DEBUG(__FILE__,__LINE__,"vc_jbufp[%d] %p\n",i,vc_jbuf+i);


	engine_local_mem_free[veid] = engine_local_mem_base[veid] 
		= ve_local_mem + veid*VCORE_LOCAL_MEM_SZ;

	engine_local_mem_sz[veid] = VCORE_LOCAL_MEM_SZ;
	

	DEBUG(__FILE__,__LINE__,"vcengine-set-ready");
	engine_ready[veid] = 1;


	int d,g;
	int nc,ng;
	int ii,gg;
	div_t qr;
	struct cmdcall_arg* cmd_argp;

	DEBUG(__FILE__,__LINE__,"[%d] ENTER BIG LOOP FOR VCENGINE\n",veid);

	do {

		DEBUG(__FILE__,__LINE__,"vcengine-wait1");

		pthread_cond_wait(&engine_sig1[veid],&engine_mtx[veid]);

		if (cmd_argp = engine_cmd_argp[veid]) {

			/* propagate info so that vcore() can execute specified kernel */

			DEBUG(__FILE__,__LINE__,"vcengine[%d]: krn %p",veid,cmd_argp->k.krn);
			edata->funcp = cmd_argp->k.ksym;
			edata->callp = cmd_argp->k.kcall;
			edata->pr_arg_vec = cmd_argp->k.pr_arg_vec;

			work.tdim = cmd_argp->k.work_dim;
			nc = ng = 1;
			size_t gid_offset[3];
			size_t gtid_offset[3];
			for(d = 0; d<work.tdim; d++) {

				work.gtsz[d] = cmd_argp->k.global_work_size[d];
				work.ltsz[d] = cmd_argp->k.local_work_size[d];

				work.gsz[d] = work.gtsz[d]/work.ltsz[d];

				gtid_offset[d] = cmd_argp->k.global_work_offset[d];
				gid_offset[d] = gtid_offset[d]/work.ltsz[d];

				DEBUG(__FILE__,__LINE__,"vcengine[%d]: %d %d %d %d\n",
					veid,d,gtid_offset[d],work.gtsz[d],work.ltsz[d]);

				nc *= work.ltsz[d];
				ng *= work.gsz[d];
			} 

			DEBUG(__FILE__,__LINE__,"vcengine[%d]: nc=%d  ng=%d",veid,nc,ng);

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

			DEBUG(__FILE__,__LINE__,"vcengine[%d]: num_of_groups %d",veid,ng);

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
						DEBUG(__FILE__,__LINE__,
							"vcengine[%d]: gid[]={%d,%d,%d} gtid[]={%d,%d,%d}",veid,
							work.gid[0],work.gid[1],work.gid[2],
							work.gtid[0],work.gtid[1],work.gtid[2]);
						break;
					case 2:
						DEBUG(__FILE__,__LINE__,
							"vcengine[%d]: gid[]={%d,%d} gtid[]={%d,%d}",veid,
							work.gid[0],work.gid[1],
							work.gtid[0],work.gtid[1]);
						break;
					case 1:
						DEBUG(__FILE__,__LINE__,
							"vcengine[%d]: gid[]={%d} gtid[]={%d}",veid,
							work.gid[0],
							work.gtid[0]);
						break;
					default: break;
				} 

				DEBUG(__FILE__,__LINE__,"launching vcores (%d)",nc);

				char* sp;
				for(i=0;i<nc;i++) {
//					if (!(setjmp(vcengine_jbuf))) {
					if (!(__vc_setjmp(vcengine_jbuf))) {
						sp = vc_stack[i];
						DEBUG(__FILE__,__LINE__,"[%d] sp %p edata %p",i,sp,edata);
//						printf("%p ",sp);
						__callsp(sp,edata->callp,edata);
					}
				}

				for(i=0;i<nc;i++) {

//					if (!(setjmp(vcengine_jbuf))) longjmp(vc_jbuf[i],i+1);
					if (!(__vc_setjmp(vcengine_jbuf))) __vc_longjmp(vc_jbuf[i],i+1);
				} 

				if (edata->vc_runc) {
					DEBUG(__FILE__,__LINE__,"vcengine[%d]: unterminated vcore",veid);
					exit(-1);
				} else {
					DEBUG(__FILE__,__LINE__,
						"vcengine[%d]: all vcores completed",veid);
				} 

			} 

			engine_cmd_argp[veid] = 0;
			
		} 

		DEBUG(__FILE__,__LINE__,"vcengine-sig2");
		pthread_cond_signal(&engine_sig2[veid]);

	} while(!engine_shutdown[veid]);

	engine_shutdown[veid] = 2;

	pthread_mutex_unlock(&engine_mtx[veid]);
	DEBUG(__FILE__,__LINE__,"vcengine-unlock");

}


void* 
vcproc_startup( void* p )
{
	DEBUG(__FILE__,__LINE__,"vcproc_create");

	int i;

	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);

	vcore_ne = ncore;

	DEBUG(__FILE__,__LINE__,"vcore_ne = %d",vcore_ne);

	engine_td = (pthread_t*)calloc(vcore_ne,sizeof(pthread_t));
	engine_mtx = (pthread_mutex_t*)calloc(vcore_ne,sizeof(pthread_mutex_t));
	engine_sig1 = (pthread_cond_t*)calloc(vcore_ne,sizeof(pthread_cond_t));
	engine_sig2 = (pthread_cond_t*)calloc(vcore_ne,sizeof(pthread_cond_t));
	engine_ready = (int*)calloc(vcore_ne,sizeof(int));
	engine_shutdown = (int*)calloc(vcore_ne,sizeof(int));
	engine_cmd_argp 
		= (struct cmdcall_arg**)calloc(vcore_ne,sizeof(struct cmdcall_arg*));
	engine_data 
		= (struct vcengine_data*)calloc(vcore_ne,sizeof(struct vcengine_data));
	engine_local_mem_base = (char**)calloc(vcore_ne,sizeof(char*));
	engine_local_mem_free = (char**)calloc(vcore_ne,sizeof(char*));
	engine_local_mem_sz = (size_t*)calloc(vcore_ne,sizeof(size_t));

	DEBUG(__FILE__,__LINE__,"alloc vc_stack_storage: calloc(%d,1)"
		,(VCORE_NC+1)*VCORE_STACK_SZ*vcore_ne);

	vc_stack_storage = (char*)calloc((VCORE_NC+1)*VCORE_STACK_SZ*vcore_ne,1);
	mprotect(vc_stack_storage,
		(VCORE_NC+1)*VCORE_STACK_SZ*vcore_ne,PROT_READ|PROT_WRITE|PROT_EXEC);

	ve_local_mem = (char*)calloc(VCORE_LOCAL_MEM_SZ*vcore_ne,1);


	pthread_attr_t td_attr;

	pthread_attr_init(&td_attr);
	pthread_attr_setdetachstate(&td_attr,PTHREAD_CREATE_JOINABLE);

//	for(i=0;i<VCORE_NE;i++) {

	cpuset_t mask;

	for(i=0;i<vcore_ne;i++) {

		pthread_mutex_init(&engine_mtx[i],0);
		pthread_cond_init(&engine_sig1[i],0);
		pthread_cond_init(&engine_sig2[i],0);

		engine_ready[i] = 0;
		engine_shutdown[i] = 0;
		engine_cmd_argp[i] = 0;
		engine_data[i].veid = i;
		engine_data[i].vc_runc = 0;

		pthread_create(&engine_td[i],&td_attr,vcengine,
			(void*)&engine_data[i]);

		CPU_ZERO(&mask);
		CPU_SET(i%ncore,&mask);
		if (pthread_setaffinity_np(engine_td[i],sizeof(cpuset_t),&mask)) {
			WARN(__FILE__,__LINE__,"vcengine: pthread_setaffinity_np failed");
		}

		pthread_yield();

		while(!engine_ready[i]) pthread_yield();

		DEBUG(__FILE__,__LINE__,"ready[%d] %d",i,engine_ready[i]);
	}

	pthread_attr_destroy(&td_attr);

	return(0);
}



void*
vcproc_cmd( int veid_base, int nve, struct cmdcall_arg* argp)
{
	int i,j;
	int e;

	int veid_end = veid_base + nve;

	DEBUG(__FILE__,__LINE__,"veid_base,nve %d,%d",veid_base,nve);

//	struct cmdcall_arg subcmd_argp[VCORE_NE];
	struct cmdcall_arg* subcmd_argp 
//		= (struct cmdcall_arg*)malloc(vcore_ne*sizeof(struct cmdcall_arg));
		= (struct cmdcall_arg*)malloc(nve*sizeof(struct cmdcall_arg));


//	for(i=0;i<VCORE_NE;i++) {
//	for(i=0;i<vcore_ne;i++) {
	for(e=veid_base;e<veid_end;e++) {

		/* must spin until engine is read to ensure valid local cache is set */
		/* XXX it would be better to spin once at initialization to take this
		/* XXX step out of the execution code -DAR */

		DEBUG(__FILE__,__LINE__,"ve[%d] vcproc_cmd-spin-until-ready",e);
		while(!engine_ready[e]) pthread_yield(); 

	}


	/* first apply correction to global ptrs */

	DEBUG(__FILE__,__LINE__,"cmdcall_x86_64:ndrange_kernel: fix global ptrs %p",
		argp->k.arg_kind);

	for(i=0;i<argp->k.krn->narg;i++) {

		DEBUG(__FILE__,__LINE__, "fix global ptrs %d",i);

		DEBUG(__FILE__,__LINE__,"arg_kind=%d", argp->k.arg_kind[i]);

   cl_context ctx;
   unsigned int ndev;
   cl_device_id* devices;
   unsigned int n;

		switch(argp->k.arg_kind[i]) {

			case CLARG_KIND_CONSTANT:
			case CLARG_KIND_GLOBAL:

				DEBUG(__FILE__,__LINE__, "argp->k.pr_arg_vec[%d]=%p",
					i,argp->k.pr_arg_vec[i]);

				DEBUG(__FILE__,__LINE__, "*cl_mem=%p",
					(*(cl_mem*)argp->k.pr_arg_vec[i]));

				ctx = (*(cl_mem*)argp->k.pr_arg_vec[i])->ctx;
				ndev = ctx->ndev;
				devices = ctx->devices;
				n = 0;

				/* XXX this is a hack, redesign devnum/devid issue -DAR */
//				while (n < ndev && devices[n] != devid) ++n;

				*(void**)argp->k.pr_arg_vec[i]
//					=(*(cl_mem*)argp->k.pr_arg_vec[i])->host_ptr;
					=(*(cl_mem*)argp->k.pr_arg_vec[i])->imp.res[n];

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

//	for(i=0;i<VCORE_NE;i++) {
//	for(i=0;i<vcore_ne;i++) {
	for(e=veid_base,i=0;e<veid_end;e++,i++) {

		memcpy(subcmd_argp+i,argp,sizeof(struct cmdcall_arg));

		__clone(subcmd_argp[i].k.pr_arg_vec,argp->k.pr_arg_vec,
			argp->k.narg,void*);

   	__clone(subcmd_argp[i].k.pr_arg_buf,argp->k.pr_arg_buf,
			argp->k.arg_buf_sz,void);

		DEBUG(__FILE__,__LINE__,"ve[%d] arg_buf %p %p",
			e,argp->k.pr_arg_buf,subcmd_argp[i].k.pr_arg_buf);

   	intptr_t offset
      	= (intptr_t)subcmd_argp[i].k.pr_arg_buf 
			- (intptr_t)argp->k.pr_arg_buf;

		DEBUG(__FILE__,__LINE__,"ve[%d] pr_arg_buf offset %p",e,offset);

   	for(j=0;j<subcmd_argp[i].k.narg;j++) 
			DEBUG(__FILE__,__LINE__,"ve[%d] arg_vec[%d] %p",
				e,j,subcmd_argp[i].k.pr_arg_vec[j]);

   	for(j=0;j<subcmd_argp[i].k.narg;j++) 
      	subcmd_argp[i].k.pr_arg_vec[j] += offset;

   	for(j=0;j<subcmd_argp[i].k.narg;j++) 
			DEBUG(__FILE__,__LINE__,"ve[%d] +offset arg_vec[%d] %p",
				e,j,subcmd_argp[i].k.pr_arg_vec[j]);

	}


	size_t sz;

//	for(e=0;e<VCORE_NE;e++) {
//	for(e=0;e<vcore_ne;e++) {
	for(e=veid_base,i=0;e<veid_end;e++,i++) {

DEBUG(__FILE__,__LINE__,"%p",&subcmd_argp[i].k.krn->narg);

		for(j=0;j<subcmd_argp[i].k.krn->narg;j++) {

			switch(subcmd_argp[i].k.arg_kind[j]) {

				case CLARG_KIND_LOCAL:

					sz = *(size_t*)subcmd_argp[i].k.pr_arg_vec[j];

					if (engine_local_mem_sz[e] < sz) {
						ERROR(__FILE__,__LINE__,"out of local mem");
						return((void*)-1);						
					}

					DEBUG(__FILE__,__LINE__,"ve[%d] argn %d alloc local mem %p %d",
						e,j,engine_local_mem_free[e],sz);

					*(void**)subcmd_argp[i].k.pr_arg_vec[j] 
						= (void*)engine_local_mem_free[e];
					
					engine_local_mem_free[e] += sz;
					engine_local_mem_sz[e] -= sz;

					DEBUG(__FILE__,__LINE__,"ve[%d] local mem sz free %d",
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


	/* now modify them to partition the work across engines */
	/* XXX this is the simplest distribution possible, elaborate later -DAR */	
	unsigned int d = argp->k.work_dim;
	unsigned int gwsd1 = argp->k.global_work_size[d-1];
	unsigned int lwsd1 = argp->k.local_work_size[d-1];

	DEBUG(__FILE__,__LINE__,"partitioning d=%d gwsz=%d lwsz=%d",d,gwsd1,lwsd1);

	DEBUG(__FILE__,__LINE__,"using nve=%d",nve);

	unsigned int ng = gwsd1/lwsd1;
//	unsigned int nge = ng/VCORE_NE;
//	unsigned int nge = ng/vcore_ne;
	unsigned int nge = ng/nve;
//	unsigned int nge0 = nge + ng%VCORE_NE;	
//	unsigned int nge0 = nge + ng%vcore_ne;	
	unsigned int nge0 = nge + ng%nve;	
	unsigned int inc0 = nge0*lwsd1;
	unsigned int inc = nge*lwsd1;

	DEBUG(__FILE__,__LINE__,"d gwsd1 lwsd1 ng: %d %d %d %d\n",d,gwsd1,lwsd1,ng);
	DEBUG(__FILE__,__LINE__,"partitioning: %d %d -> %d %d \n",nge,nge0,inc,inc0);

	unsigned int gwo = argp->k.global_work_offset[d-1];

	DEBUG(__FILE__,__LINE__,"initial gwo %d\n",gwo);

	subcmd_argp[0].k.global_work_offset[d-1] = gwo;
	gwo += subcmd_argp[0].k.global_work_size[d-1] = inc0;

//	for(i=1;i<VCORE_NE;i++) {
//	for(i=1;i<vcore_ne;i++) {
//	for(e=veid_base+1,i=0;e<veid_end;e++,i++) {
	for(e=veid_base+1,i=1;e<veid_end;e++,i++) {
		subcmd_argp[i].k.global_work_offset[d-1] = gwo;
		gwo += subcmd_argp[i].k.global_work_size[d-1] = inc;
	}

//	for(i=0;i<VCORE_NE;i++) 
//	for(i=0;i<vcore_ne;i++) 
	for(e=veid_base,i=0;e<veid_end;e++,i++) {
		DEBUG(__FILE__,__LINE__,"%d %d %d\n",
			subcmd_argp[i].k.global_work_offset[d-1],
			subcmd_argp[i].k.global_work_size[d-1],
			subcmd_argp[i].k.local_work_size[d-1]);
	}
	

//	for(i=0;i<VCORE_NE;i++) {
//	for(i=0;i<vcore_ne;i++) {
	for(e=veid_base,i=0;e<veid_end;e++,i++) {

		DEBUG(__FILE__,__LINE__,"ve[%d] vcproc_cmd-spin-until-ready",e);
		while(!engine_ready[e]) pthread_yield(); 

		DEBUG(__FILE__,__LINE__,"ve[%d] vcproc_cmd-attempt-lock",e);
		pthread_mutex_lock(&engine_mtx[e]);

		DEBUG(__FILE__,__LINE__,"ve[%d] arg0[2] %d",
			e,((int*)((subcmd_argp+i)->k.pr_arg_vec))[2]);

		engine_cmd_argp[e] = subcmd_argp+i;

		DEBUG(__FILE__,__LINE__,"ve[%d] vcproc_cmd-sig1",e);
		pthread_cond_signal(&engine_sig1[e]);

		DEBUG(__FILE__,__LINE__,"ve[%d] vcproc_cmd-unlock",e);
		pthread_mutex_unlock(&engine_mtx[e]);

	}


	pthread_yield(); 


//	for(e=0;e<VCORE_NE;e++) {
//	for(e=0;e<vcore_ne;e++) {
	for(e=veid_base;e<veid_end;e++) {

		DEBUG(__FILE__,__LINE__,"ve[%d] vcproc_cmd-attempt-lock",e);
		pthread_mutex_lock(&engine_mtx[e]);

		DEBUG(__FILE__,__LINE__,"ve[%d] vcproc_cmd-wait2",e);
		if (engine_cmd_argp[e]) pthread_cond_wait(&engine_sig2[e],&engine_mtx[e]);

		DEBUG(__FILE__,__LINE__,"ve[%d] vcproc_cmd complete",e);

		engine_local_mem_free[e] = engine_local_mem_base[e];
		engine_local_mem_sz[e] = VCORE_LOCAL_MEM_SZ;

		DEBUG(__FILE__,__LINE__,"ve[%d] vcproc_cmd-unlock",e);
		pthread_mutex_unlock(&engine_mtx[e]);

	}

}


/* This should never be needed since the vcengines should be idle at the
 * time the program is being taken down.  If this is not true, the user
 * is exiting with uncompleted events in the queue and this should be
 * dealt with elsewhere.  This is left here for testing purposes. -DAR 

static void __attribute__((destructor))
__vcproc_shutdown_dtor( void* p )
{
	DEBUG(__FILE__,__LINE__,"__vcproc_shutdown_dtor");

	int i;
	int retval;
	int count;
//	int remaining = VCORE_NE;
	int remaining = vcore_ne;

//	while (remaining) for(i=0;i<VCORE_NE;i++) {
	while (remaining) for(i=0;i<vcore_ne;i++) {

		DEBUG(__FILE__,__LINE__,"remaining %d",remaining);

		if (engine_data[i].vc_runc > 0) {
			fprintf(stderr,"very bad, vc_runc[%d] > 0",i);
			DEBUG(__FILE__,__LINE__,"pthread_cancel %d",i)
			pthread_cancel(engine_td[i]);
			DEBUG(__FILE__,__LINE__,"pthread_cancel succeded %d",i)
			engine_shutdown[i] = 3;
			--remaining;
		}

		DEBUG(__FILE__,__LINE__,"shutdown[%d] %d",i,engine_shutdown[i]);

		switch(engine_shutdown[i]) {

			case 0:
				pthread_mutex_lock(&engine_mtx[i]);
				engine_shutdown[i] = 1;
				pthread_cond_signal(&engine_sig[i]);
				pthread_mutex_unlock(&engine_mtx[i]);
				break;

			case 1:
				pthread_yield();
				break;

			case 2:

				pthread_join(engine_td[i],(void**)&retval);
				pthread_cond_destroy(&engine_sig[i]);
				pthread_mutex_destroy(&engine_mtx[i]);
				--remaining;
	
			case 3:
			default: break;

		}

		pthread_yield();

	}

	return;
}
*/


