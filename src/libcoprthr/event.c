/* event.c
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

#include <CL/cl.h>

#include "xcl_structs.h"
#include "event.h"
#include "command_queue.h"


void __do_release_event(cl_event ev) 
{
	/* XXX may need to check state of event for controlled release -DAR */

	/* XXX this assumes the ev is on cmds_complete, check this first! -DAR */

	DEBUG(__FILE__,__LINE__,"__do_release_event: attempt lock cmdq");
	__lock_cmdq(ev->cmdq);
	DEBUG(__FILE__,__LINE__,"__do_release_event: locked cmdq");
	DEBUG(__FILE__,__LINE__,"__do_release_event: remove ev from cmdq");
	DEBUG(__FILE__,__LINE__,"__do_release_event: cmdq %p\n",ev->cmdq);
	TAILQ_REMOVE(&ev->cmdq->imp.cmds_complete,ev,imp.cmds);
	DEBUG(__FILE__,__LINE__,"__do_release_event: removed from cmdq");
	__unlock_cmdq(ev->cmdq);
	ev->cmdq = 0;
	DEBUG(__FILE__,__LINE__,"__do_release_event: success");
}



#define __copy3(dst,src) do { \
	(dst)[0]=(src)[0]; (dst)[1]=(src)[1]; (dst)[2]=(src)[2]; \
	} while(0)


/*
 * set cmd
 */

void __do_set_cmd_read_buffer( 
	cl_event ev, 
	cl_mem src, size_t src_offset, size_t len, 
	void* dst
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->m.dst = dst;
	ev->imp.cmd_argp->m.src = (void*)src;
	ev->imp.cmd_argp->m.src_offset = src_offset;
	ev->imp.cmd_argp->m.len = len;
}


void __do_set_cmd_write_buffer( 
	cl_event ev, 
	cl_mem dst, size_t dst_offset, size_t len, 
	const void* src
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->m.dst = (void*)dst;
	ev->imp.cmd_argp->m.src = (void*)src;
	ev->imp.cmd_argp->m.dst_offset = dst_offset;
	ev->imp.cmd_argp->m.len = len;
}


void __do_set_cmd_copy_buffer( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	size_t src_offset, size_t dst_offset, size_t len 
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->m.dst = (void*)dst;
	ev->imp.cmd_argp->m.src = (void*)src;
	ev->imp.cmd_argp->m.dst_offset = dst_offset;
	ev->imp.cmd_argp->m.src_offset = src_offset;
	ev->imp.cmd_argp->m.len = len;
}


void __do_set_cmd_read_image( 
	cl_event ev, 
	cl_mem src, 
	const size_t* src_origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, 
	void* dst
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->m.dst = dst;
	ev->imp.cmd_argp->m.src = (void*)src;

	if (src_origin) __copy3(ev->imp.cmd_argp->m.src_origin,src_origin); 
	else ERROR(__FILE__,__LINE__,"fix this");

	if (region) __copy3(ev->imp.cmd_argp->m.region,region);
	else ERROR(__FILE__,__LINE__,"fix this");

	ev->imp.cmd_argp->m.row_pitch = (void*)row_pitch;
	ev->imp.cmd_argp->m.slice_pitch = (void*)slice_pitch;
}


void __do_set_cmd_write_image( 
	cl_event ev, 
	cl_mem dst, 
	const size_t* dst_origin, const size_t* region, 
	size_t row_pitch, size_t slice_pitch, 
	const void* src
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->m.dst = dst;
	ev->imp.cmd_argp->m.src = (void*)src;
	__copy3(ev->imp.cmd_argp->m.dst_origin,dst_origin);
	__copy3(ev->imp.cmd_argp->m.region,region);
	ev->imp.cmd_argp->m.row_pitch = (void*)row_pitch;
	ev->imp.cmd_argp->m.slice_pitch = (void*)slice_pitch;
}


void __do_set_cmd_copy_image( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	const size_t* src_origin, 
	const size_t* dst_origin, const size_t* region
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->m.dst = (void*)dst;
	ev->imp.cmd_argp->m.src = (void*)src;
	__copy3(ev->imp.cmd_argp->m.dst_origin,dst_origin);
	__copy3(ev->imp.cmd_argp->m.src_origin,src_origin);
	__copy3(ev->imp.cmd_argp->m.region,region);
}


void __do_set_cmd_copy_image_to_buffer( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	const size_t* src_origin, const size_t* region, 
	size_t dst_offset
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->m.dst = (void*)dst;
	ev->imp.cmd_argp->m.src = (void*)src;
	ev->imp.cmd_argp->m.dst_offset = dst_offset;
	__copy3(ev->imp.cmd_argp->m.src_origin,src_origin);
	__copy3(ev->imp.cmd_argp->m.region,region);
}


void __do_set_cmd_copy_buffer_to_image( 
	cl_event ev, 
	cl_mem src, cl_mem dst, 
	size_t src_offset, 
	const size_t* dst_origin, const size_t* region
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->m.dst = (void*)dst;
	ev->imp.cmd_argp->m.src = (void*)src;
	ev->imp.cmd_argp->m.src_offset = src_offset;
	__copy3(ev->imp.cmd_argp->m.dst_origin,dst_origin);
	__copy3(ev->imp.cmd_argp->m.region,region);
}


void __do_set_cmd_map_buffer( 
	cl_event ev, 
	cl_mem membuf,
	cl_map_flags flags, size_t offset, size_t len,
	void* p
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->flags = flags;
	ev->imp.cmd_argp->m.dst = (void*)p;
	ev->imp.cmd_argp->m.src = (void*)membuf;
	ev->imp.cmd_argp->m.src_offset = offset;
	ev->imp.cmd_argp->m.len = len;
}


void __do_set_cmd_map_image( 
	cl_event ev, 
	cl_mem image,
	cl_map_flags flags, const size_t* origin, const size_t* region,
	size_t* row_pitch, size_t* slice_pitch,
	void* p
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->flags = flags;
	ev->imp.cmd_argp->m.dst = (void*)p;
	ev->imp.cmd_argp->m.src = (void*)image;
	__copy3(ev->imp.cmd_argp->m.src_origin,origin);
	__copy3(ev->imp.cmd_argp->m.region,region);
	ev->imp.cmd_argp->m.row_pitch = (void*)row_pitch;
	ev->imp.cmd_argp->m.slice_pitch = (void*)slice_pitch;
}


void __do_set_cmd_unmap_memobj( 
	cl_event ev, 
	cl_mem memobj, void* p
)
{
	ev->imp.cmd_argp = (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(ev->imp.cmd_argp);

	ev->imp.cmd_argp->m.dst = (void*)p;
	ev->imp.cmd_argp->m.src = (void*)memobj;
}


void __do_set_cmd_ndrange_kernel(
	cl_command_queue cmdq,
	cl_event ev,
	cl_kernel krn,
	cl_uint work_dim,
	const size_t* global_work_offset,
	const size_t* global_work_size,
	const size_t* local_work_size
)
{
	int i;

	struct cmdcall_arg* argp = ev->imp.cmd_argp 
		= (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(argp);

	argp->flags = CMDCALL_ARG_K;

	argp->k.krn = krn;

	DEBUG(__FILE__,__LINE__,"ndev = %d",krn->prg->ndev);

	int devnum;
	for(devnum=0;devnum<krn->prg->ndev;devnum++) 
		if (cmdq->devid == krn->prg->devices[devnum]) break;

	if (devnum == krn->prg->ndev) {
		ERROR(__FILE__,__LINE__,"internal error");
		exit(-1);
	}

	DEBUG(__FILE__,__LINE__,"devnum = %d",devnum);

//	argp->k.ksym = ((void***)krn->imp.v_ksym)[devnum][krn->imp.knum];
//	argp->k.kcall = ((void***)krn->imp.v_kcall)[devnum][krn->imp.knum];
//	argp->k.ksym = krn->imp.v_ksyms[devnum][krn->imp.knum].kthr;
//	argp->k.kcall = krn->imp.v_ksyms[devnum][krn->imp.knum].kcall;
	argp->k.ksyms = &krn->imp.v_ksyms[devnum][krn->imp.knum];
	argp->k.narg = krn->narg;
	argp->k.arg_buf_sz = krn->imp.arg_buf_sz;

DEBUG(__FILE__,__LINE__,"setting argp->k.arg_kind %p",krn->imp.arg_kind);
	
	argp->k.arg_kind = krn->imp.arg_kind;
	argp->k.arg_sz = krn->imp.arg_sz;


		
	/* XXX simplest to copy args, later test copy-on-set -DAR */

	__clone(argp->k.pr_arg_off,krn->imp.arg_off,krn->narg,uint32_t);
	__clone(argp->k.pr_arg_buf,krn->imp.arg_buf,krn->imp.arg_buf_sz,void);

	DEBUG(__FILE__,__LINE__,"arg_buf %p,%p",krn->imp.arg_buf,argp->k.pr_arg_buf);	
	intptr_t offset 
		= (intptr_t)argp->k.pr_arg_buf - (intptr_t)krn->imp.arg_buf;

	ev->imp.cmd_argp->k.work_dim = work_dim;

	for(i=0;i<work_dim;i++) {
		if (global_work_offset)
			ev->imp.cmd_argp->k.global_work_offset[i] = global_work_offset[i];
		ev->imp.cmd_argp->k.global_work_size[i] = global_work_size[i];
		ev->imp.cmd_argp->k.local_work_size[i] = local_work_size[i];
	}

}


void __do_set_cmd_task( cl_event ev, cl_kernel krn)
{
	int i;

	struct cmdcall_arg* argp = ev->imp.cmd_argp 
		= (struct cmdcall_arg*)malloc(sizeof(struct cmdcall_arg));

	__init_cmdcall_arg(argp);

	argp->flags = CMDCALL_ARG_K;

	argp->k.krn = krn; /* XXX depreacted, remove -DAR */

	int devnum;
	for(devnum=0;devnum<krn->prg->ndev;devnum++) 
		if (ev->cmdq->devid == krn->prg->devices[devnum]) break;

	if (devnum == krn->prg->ndev) {
		ERROR(__FILE__,__LINE__,"internal error");
		exit(-1);
	}

//	argp->k.ksym = ((void***)krn->imp.v_ksym)[devnum][krn->imp.knum];
//	argp->k.kcall = ((void***)krn->imp.v_kcall)[devnum][krn->imp.knum];
	argp->k.ksym = krn->imp.v_ksyms[devnum][krn->imp.knum].kthr;
	argp->k.kcall = krn->imp.v_ksyms[devnum][krn->imp.knum].kcall;
	argp->k.narg = krn->narg;

DEBUG(__FILE__,__LINE__,"setting argp->k.arg_kind %p",krn->imp.arg_kind);
	
	argp->k.arg_kind = krn->imp.arg_kind;
	argp->k.arg_sz = krn->imp.arg_sz;

	/* XXX simplest to copy args, later test copy-on-set -DAR */

	__clone(argp->k.pr_arg_off,krn->imp.arg_off,krn->narg,uint32_t);
	__clone(argp->k.pr_arg_buf,krn->imp.arg_buf,krn->imp.arg_buf_sz,void);

	ev->imp.cmd_argp->k.work_dim = 0;

}


/*
 * wait
 */

void __do_wait_for_events( cl_uint nev, const cl_event* evlist)
{
	int i;
	cl_event ev;

	for(i=0;i<nev;i++) {

		ev = evlist[i];

		DEBUG(__FILE__,__LINE__,"wait for event %p %d\n",ev,ev->cmd_stat);

		__lock_event(ev);

		while (ev->cmd_stat != CL_COMPLETE) {
			DEBUG(__FILE__,__LINE__,"__do_wait_for_events: wait-sleep\n");
			__wait_event(ev);
			DEBUG(__FILE__,__LINE__,"__do_wait_for_events: wait-wake\n");
		}
 
		DEBUG(__FILE__,__LINE__,"event %p complete\n",ev);

		__unlock_event(evlist[i]);

	}

}

