/* sl_engine.c 
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
#include <setjmp.h>
#include <stdint.h>

#include "coprthr_device.h"
#include "coprthr_sched.h"
#include "coprthr_program.h"
#include "coprthr_mem.h"

#include "cmdcall.h"
#include "workp.h"
#include "cpuset_type.h"

#include "sl_engine.h"

#include <setjmp.h>

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif

static unsigned int nengines = 0;

static pthread_t* engine_td = 0;
static pthread_mutex_t* engine_mtx = 0;
static pthread_cond_t* engine_sig1 = 0;
static pthread_cond_t* engine_sig2 = 0;
static int* engine_ready = 0;
static int* engine_shutdown = 0;
static struct cmdcall_arg** engine_cmd_argp = 0;
static struct workp* common_engine_workp = 0;
static struct engine_data* engine_data = 0;
static char** engine_local_mem_base = 0;
static char** engine_local_mem_free = 0;
static size_t* engine_local_mem_sz = 0;

static char* engine_stack_storage = 0;
static char* engine_local_mem = 0;


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

int sl_engine_ready() 
{ return((engine_td)? 1:0); }

static void* sl_engine( void* p );

void* sl_engine_startup( void* p )
{
	int i;

	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);

	nengines = ncore;

	if (getenv("COPRTHR_MAX_NUM_ENGINES")) 
		nengines = min(nengines,atoi(getenv("COPRTHR_MAX_NUM_ENGINES")));

	printcl( CL_INFO "sl_engine_startup: nengines=%d",nengines);

	engine_td = (pthread_t*)calloc(nengines,sizeof(pthread_t));
	engine_mtx = (pthread_mutex_t*)calloc(nengines,sizeof(pthread_mutex_t));
	engine_sig1 = (pthread_cond_t*)calloc(nengines,sizeof(pthread_cond_t));
	engine_sig2 = (pthread_cond_t*)calloc(nengines,sizeof(pthread_cond_t));
	engine_ready = (int*)calloc(nengines,sizeof(int));
	engine_shutdown = (int*)calloc(nengines,sizeof(int));
	engine_cmd_argp 
		= (struct cmdcall_arg**)calloc(nengines,sizeof(struct cmdcall_arg*));
	engine_data 
		= (struct engine_data*)calloc(nengines,sizeof(struct engine_data));
	engine_local_mem_base = (char**)calloc(nengines,sizeof(char*));
	engine_local_mem_free = (char**)calloc(nengines,sizeof(char*));
	engine_local_mem_sz = (size_t*)calloc(nengines,sizeof(size_t));

	engine_stack_storage = (char*)malloc(nengines*(MAX_NUM_THR+1)*THR_STACK_SZ);
	memset(engine_stack_storage,73,nengines*(MAX_NUM_THR+1)*THR_STACK_SZ);
	mprotect(engine_stack_storage,(MAX_NUM_THR+1)*THR_STACK_SZ*nengines,
		PROT_READ|PROT_WRITE);

	engine_local_mem = (char*)malloc(nengines*BLK_LOCAL_MEM_SZ);
	memset(engine_local_mem,150,nengines*BLK_LOCAL_MEM_SZ);

	pthread_attr_t td_attr;

	pthread_attr_init(&td_attr);
	pthread_attr_setdetachstate(&td_attr,PTHREAD_CREATE_JOINABLE);

	cpu_set_t mask;

	for(i=0;i<nengines;i++) {

		pthread_mutex_init(&engine_mtx[i],0);
		pthread_cond_init(&engine_sig1[i],0);
		pthread_cond_init(&engine_sig2[i],0);

		engine_ready[i] = 0;
		engine_shutdown[i] = 0;
		engine_cmd_argp[i] = 0;
		engine_data[i].engid = i;
		engine_data[i].thr_runc = 0;

		printcl( CL_DEBUG "pthread_create %d",i); 

		pthread_create(&engine_td[i],&td_attr,sl_engine,(void*)&engine_data[i]);

		CPU_ZERO(&mask);
		CPU_SET(i%ncore,&mask);
		if (pthread_setaffinity_np(engine_td[i],sizeof(cpu_set_t),&mask)) {
			printcl( CL_WARNING "engine: pthread_setaffinity_np failed");
		}

		pthread_yield();

		printcl( CL_DEBUG "wait until engine ready %d",i); 

		while(!engine_ready[i]) pthread_yield();

		printcl( CL_DEBUG "ready[%d] %d",i,engine_ready[i]);
	}

	pthread_attr_destroy(&td_attr);

	return(0);
}


static void* sl_engine( void* p )
{

	int i;
	struct engine_data* edata = (struct engine_data*)p;
	int engid = edata->engid;

	jmp_buf engine_jbuf;
	jmp_buf thr_jbuf[MAX_NUM_THR];

	printcl( CL_DEBUG "engine-attempt-lock");
	pthread_mutex_lock(&engine_mtx[engid]);


	/* setup stacks and jbufs */

	size_t offset = engid*(MAX_NUM_THR+1)*THR_STACK_SZ + THR_STACK_SZ;
	char* thr_stack_base 
		= (char*) (((intptr_t)engine_stack_storage+offset)&THR_STACK_MASK);

	char* thr_stack[MAX_NUM_THR];

	printcl( CL_DEBUG "thr_stack_base %p\n",thr_stack_base);


	for(i=0;i<MAX_NUM_THR;i++) {
	
		int vcid = i + MAX_NUM_THR*engid;	

		thr_stack[i] = thr_stack_base + (i+1)*THR_STACK_SZ;

		struct thr_data* data = (struct thr_data*)(thr_stack[i]-THR_STACK_SZ);

		*data = (struct thr_data){vcid,&engine_jbuf,thr_jbuf+i,0,0};

		printcl( CL_DEBUG "sp %p data %p\n",thr_stack[i],data);
	}


	printcl( CL_DEBUG "[%d] engine_jbuf %p\n",engid,&engine_jbuf);

	for(i=0;i<MAX_NUM_THR;i++) 
		printcl( CL_DEBUG "thr_jbufp[%d] %p\n",i,thr_jbuf+i);


	engine_local_mem_free[engid] = engine_local_mem_base[engid] 
		= engine_local_mem + engid*BLK_LOCAL_MEM_SZ;

	engine_local_mem_sz[engid] = BLK_LOCAL_MEM_SZ;

	printcl( CL_DEBUG "engine_local_mem=%p",
		engine_local_mem);

	printcl( CL_DEBUG "engine_local_mem_free[%d]=%p",
		engid,engine_local_mem_free[engid]);

	printcl( CL_DEBUG "engine-set-ready");
	engine_ready[engid] = 1;


	int d,g;
	int nc,ng;
	int ii,gg;
	div_t qr;
	struct cmdcall_arg* cmd_argp;

	printcl( CL_DEBUG "[%d] ENTER BIG LOOP FOR VCENGINE\n",engid);

	do {

		printcl( CL_DEBUG "[%d] engine-wait1",engid);

		pthread_cond_wait(&engine_sig1[engid],&engine_mtx[engid]);

		if (cmd_argp = engine_cmd_argp[engid]) {

			/* XXX check to make sure this vcore has work */
			/* XXX if not, klaunch screwed up but we can catch this here */
			if ( engid >= common_engine_workp->n )
				goto skip;

			/* propagate info so that vcore() can execute specified kernel */

			printcl( CL_DEBUG "vcengine[%d]: krn %p",engid,cmd_argp->k.krn);
			edata->funcp = cmd_argp->k.ksyms->kthr;
			edata->callp = cmd_argp->k.ksyms->kcall;
			edata->pr_arg_buf = cmd_argp->k.pr_arg_buf;
			edata->pr_arg_off = cmd_argp->k.pr_arg_off;

		printcl( CL_DEBUG "workp_get_entry %p %d",common_engine_workp, engid);
			struct workp_entry* e = workp_get_entry( common_engine_workp, engid );

			report_workp_entry( CL_DEBUG, e );

         nc = 1;
         for(d = 0; d < e->ndr_dim; d++) nc *= e->ndr_ltdsz[d];

         printcl( CL_DEBUG "vcengine[%d]: nc=%d",engid,nc);

         for(i=0;i<nc;i++) {

         	struct thr_data* data 
					= (struct thr_data*)(thr_stack[i]-THR_STACK_SZ);

            data->next_jbufp = thr_jbuf + (i+1)%nc; /*****/

				data->we  = e;

            int ii = i;
            switch(e->ndr_dim) {
               case 3:
                  qr = div(ii,(e->ndr_ltdsz[1]*e->ndr_ltdsz[0]));
                  data->ltdidx[2] = qr.quot;
                  ii = qr.rem;
               case 2:
                  qr = div(ii,e->ndr_ltdsz[0]);
                  data->ltdidx[1] = qr.quot;
                  ii = qr.rem;
               case 1:
                  data->ltdidx[0] = ii;
               default: break;
            }

         }

         edata->thr_runc = 0;

			switch (e->ndr_dim) {

			case 3:
			{
			int blk0,blk1,blk2;

			printcl( CL_DEBUG "blk loop %d %d | %d %d | %d %d",
				e->ndp_blk_first[2],e->ndp_blk_end[2],
				e->ndp_blk_first[1],e->ndp_blk_end[1],
				e->ndp_blk_first[0],e->ndp_blk_end[0]);

			for(blk2=e->ndp_blk_first[2]; blk2 < e->ndp_blk_end[2]; blk2++) {
			for(blk1=e->ndp_blk_first[1]; blk1 < e->ndp_blk_end[1]; blk1++) {
			for(blk0=e->ndp_blk_first[0]; blk0 < e->ndp_blk_end[0]; blk0++) {

				for(i=0;i<nc;i++) {

					struct thr_data* data 
						=(struct thr_data*)(thr_stack[i]-THR_STACK_SZ);

					data->blkidx[2] = blk2;
					data->gtdidx[2] = blk2*e->ndr_ltdsz[2] + data->ltdidx[2];

					data->blkidx[1] = blk1;
					data->gtdidx[1] = blk1*e->ndr_ltdsz[1] + data->ltdidx[1];

					data->blkidx[0] = blk0;
					data->gtdidx[0] = blk0*e->ndr_ltdsz[0] + data->ltdidx[0];

				}


				printcl( CL_DEBUG "launching vcores (%d)",nc);

				char* sp;
				for(i=0;i<nc;i++) {
					if (!(setjmp(engine_jbuf))) {
						sp = thr_stack[i];
						printcl( CL_DEBUG "[%d] sp %p callp %p edata %p",
							i,sp,edata->callp,edata);
						__callsp(sp,edata->callp,edata);
					}
				}

				for(i=0;i<nc;i++) {

					if (!(setjmp(engine_jbuf))) longjmp(thr_jbuf[i],i+1);
				} 

				if (edata->thr_runc) {
					printcl( CL_DEBUG "vcengine[%d]: unterminated vcore",engid);
					exit(-1);
				} else {
					printcl( CL_DEBUG 
						"vcengine[%d]: all vcores completed",engid);
				} 

			} } }

			}
			break;

			case 2:
			{
			int blk0,blk1;

			printcl( CL_DEBUG "blk loop %d %d | %d %d",
				e->ndp_blk_first[1],e->ndp_blk_end[1],
				e->ndp_blk_first[0],e->ndp_blk_end[0]);

			for(blk1=e->ndp_blk_first[1]; blk1 < e->ndp_blk_end[1]; blk1++) {
			for(blk0=e->ndp_blk_first[0]; blk0 < e->ndp_blk_end[0]; blk0++) {

				for(i=0;i<nc;i++) {

					struct thr_data* data 
						=(struct thr_data*)(thr_stack[i]-THR_STACK_SZ);

					data->blkidx[1] = blk1;
					data->gtdidx[1] = blk1*e->ndr_ltdsz[1] + data->ltdidx[1];

					data->blkidx[0] = blk0;
					data->gtdidx[0] = blk0*e->ndr_ltdsz[0] + data->ltdidx[0];

				}


				printcl( CL_DEBUG "launching vcores (%d)",nc);

				char* sp;
				for(i=0;i<nc;i++) {
					if (!(setjmp(engine_jbuf))) {
						sp = thr_stack[i];
						printcl( CL_DEBUG "[%d] sp %p callp %p edata %p",
							i,sp,edata->callp,edata);
						__callsp(sp,edata->callp,edata);
					}
				}

				for(i=0;i<nc;i++) {

					if (!(setjmp(engine_jbuf))) longjmp(thr_jbuf[i],i+1);
				} 

				if (edata->thr_runc) {
					printcl( CL_DEBUG "vcengine[%d]: unterminated vcore",engid);
					exit(-1);
				} else {
					printcl( CL_DEBUG 
						"vcengine[%d]: all vcores completed",engid);
				} 

			} }

			}
			break;

			case 1:

			{
			int blk0;

			printcl( CL_DEBUG "blk loop %d %d",
				e->ndp_blk_first[0],e->ndp_blk_end[0]);

			for(blk0=e->ndp_blk_first[0]; blk0 < e->ndp_blk_end[0]; blk0++) {

				for(i=0;i<nc;i++) {

					struct thr_data* data 
						=(struct thr_data*)(thr_stack[i]-THR_STACK_SZ);

					data->blkidx[0] = blk0;
					data->gtdidx[0] = blk0*e->ndr_ltdsz[0] + data->ltdidx[0];

					printcl( CL_DEBUG "core data %p this jbufp %p",
						data,data->this_jbufp+i);
				}


				printcl( CL_DEBUG "launching vcores (%d)",nc);

				char* sp;
				for(i=0;i<nc;i++) {
					if (!(setjmp(engine_jbuf))) {
						sp = thr_stack[i];
						printcl( CL_DEBUG "[%d] sp %p callp %p edata %p",
							i,sp,edata->callp,edata);
						__callsp(sp,edata->callp,edata);
					}
				}

				for(i=0;i<nc;i++) {

					printcl( CL_DEBUG "second loop %d",i);
					if (!(setjmp(engine_jbuf))) longjmp(thr_jbuf[i],i+1);
				} 

				if (edata->thr_runc) {
					printcl( CL_DEBUG "vcengine[%d]: unterminated vcore",engid);
					exit(-1);
				} else {
					printcl( CL_DEBUG 
						"vcengine[%d]: all vcores completed",engid);
				} 

			}

			}
			break;

			}

skip:
			engine_cmd_argp[engid] = 0;
			
		} 

		printcl( CL_DEBUG "engine-sig2");
		pthread_cond_signal(&engine_sig2[engid]);

	} while(!engine_shutdown[engid]);

	engine_shutdown[engid] = 2;

	pthread_mutex_unlock(&engine_mtx[engid]);
	printcl( CL_DEBUG "engine-unlock");

}


static void* sl_engine_klaunch_vec( int engid_base, int ne, struct workp* wp,
	struct cmdcall_arg* argp );

void* sl_engine_klaunch( int engid_base, int ne, struct workp* wp,
	struct cmdcall_arg* argp )
{
	int i,j;
	int e;

	/* XXX here we are chaining the multiple/vec_argp case */
	if (wp->flags == 1)
		return sl_engine_klaunch_vec(engid_base,ne,wp,argp);

	int engid_end = engid_base + ne;

	printcl( CL_DEBUG "engid_base,ne %d,%d",engid_base,ne);

	common_engine_workp = wp;

	struct cmdcall_arg* subcmd_argp 
		= (struct cmdcall_arg*)malloc(ne*sizeof(struct cmdcall_arg));


	for(e=engid_base;e<engid_end;e++) {

		/* must spin until engine is read to ensure valid local cache is set */
		/* XXX it would be better to spin once at initialization to take this
		/* XXX step out of the execution code -DAR */

		printcl( CL_DEBUG "ve[%d] klaunch-spin-until-ready",e);
		while(!engine_ready[e]) pthread_yield(); 

	}


	/* first apply correction to global ptrs */

	printcl( CL_DEBUG "cmdcall_x86_64:ndrange_kernel: fix global ptrs %p",
		argp->k.arg_kind);

	for(i=0;i<argp->k.krn->prg1->knarg[argp->k.krn->knum];i++) {

		printcl( CL_DEBUG  "fix global ptrs %d",i);

		printcl( CL_DEBUG "arg_kind=%d", argp->k.arg_kind[i]);

   unsigned int ndev;
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

				struct coprthr1_mem* mem1 = *(struct coprthr1_mem**)p;

				printcl( CL_DEBUG  "mem1=%p",mem1);

				*(void**)p =mem1->res;

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

	printcl( CL_DEBUG "copy *argp for each engine and alloc local mem");

	for(e=engid_base,i=0;e<engid_end;e++,i++) {

		memcpy(subcmd_argp+i,argp,sizeof(struct cmdcall_arg));

		__clone(subcmd_argp[i].k.pr_arg_off,argp->k.pr_arg_off,
			argp->k.narg,uint32_t);

   	__clone(subcmd_argp[i].k.pr_arg_buf,argp->k.pr_arg_buf,
			argp->k.arg_buf_sz,void);

		printcl( CL_DEBUG "ve[%d] arg_buf %p %p",
			e,argp->k.pr_arg_buf,subcmd_argp[i].k.pr_arg_buf);

   	for(j=0;j<subcmd_argp[i].k.narg;j++) 
			printcl( CL_DEBUG "ve[%d] arg_off[%d] %p",
				e,j,subcmd_argp[i].k.pr_arg_off[j]);

	}

	size_t sz;

	for(e=engid_base,i=0;e<engid_end;e++,i++) {

		for(j=0;j<subcmd_argp[i].k.krn->prg1->knarg[subcmd_argp[i].k.krn->knum];j++) {

			void* p = (intptr_t)subcmd_argp[i].k.pr_arg_buf
				+ subcmd_argp[i].k.pr_arg_off[j];

			switch(subcmd_argp[i].k.arg_kind[j]) {

				case CLARG_KIND_LOCAL:

					sz = *(size_t*)p;

					if (engine_local_mem_sz[e] < sz) {
						printcl( CL_ERR "out of local mem");
						return((void*)-1);						
					}

					printcl( CL_DEBUG "ve[%d] argn %d alloc local mem %p %d",
						e,j,engine_local_mem_free[e],sz);

					*(void**)p = (void*)engine_local_mem_free[e];
					
					engine_local_mem_free[e] += sz;
					engine_local_mem_sz[e] -= sz;

					printcl( CL_DEBUG "ve[%d] local mem sz free %d",
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


	for(e=engid_base,i=0;e<engid_end;e++,i++) {

		printcl( CL_DEBUG "ve[%d] klaunch-spin-until-ready",e);
		while(!engine_ready[e]) pthread_yield(); 

		printcl( CL_DEBUG "ve[%d] klaunch-attempt-lock",e);
		pthread_mutex_lock(&engine_mtx[e]);

		engine_cmd_argp[e] = subcmd_argp+i;

		printcl( CL_DEBUG "ve[%d] klaunch-sig1",e);
		pthread_cond_signal(&engine_sig1[e]);

		printcl( CL_DEBUG "ve[%d] klaunch-unlock",e);
		pthread_mutex_unlock(&engine_mtx[e]);

	}

	pthread_yield(); 

	for(e=engid_base;e<engid_end;e++) {

		printcl( CL_DEBUG "ve[%d] klaunch-attempt-lock",e);
		pthread_mutex_lock(&engine_mtx[e]);

		printcl( CL_DEBUG "ve[%d] klaunch-wait2",e);
		if (engine_cmd_argp[e]) pthread_cond_wait(&engine_sig2[e],&engine_mtx[e]);

		printcl( CL_DEBUG "ve[%d] klaunch complete",e);

		engine_local_mem_free[e] = engine_local_mem_base[e];
		engine_local_mem_sz[e] = BLK_LOCAL_MEM_SZ;

		printcl( CL_DEBUG "ve[%d] klaunch-unlock",e);
		pthread_mutex_unlock(&engine_mtx[e]);

	}

	common_engine_workp = wp;

}

static void* sl_engine_klaunch_vec( int engid_base, int ne, struct workp* wp,
	struct cmdcall_arg* argp )
{
	int i,j,k;
	int e;

	int engid_end = engid_base + ne;

	printcl( CL_DEBUG "engid_base,ne %d,%d",engid_base,ne);

	common_engine_workp = wp;

//	struct cmdcall_arg* subcmd_argp 
//		= (struct cmdcall_arg*)malloc(ne*sizeof(struct cmdcall_arg));

	int nargp = wp->n; /* XXX use this for now */

	for(e=engid_base;e<engid_end;e++) {

		/* must spin until engine is read to ensure valid local cache is set */
		/* XXX it would be better to spin once at initialization to take this
		/* XXX step out of the execution code -DAR */

		printcl( CL_DEBUG "ve[%d] klaunch-spin-until-ready",e);
		while(!engine_ready[e]) pthread_yield(); 

	}


	/* first apply correction to global ptrs */

//// LOOP OVER VEC_ARGP
//// COULD USE THE SAME LOOP CONSTRUCT AS BELOW
	for(k=0;k<nargp;k++) { 

	printcl( CL_DEBUG "cmdcall_x86_64:ndrange_kernel: fix global ptrs %p",
		argp[k].k.arg_kind);

	for(i=0;i<argp[k].k.krn->prg1->knarg[argp[k].k.krn->knum];i++) {

		printcl( CL_DEBUG  "fix global ptrs %d",i);

		printcl( CL_DEBUG "arg_kind=%d", argp[k].k.arg_kind[i]);

   	unsigned int ndev;
   	unsigned int n;

		void* p = (void*)(argp[k].k.pr_arg_buf + argp[k].k.pr_arg_off[i]);

		printcl( CL_DEBUG "XXX %d %p %p", 
			argp[k].k.pr_arg_off[i],
			argp[k].k.pr_arg_buf,
			p);

		switch(argp[k].k.arg_kind[i]) {

			case CLARG_KIND_CONSTANT:
			case CLARG_KIND_GLOBAL:

				{

				printcl( CL_DEBUG  "argp->k.pr_arg_off[%d]=%p",
					i,argp[k].k.pr_arg_off[i]);

				struct coprthr1_mem* mem1 = *(struct coprthr1_mem**)p;

				printcl( CL_DEBUG  "mem1=%p",mem1);

				*(void**)p =mem1->res;

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

	}




	/* make copies of *argp for each engine and allocate local mem */

//// NOT NECESSARY FOR VEC_ARGP
/*
	printcl( CL_DEBUG "copy *argp for each engine and alloc local mem");

	for(e=engid_base,i=0;e<engid_end;e++,i++) {

		memcpy(subcmd_argp+i,argp,sizeof(struct cmdcall_arg));

		__clone(subcmd_argp[i].k.pr_arg_off,argp->k.pr_arg_off,
			argp->k.narg,uint32_t);

   	__clone(subcmd_argp[i].k.pr_arg_buf,argp->k.pr_arg_buf,
			argp->k.arg_buf_sz,void);

		printcl( CL_DEBUG "ve[%d] arg_buf %p %p",
			e,argp->k.pr_arg_buf,subcmd_argp[i].k.pr_arg_buf);

   	for(j=0;j<subcmd_argp[i].k.narg;j++) 
			printcl( CL_DEBUG "ve[%d] arg_off[%d] %p",
				e,j,subcmd_argp[i].k.pr_arg_off[j]);

	}
*/


	size_t sz;

	for(e=engid_base,i=0;e<engid_end;e++,i++) {

		printcl( CL_DEBUG "knum %d (%d)",argp[i].k.krn->knum,i);
		printcl( CL_DEBUG "knarg %d (%d)",argp[i].k.krn->prg1->knarg[argp[i].k.krn->knum],i);

		for(j=0;j<argp[i].k.krn->prg1->knarg[argp[i].k.krn->knum];j++) {

			void* p = (intptr_t)argp[i].k.pr_arg_buf
				+ argp[i].k.pr_arg_off[j];

			switch(argp[i].k.arg_kind[j]) {

				case CLARG_KIND_LOCAL:

					sz = *(size_t*)p;

					if (engine_local_mem_sz[e] < sz) {
						printcl( CL_ERR "out of local mem");
						return((void*)-1);						
					}

					printcl( CL_DEBUG "ve[%d] argn %d alloc local mem %p %d",
						e,j,engine_local_mem_free[e],sz);

					*(void**)p = (void*)engine_local_mem_free[e];
					
					engine_local_mem_free[e] += sz;
					engine_local_mem_sz[e] -= sz;

					printcl( CL_DEBUG "ve[%d] local mem sz free %d",
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


	for(e=engid_base,i=0;e<engid_end;e++,i++) {

		printcl( CL_DEBUG "ve[%d] klaunch-spin-until-ready",e);
		while(!engine_ready[e]) pthread_yield(); 

		printcl( CL_DEBUG "ve[%d] klaunch-attempt-lock",e);
		pthread_mutex_lock(&engine_mtx[e]);

		engine_cmd_argp[e] = argp+i;

		printcl( CL_DEBUG "ve[%d] klaunch-sig1",e);
		pthread_cond_signal(&engine_sig1[e]);

		printcl( CL_DEBUG "ve[%d] klaunch-unlock",e);
		pthread_mutex_unlock(&engine_mtx[e]);

	}

	pthread_yield(); 

	for(e=engid_base;e<engid_end;e++) {

		printcl( CL_DEBUG "ve[%d] klaunch-attempt-lock",e);
		pthread_mutex_lock(&engine_mtx[e]);

		printcl( CL_DEBUG "ve[%d] klaunch-wait2",e);
		if (engine_cmd_argp[e]) pthread_cond_wait(&engine_sig2[e],&engine_mtx[e]);

		printcl( CL_DEBUG "ve[%d] klaunch complete",e);

		engine_local_mem_free[e] = engine_local_mem_base[e];
		engine_local_mem_sz[e] = BLK_LOCAL_MEM_SZ;

		printcl( CL_DEBUG "ve[%d] klaunch-unlock",e);
		pthread_mutex_unlock(&engine_mtx[e]);

	}

//	common_engine_workp = wp;

}

