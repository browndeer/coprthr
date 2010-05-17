/* clmalloc.c
 *
 * Copyright (c) 2009 Brown Deer Technology, LLC.  All Rights Reserved.
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/queue.h>

#include "stdcl.h"
#include "util.h"
#include "clmalloc.h"
#include "clsched.h"

//#include "sigsegv.h"
//#include "mmaputil.h"

//#ifndef  HAVE_SIGSEGV_RECOVERY
//#error HAVE_SIGSEGV_RECOVERY not defined, clmalloc will not work properly
//#endif



#define __MEMD_F_R			0x01
#define __MEMD_F_W			0x02
#define __MEMD_F_RW 			(__MEMD_F_R|__MEMD_F_W)
#define __MEMD_F_ATTACHED	0x04
#define __MEMD_F_LOCKED		0x08
#define __MEMD_F_DIRTY		0x10



/* XXX hack to work around the problem with clCreateCommandQueue -DAR */
//static inline void __cmdq__(CONTEXT* cp, cl_uint n)
//{
//   if (!cp->cmdq[n]) 
//      cp->cmdq[n] = clCreateCommandQueue(cp->ctx,cp->dev[n],0,0);
//}



inline int
__test_memd_magic(void* ptr) 
{
	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;
	if (memd->magic == CLMEM_MAGIC) return(1);
	return(0);
}

void* clmalloc(CONTEXT* cp, size_t size, int flags)
{

//	size *=2;

	DEBUG(__FILE__,__LINE__,"clmalloc: size=%d flag=%d",size,flags);

	int err;
	intptr_t ptri = (intptr_t)malloc(size+sizeof(struct _memd_struct));
	intptr_t ptr = ptri+sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	DEBUG(__FILE__,__LINE__,"clmalloc: ptri=%p ptr=%p memd=%p",ptri,ptr,memd);

	DEBUG(__FILE__,__LINE__,"clmalloc: sizeof struct _memd_struct %d",
		sizeof(struct _memd_struct));

	if ((flags&CL_MEM_READ_ONLY) || (flags&CL_MEM_WRITE_ONLY)) {
		WARN(__FILE__,__LINE__,
			"clmalloc: CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY unsupported");
	} //// XXX CL_MEM_READ_WRITE implied -DAR

	memd->flags = __MEMD_F_RW;
	memd->sz = size;

	if (flags&CL_MEM_UNATTACHED) {
	
		memd->clbuf = (cl_mem)0;

	} else {

		memd->clbuf = clCreateBuffer(
//      	cp->ctx,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,
      	cp->ctx,CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
      	size,(void*)ptr,&err
   	);

		DEBUG(__FILE__,__LINE__,"clmalloc: clCreateBuffer clbuf=%p",memd->clbuf);

		DEBUG(__FILE__,__LINE__,"clmalloc: err from clCreateBuffer %d",err);

		memd->flags |= __MEMD_F_ATTACHED;

		LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

	}

	return((void*)ptr);
	
}


void clfree( void* ptr )
{
	int err;

	DEBUG(__FILE__,__LINE__,"clfree: ptr=%p\n",ptr);

	if (!ptr) { WARN(__FILE__,__LINE__,"clfree: null ptr"); return; }

//	if (!assert_cldev_valid(dev)) return (0);

	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	if (memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) {

		err = clReleaseMemObject(memd->clbuf);

		LIST_REMOVE(memd, memd_list);

	}

	free((void*)ptri);	
			
}


int clmattach( CONTEXT* cp, void* ptr )
{
	int err;

	if (!__test_memd_magic(ptr)) {

		ERROR(__FILE__,__LINE__,"clmattach: invalid ptr");

		return(EFAULT);

	}
	
	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	if ( (!memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) 
		|| (memd->clbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {

		ERROR(__FILE__,__LINE__,"clmdetach: memd corrupt");

		return(EFAULT);

	}

	if (memd->flags&__MEMD_F_ATTACHED) return(EINVAL);

	memd->clbuf = clCreateBuffer(
     	cp->ctx,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,
     	memd->sz,(void*)ptr,&err
  	);

	DEBUG(__FILE__,__LINE__,"clmattach: clCreateBuffer clbuf=%p",memd->clbuf);

	DEBUG(__FILE__,__LINE__,"clmattach: err from clCreateBuffer %d",err);

	memd->flags |= __MEMD_F_ATTACHED;

	LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

}


int clmdetach( void* ptr )
{
	int err; 

	if (!__test_memd_magic(ptr)) {

		ERROR(__FILE__,__LINE__,"clmdetach: invalid ptr");

		return(EFAULT);

	}

	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	if ( (!memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) 
		|| (memd->clbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {

		ERROR(__FILE__,__LINE__,"clmdetach: memd corrupt");

		return(EFAULT);

	}

	if (!(memd->flags&__MEMD_F_ATTACHED)) return(EINVAL);

	err = clReleaseMemObject(memd->clbuf);

	LIST_REMOVE(memd, memd_list);

	memd->flags &= ~(cl_uint)__MEMD_F_ATTACHED;

	return(0);
}


int clmctl( void* ptr, int op, int arg )
{
	int err; 

	if (!__test_memd_magic(ptr)) {

		ERROR(__FILE__,__LINE__,"clmdetach: invalid ptr");

		return(EFAULT);

	}

	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	if ( (!memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) 
		|| (memd->clbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {

		ERROR(__FILE__,__LINE__,"clmdetach: memd corrupt");

		return(EFAULT);

	}

	switch (op) {

		case CL_MCTL_GET_STATUS:
			return(memd->flags);

		case CL_MCTL_GET_DEVNUM:
			return(memd->devnum);

		case CL_MCTL_SET_DEVNUM:
			memd->devnum = arg;
			return(0);

		case CL_MCTL_MARK_CLEAN:
			memd->flags &= ~(cl_uint)__MEMD_F_DIRTY;
			return(0);

		default:
			return(0);

	}

}


cl_event clmsync(CONTEXT* cp, unsigned int devnum, void* ptr, int flags )
{
	int err;
	cl_event ev;
	if (!ptr) return((cl_event)0);
	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

//	__cmdq__(cp,0); // XXX this is a hack -DAR

//	if (flags&CL_MEM_WRITE) {
	if (flags&CL_MEM_WRITE || flags&CL_MEM_DEVICE) {
		err = clEnqueueWriteBuffer(
			cp->cmdq[devnum],memd->clbuf,CL_FALSE,0,memd->sz,ptr,0,0,&ev
   	);
		DEBUG(__FILE__,__LINE__,"clmsync: clEnqueueWriteBuffer err %d",err);
	} else if (flags&CL_MEM_HOST) { 
		err = clEnqueueReadBuffer(
			cp->cmdq[devnum],memd->clbuf,CL_FALSE,0,memd->sz,ptr,0,0,&ev
   	);
		DEBUG(__FILE__,__LINE__,"clmsync: clEnqueueReadBuffer err %d",err);
	} else { /* XXX use of no flag is deprecated! disallow it -DAR */
		err = clEnqueueReadBuffer(
			cp->cmdq[devnum],memd->clbuf,CL_FALSE,0,memd->sz,ptr,0,0,&ev
   	);
		DEBUG(__FILE__,__LINE__,"clmsync: clEnqueueReadBuffer err %d",err);
	}


	/* XXX need to add eventq for mem cmds, for now just warn no supp -DAR */

//	WARN(__FILE__,__LINE__,
//		"clmsync warning: blocking, CL_EVENT_NOWAIT unsupported\n");

	if (flags & CL_EVENT_NOWAIT) {

		cp->mev[devnum].ev[cp->mev[devnum].ev_free++] = ev;
		cp->mev[devnum].ev_free %= 128;
		++cp->mev[devnum].nev;

	} else { /* CL_EVENT_WAIT */

		err = clWaitForEvents(1,&ev);

		if (flags & CL_EVENT_RELEASE) {

			clReleaseEvent(ev);
			ev = (cl_event)0;

		}

	}

	return(ev);

}

