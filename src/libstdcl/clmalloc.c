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

#ifdef _WIN64
#include "fix_windows.h"
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
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



#define __MEMD_F_R				0x0001
#define __MEMD_F_W				0x0002
#define __MEMD_F_RW 				(__MEMD_F_R|__MEMD_F_W)
#define __MEMD_F_ATTACHED		0x0004
#define __MEMD_F_LOCKED			0x0008
#define __MEMD_F_DIRTY			0x0010
#define __MEMD_F_TRACKED		0x0020

#define __MEMD_F_IMG				0x0100
#define __MEMD_F_IMG2D			(__MEMD_F_IMG|0x0200)
#define __MEMD_F_IMG3D			(__MEMD_F_IMG|0x0400)

#ifdef ENABLE_CLGL
#define __MEMD_F_GLBUF			0x1000
#define __MEMD_F_GLTEX2D		0x2000
#define __MEMD_F_GLTEX3D		0x4000
#define __MEMD_F_GLRBUF			0x8000
#endif


LIBSTDCL_API 
void* clmalloc(CONTEXT* cp, size_t size, int flags)
{

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

	memd->magic = CLMEM_MAGIC;
	memd->flags = __MEMD_F_RW;
	memd->sz = size;
	if (flags&CL_MEM_IMAGE2D) {
			memd->flags |= __MEMD_F_IMG2D;
			memd->sz1 = 1;
			memd->sz2 = 0;
			memd->imgfmt.image_channel_order = CL_RGBA;
			memd->imgfmt.image_channel_data_type = CL_FLOAT;
	}

	memd->devnum = -1;

	if (flags&CL_MEM_DETACHED) {
	
		memd->clbuf = (cl_mem)0;

	} else {


		if (memd->flags&__MEMD_F_IMG2D) {

			memd->clbuf = clCreateImage2D(
				cp->ctx, CL_MEM_READ_WRITE, &(memd->imgfmt), 
				memd->sz, memd->sz1, memd->sz2,
				NULL, &err
			);

			DEBUG(__FILE__,__LINE__,"create image sz: %d %d %d\n",
				memd->sz, memd->sz1, memd->sz2);

		} else {

			DEBUG(__FILE__,__LINE__,"create buffer");

			memd->clbuf = clCreateBuffer(
  	   	 	cp->ctx,CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
  		  	  	size,(void*)ptr,&err
  		 	);

		}

		DEBUG(__FILE__,__LINE__,"clmalloc: clCreateBuffer clbuf=%p",memd->clbuf);

		DEBUG(__FILE__,__LINE__,"clmalloc: err from clCreateBuffer %d",err);

		memd->flags |= __MEMD_F_ATTACHED;

		if (!memd->clbuf) {

			free((void*)ptri);
			ptr = 0;

		} else {

			LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

		}

	}

	if (memd->flags&__MEMD_F_IMG2D) {
		DEBUG(__FILE__,__LINE__,"clmattach: order = %x",
			memd->imgfmt.image_channel_order);
		DEBUG(__FILE__,__LINE__,"clmattach: type = %x",
			memd->imgfmt.image_channel_data_type);
	}

	return((void*)ptr);
}


LIBSTDCL_API
void clfree( void* ptr )
{
	int err;

	DEBUG(__FILE__,__LINE__,"clfree: ptr=%p\n",ptr);

	if (!__test_memd_magic(ptr)) {

		WARN(__FILE__,__LINE__,"clfree: invalid ptr");

		return;

	}
	
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


LIBSTDCL_API
int clmattach( CONTEXT* cp, void* ptr )
{
	int err;

	DEBUG(__FILE__,__LINE__,"clmattach: ptr=%p",ptr);

	if (!__test_memd_magic(ptr)) {

		ERROR(__FILE__,__LINE__,"clmattach: invalid ptr");

		return(EFAULT);

	}
	
	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	if ( (!memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) 
		|| (memd->clbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {

		DEBUG(__FILE__,__LINE__,"%p %d",
			memd->clbuf,memd->flags&__MEMD_F_ATTACHED);

		ERROR(__FILE__,__LINE__,"clmattach: memd corrupt");

		return(EFAULT);

	}

	if (memd->flags&__MEMD_F_ATTACHED) return(EINVAL);

	if (memd->flags&__MEMD_F_IMG2D) {

			cl_image_format fmt = memd->imgfmt;
			DEBUG(__FILE__,__LINE__,"clmattach: order = %x",fmt.image_channel_order);
			DEBUG(__FILE__,__LINE__,"clmattach: type = %x",fmt.image_channel_data_type);

		memd->clbuf = clCreateImage2D(
			cp->ctx, CL_MEM_READ_WRITE, &(memd->imgfmt), 
			memd->sz, memd->sz1, memd->sz2,
			NULL, &err
		);

	} else {

		memd->clbuf = clCreateBuffer(
//    	 	cp->ctx,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,
    	 	cp->ctx,CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
    	 	memd->sz,(void*)ptr,&err
  		);

	}

	DEBUG(__FILE__,__LINE__,"clmattach: clCreateBuffer clbuf=%p",memd->clbuf);

	DEBUG(__FILE__,__LINE__,"clmattach: err from clCreateBuffer %d",err);

	memd->flags |= __MEMD_F_ATTACHED;

	LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

}


LIBSTDCL_API
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

	memd->clbuf = (cl_mem)0;
	memd->flags &= ~(cl_uint)__MEMD_F_ATTACHED;

	return(0);
}


LIBSTDCL_API
int clmctl_va( void* ptr, int op, va_list ap )
{
	int err; 
	int retval = 0;

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

	void* ptmp;

	switch (op) {

		case CL_MCTL_GET_STATUS:

			retval = memd->flags;

			break;

		case CL_MCTL_GET_DEVNUM:

			retval = memd->devnum;

			break;

		case CL_MCTL_SET_DEVNUM:
//			memd->devnum = arg;

			memd->devnum = va_arg(ap,int);

			break;

		case CL_MCTL_MARK_CLEAN:

			memd->flags &= ~(cl_uint)__MEMD_F_DIRTY;

			break;

		case CL_MCTL_SET_IMAGE2D:

			if (memd->flags&__MEMD_F_ATTACHED) {

				WARN(__FILE__,__LINE__,"clmctl: operation not permitted");

				retval = EPERM;

			} else {

				memd->flags |= __MEMD_F_IMG2D;
				memd->sz = va_arg(ap,size_t);
				memd->sz1 = va_arg(ap,size_t);
				memd->sz2 = 0;
				ptmp = va_arg(ap,void*);

				if (ptmp) {
					memd->imgfmt = *(cl_image_format*)ptmp;
				} else {
					memd->imgfmt.image_channel_order = CL_RGBA;
					memd->imgfmt.image_channel_data_type = CL_FLOAT;
				}

				DEBUG(__FILE__,__LINE__,"clmctl: set image2d");

			}

			break;

		default:

			WARN(__FILE__,__LINE__,"clmctl: invalid operation");

			retval = EINVAL;
			
			break;

	}

	return(retval);

}


LIBSTDCL_API
cl_event 
clmsync(CONTEXT* cp, unsigned int devnum, void* ptr, int flags )
{
	int err;

	cl_event ev;

	if (!ptr) return((cl_event)0);

	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	if (memd->magic != CLMEM_MAGIC) {
		for (
         memd = cp->memd_listhead.lh_first; memd != 0;
         memd = memd->memd_list.le_next
         ) {
            intptr_t p1 = (intptr_t)memd + sizeof(struct _memd_struct);
            intptr_t p2 = p1 + memd->sz;
            if (p1 < (intptr_t)ptr && (intptr_t)ptr < p2) {
               DEBUG(__FILE__,__LINE__,"memd match");
					ptr = (void*)p1;
               break;
            }
      }

		if (memd == 0) {

			WARN(__FILE__,__LINE__,"clmsync: bad pointer");

			return((cl_event)0);

		}

	}

	DEBUG(__FILE__,__LINE__,"clmsync: memd = %p, base_ptr = %p",
		memd,(intptr_t)memd+sizeof(struct _memd_struct));

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

	if (flags&CL_MEM_DEVICE) {

		/* XXX this is a test for tracking-DAR */
		if (flags&CL_MEM_NOFORCE && memd->devnum == devnum) {
			WARN(__FILE__,__LINE__,"clmsync/CL_MEM_NOFORCE no transfer");
			return((cl_event)0);
		}

//		if (memd->flags&__MEMD_F_IMG2D) {
		if (memd->flags&__MEMD_F_IMG) {

			DEBUG(__FILE__,__LINE__,"%d %d",memd->sz,memd->sz1);

			size_t origin[3] = {0,0,0};
			size_t region[3] = { memd->sz, memd->sz1, 1 };

			err = clEnqueueWriteImage(
				cp->cmdq[devnum],memd->clbuf,CL_FALSE,
				origin,region,0,0,ptr,0,NULL,&ev
			);
			DEBUG(__FILE__,__LINE__,"clmsync: clEnqueueWriteImage err %d",err);

		} else {

			err = clEnqueueWriteBuffer(
				cp->cmdq[devnum],memd->clbuf,CL_FALSE,0,memd->sz,ptr,0,0,&ev
  		 	);
			DEBUG(__FILE__,__LINE__,"clmsync: clEnqueueWriteBuffer err %d",err);

		}

		memd->devnum = devnum;


	} else if (flags&CL_MEM_HOST) { 

		/* XXX this is a test for tracking-DAR */
		if (flags&CL_MEM_NOFORCE && memd->devnum == -1) {
			WARN(__FILE__,__LINE__,"clmsync/CL_MEM_NOFORCE no transfer");
			return((cl_event)0);
		}

//		if (memd->flags&__MEMD_F_IMG2D) {
		if (memd->flags&__MEMD_F_IMG) {

			size_t origin[3] = {0,0,0};
			size_t region[3] = { memd->sz, memd->sz1, 1 };

			err = clEnqueueReadImage(
				cp->cmdq[devnum],memd->clbuf,CL_FALSE,
				origin,region,0,0,ptr,0,NULL,&ev
			);

		} else {

			err = clEnqueueReadBuffer(
				cp->cmdq[devnum],memd->clbuf,CL_FALSE,0,memd->sz,ptr,0,0,&ev
   		);

		}

		memd->devnum = -1;

		DEBUG(__FILE__,__LINE__,"clmsync: clEnqueueReadBuffer err %d",err);

	} else {

		WARN(__FILE__,__LINE__,"clmsync: no target specified");

		return((cl_event)0);

	}


	/* XXX need to allow either sync or async transfer supp, add this -DAR */

	if (flags & CL_EVENT_NOWAIT) {

		cp->mev[devnum].ev[cp->mev[devnum].ev_free++] = ev;
		cp->mev[devnum].ev_free %= STDCL_EVENTLIST_MAX;
		++cp->mev[devnum].nev;

	} else { /* CL_EVENT_WAIT */

		err = clWaitForEvents(1,&ev);

		if ( !(flags & CL_EVENT_NORELEASE) ) {
			clReleaseEvent(ev);
			ev = (cl_event)0;
		}

	}

	return(ev);

}


LIBSTDCL_API
cl_event 
clmcopy(
	CONTEXT* cp, unsigned int devnum, void* src, void* dst, int flags )
{
	int err;

	cl_event ev;

	if (!src || !dst) return((cl_event)0);

	intptr_t src_ptri = (intptr_t)src - sizeof(struct _memd_struct);
	struct _memd_struct* src_memd = (struct _memd_struct*)src_ptri;
	intptr_t dst_ptri = (intptr_t)dst - sizeof(struct _memd_struct);
	struct _memd_struct* dst_memd = (struct _memd_struct*)dst_ptri;

	if (src_memd->magic != CLMEM_MAGIC) {
		for (
         src_memd = cp->memd_listhead.lh_first; src_memd != 0;
         src_memd = src_memd->memd_list.le_next
         ) {
            intptr_t p1 = (intptr_t)src_memd + sizeof(struct _memd_struct);
            intptr_t p2 = p1 + src_memd->sz;
            if (p1 < (intptr_t)src && (intptr_t)src < p2) {
               DEBUG(__FILE__,__LINE__,"memd match");
					src = (void*)p1;
               break;
            }
      }

		if (src_memd == 0) {

			WARN(__FILE__,__LINE__,"clmsync: bad pointer");

			return((cl_event)0);

		}

	}

	if (dst_memd->magic != CLMEM_MAGIC) {
		for (
         dst_memd = cp->memd_listhead.lh_first; dst_memd != 0;
         dst_memd = dst_memd->memd_list.le_next
         ) {
            intptr_t p1 = (intptr_t)dst_memd + sizeof(struct _memd_struct);
            intptr_t p2 = p1 + dst_memd->sz;
            if (p1 < (intptr_t)dst && (intptr_t)dst < p2) {
               DEBUG(__FILE__,__LINE__,"memd match");
					dst = (void*)p1;
               break;
            }
      }

		if (dst_memd == 0) {

			WARN(__FILE__,__LINE__,"clmsync: bad pointer");

			return((cl_event)0);

		}

	}

	DEBUG(__FILE__,__LINE__,"clmcopy: (src) memd = %p, base_ptr = %p",
		src_memd,(intptr_t)src_memd+sizeof(struct _memd_struct));

	DEBUG(__FILE__,__LINE__,"clmsync: (dst) memd = %p, base_ptr = %p",
		dst_memd,(intptr_t)dst_memd+sizeof(struct _memd_struct));

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

//	if (src_memd->flags&__MEMD_F_IMG2D) {
	if (src_memd->flags&__MEMD_F_IMG) {

//		if (dst_memd->flags&__MEMD_F_IMG2D) {
		if (dst_memd->flags&__MEMD_F_IMG) {
			
			DEBUG(__FILE__,__LINE__,"%d %d",src_memd->sz,src_memd->sz1);
			DEBUG(__FILE__,__LINE__,"%d %d",dst_memd->sz,dst_memd->sz1);

			size_t origin[3] = {0,0,0};
			size_t region[3] = { src_memd->sz, src_memd->sz1, 1 };

			err = clEnqueueCopyImage( 
				cp->cmdq[devnum],src_memd->clbuf, dst_memd->clbuf,
				origin,origin,region, 0,0,&ev
			);

			DEBUG(__FILE__,__LINE__,"clmsync: clEnqueueCopyImage err %d",err);

		} else {

		}

	} else {

//		if (dst_memd->flags&__MEMD_F_IMG2D) {
		if (dst_memd->flags&__MEMD_F_IMG) {

		} else {

			err = clEnqueueCopyBuffer(
				cp->cmdq[devnum],src_memd->clbuf,dst_memd->clbuf,
				0,0,src_memd->sz,0,0,&ev
  	 		);

			DEBUG(__FILE__,__LINE__,"clmsync: clEnqueueCopyBuffer err %d",err);

		}

	}

	if (flags & CL_EVENT_NOWAIT) {

		cp->mev[devnum].ev[cp->mev[devnum].ev_free++] = ev;
		cp->mev[devnum].ev_free %= STDCL_EVENTLIST_MAX;
		++cp->mev[devnum].nev;

	} else { /* CL_EVENT_WAIT */

		err = clWaitForEvents(1,&ev);

		if ( !(flags & CL_EVENT_NORELEASE) ) {
			clReleaseEvent(ev);
			ev = (cl_event)0;
		}

	}

	return(ev);

}


LIBSTDCL_API
void* clmemptr( CONTEXT* cp, void* ptr ) 
{

	void* p = ptr;

	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;


	if (memd->magic != CLMEM_MAGIC) {
		p = 0;
		for (
         memd = cp->memd_listhead.lh_first; memd != 0;
         memd = memd->memd_list.le_next
         ) {
            intptr_t p1 = (intptr_t)memd + sizeof(struct _memd_struct);
            intptr_t p2 = p1 + memd->sz;
            if (p1 < (intptr_t)ptr && (intptr_t)ptr < p2) {
               p = (void*)p1;
               break;
            }
      }
	}

	return(p);

}


LIBSTDCL_API
void* clmrealloc( CONTEXT* cp, void* p, size_t size, int flags )
{
	int err;

	DEBUG(__FILE__,__LINE__,"clmrealloc: p=%p size=%d flag=%d",p,size,flags);

	if (p == 0) return clmalloc(cp,size,flags);

	if (!__test_memd_magic(p)) {

		ERROR(__FILE__,__LINE__,"clmrealloc: invalid ptr");

		return(0);
	}

	if ((flags&CL_MEM_READ_ONLY) || (flags&CL_MEM_WRITE_ONLY)) {
		WARN(__FILE__,__LINE__,
			"clmrealloc: CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY unsupported");
	} //// XXX CL_MEM_READ_WRITE implied -DAR


	intptr_t ptr = (intptr_t)p;

	intptr_t ptri;
	struct _memd_struct* memd;
	cl_uint memd_flags;


	if (ptr) {

		DEBUG(__FILE__,__LINE__,"ptr != 0");
		ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
		memd = (struct _memd_struct*)ptri;
		memd_flags = memd->flags;

#ifdef ENABLE_CLGL
		if (memd_flags&__MEMD_F_GLBUF) {

			ERROR(__FILE__,__LINE__,"clmrealloc: invalid ptr");

			return(0);
		}
#endif

		if ( (!memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) 
			|| (memd->clbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {

			ERROR(__FILE__,__LINE__,"clmrealloc: memd corrupt");

			return(0);
		}

		if (memd->flags&__MEMD_F_ATTACHED) {

			err = clReleaseMemObject(memd->clbuf);

			LIST_REMOVE(memd, memd_list);

		}

		ptri = (intptr_t)realloc((void*)ptri,size+sizeof(struct _memd_struct));

		DEBUG(__FILE__,__LINE__,"%p %d",ptri,size+sizeof(struct _memd_struct));

	} else {

		ptri = (intptr_t)malloc(size+sizeof(struct _memd_struct));

		memd_flags = __MEMD_F_RW;

	}


	ptr = ptri+sizeof(struct _memd_struct);
	memd = (struct _memd_struct*)ptri;
	DEBUG(__FILE__,__LINE__,"%p %p",ptr,memd);

	memd->magic = CLMEM_MAGIC;
	memd->flags = memd_flags;
	memd->sz = size;


	if (flags&CL_MEM_DETACHED) {
	
		DEBUG(__FILE__,__LINE__,"detached %p %p",ptr,memd);
		memd->clbuf = (cl_mem)0;
		memd->flags &= ~(__MEMD_F_ATTACHED);

	} else {

		if (memd->flags&__MEMD_F_IMG2D) {

			memd->clbuf = clCreateImage2D(
				cp->ctx, CL_MEM_READ_WRITE, &(memd->imgfmt), 
				memd->sz, memd->sz1, memd->sz2,
				NULL, &err
			);

		} else {

			memd->clbuf = clCreateBuffer(
  	   	 	cp->ctx,CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
  	  	  		size,(void*)ptr,&err
  		 	);

		}

		DEBUG(__FILE__,__LINE__,"clmrealloc: clCreateBuffer clbuf=%p",memd->clbuf);

		DEBUG(__FILE__,__LINE__,"clmrealloc: err from clCreateBuffer %d",err);

		memd->flags |= __MEMD_F_ATTACHED;

		if (!memd->clbuf) {

			free((void*)ptri);
			ptr = 0;

		} else {

			LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

		}

	}

	if (!__test_memd_magic((void*)ptr)) {

		ERROR(__FILE__,__LINE__,"clmrealloc: invalid ptr");

		return(0);
	}

	return((void*)ptr);

}


#ifdef ENABLE_CLGL

void* clglmalloc(CONTEXT* cp, cl_GLuint glbuf, int flags)
{

	DEBUG(__FILE__,__LINE__,"clglmalloc: glbuf=%d flag=%d",glbuf,flags);

	int err;

	cl_mem tmp_clbuf;
	
	if (flags&CL_MEM_DETACHED) {
	
		WARN(__FILE__,__LINE__,"clglmalloc: invalid flag: CL_MEM_DETACHED");

		return(0);

	}

	tmp_clbuf = clCreateFromGLBuffer(
     	cp->ctx,CL_MEM_READ_WRITE,
     	glbuf, &err
  	);

	DEBUG(__FILE__,__LINE__,
		"clglmalloc: clCreateFromGLBuffer clbuf=%p",tmp_clbuf);

	DEBUG(__FILE__,__LINE__, "clglmalloc: err from clCreateFromGLBuffer %d",err);

	size_t size;
	err = clGetMemObjectInfo(tmp_clbuf,CL_MEM_SIZE,sizeof(size_t),&size,0);

	DEBUG(__FILE__,__LINE__,"clglmalloc: clGetMemObjectInfo err %d",err);

	if (size == 0) {

		WARN(__FILE__,__LINE__,"clglmalloc: size=0, something went wrong");

		clReleaseMemObject(tmp_clbuf);

		return(0);

	}

	intptr_t ptri = (intptr_t)malloc(size+sizeof(struct _memd_struct));

	if (ptri == 0) {

		WARN(__FILE__,__LINE__,"clglmalloc: out of memory");

		clReleaseMemObject(tmp_clbuf);

		return(0);

	}

	intptr_t ptr = ptri+sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	DEBUG(__FILE__,__LINE__,"clglmalloc: ptri=%p ptr=%p memd=%p",ptri,ptr,memd);

	DEBUG(__FILE__,__LINE__,"clglmalloc: sizeof struct _memd_struct %d",
		sizeof(struct _memd_struct));

	if ((flags&CL_MEM_READ_ONLY) || (flags&CL_MEM_WRITE_ONLY)) {
		WARN(__FILE__,__LINE__,
			"clglmalloc: CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY unsupported");
	} //// XXX CL_MEM_READ_WRITE implied -DAR

	memd->clbuf = tmp_clbuf;
	memd->magic = CLMEM_MAGIC;
	memd->flags = __MEMD_F_RW|__MEMD_F_GLBUF|__MEMD_F_ATTACHED;
	memd->sz = size;

	LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

	return((void*)ptr);

}


void* xxx_clglmalloc(
	CONTEXT* cp, cl_GLuint glbuf, int flags, 
	cl_GLenum target, cl_GLint miplevel 
)
{

	DEBUG(__FILE__,__LINE__,"xxx_clglmalloc: glbuf=%d flag=%d",glbuf,flags);

	int err;

	unsigned int tmp_flags = 0;
	cl_mem tmp_clbuf = 0;
	
	if (flags&CL_MEM_DETACHED) {
	
		WARN(__FILE__,__LINE__,"xxx_clglmalloc: invalid flag: CL_MEM_DETACHED");

		return(0);

	}

	tmp_clbuf = clCreateFromGLBuffer(
     	cp->ctx,CL_MEM_READ_WRITE,
     	glbuf,&err
  	);

	if (flags&CL_MEM_GLTEX2D) {

		tmp_clbuf = clCreateFromGLTexture2D(
    	 	cp->ctx,CL_MEM_READ_WRITE,
			target,miplevel,
    	 	glbuf,&err
  		);

		tmp_flags = __MEMD_F_RW|__MEMD_F_IMG2D|__MEMD_F_GLTEX2D|__MEMD_F_ATTACHED;

	} else if (flags&CL_MEM_GLTEX3D) {

		tmp_clbuf = clCreateFromGLTexture3D(
    	 	cp->ctx,CL_MEM_READ_WRITE,
			target,miplevel,
    	 	glbuf,&err
  		);

		tmp_flags = __MEMD_F_RW|__MEMD_F_IMG3D|__MEMD_F_GLTEX3D|__MEMD_F_ATTACHED;

	} else if (flags&CL_MEM_GLRBUF) {

		tmp_clbuf = clCreateFromGLRenderbuffer(
    	 	cp->ctx,CL_MEM_READ_WRITE,
    	 	glbuf,&err
  		);

		tmp_flags = __MEMD_F_RW|__MEMD_F_IMG2D|__MEMD_F_GLRBUF|__MEMD_F_ATTACHED;

	} else {

		WARN(__FILE__,__LINE__,"xxx_clglmalloc: invalid flags");

		return(0);

	}

	DEBUG(__FILE__,__LINE__,
		"xxx_clglmalloc: clCreateFromGL* clbuf=%p",tmp_clbuf);

	DEBUG(__FILE__,__LINE__, "xxx_clglmalloc: err from clCreateFromGL* %d",err);

	size_t size;
	err = clGetMemObjectInfo(tmp_clbuf,CL_MEM_SIZE,sizeof(size_t),&size,0);

	DEBUG(__FILE__,__LINE__,"clglmalloc: clGetMemObjectInfo err %d",err);

	if (size == 0) {

		WARN(__FILE__,__LINE__,"clglmalloc: size=0, something went wrong");

		clReleaseMemObject(tmp_clbuf);

		return(0);

	}

	intptr_t ptri = (intptr_t)malloc(size+sizeof(struct _memd_struct));

	if (ptri == 0) {

		WARN(__FILE__,__LINE__,"clglmalloc: out of memory");

		clReleaseMemObject(tmp_clbuf);

		return(0);

	}

	intptr_t ptr = ptri+sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	DEBUG(__FILE__,__LINE__,"clmalloc: ptri=%p ptr=%p memd=%p",ptri,ptr,memd);

	DEBUG(__FILE__,__LINE__,"clmalloc: sizeof struct _memd_struct %d",
		sizeof(struct _memd_struct));

	if ((flags&CL_MEM_READ_ONLY) || (flags&CL_MEM_WRITE_ONLY)) {
		WARN(__FILE__,__LINE__,
			"clmalloc: CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY unsupported");
	} //// XXX CL_MEM_READ_WRITE implied -DAR


	memd->clbuf = tmp_clbuf;
	memd->magic = CLMEM_MAGIC;
	memd->flags = tmp_flags;
	memd->sz = size;


	LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

	return((void*)ptr);

}


cl_event 
clglmsync(CONTEXT* cp, unsigned int devnum, void* ptr, int flags )
{
	int err;

	cl_event ev;

	if (!ptr) return((cl_event)0);


	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	if (memd->magic != CLMEM_MAGIC) {
		for (
         memd = cp->memd_listhead.lh_first; memd != 0;
         memd = memd->memd_list.le_next
         ) {
            intptr_t p1 = (intptr_t)memd + sizeof(struct _memd_struct);
            intptr_t p2 = p1 + memd->sz;
            if (p1 < (intptr_t)ptr && (intptr_t)ptr < p2) {
               DEBUG(__FILE__,__LINE__,"memd match");
					ptr = (void*)p1;
               break;
            }
      }
	}


	DEBUG(__FILE__,__LINE__,"clglmsync: memd = %p, base_ptr = %p",
		memd,(intptr_t)memd+sizeof(struct _memd_struct));


	WARN(__FILE__,__LINE__,"clglmsync: no supp for dev/host sync yet");

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

	if (flags&CL_MEM_CLBUF) {

		err = clEnqueueAcquireGLObjects( 
			cp->cmdq[devnum],1,&(memd->clbuf),0,0,&ev);

		DEBUG(__FILE__,__LINE__,
			"clglmsync: clEnqueueAcquireGLObjects err %d",err);

	} else if (flags&CL_MEM_GLBUF) {

		err = clEnqueueReleaseGLObjects(
			cp->cmdq[devnum],1,&(memd->clbuf),0,0,&ev);

		DEBUG(__FILE__,__LINE__,
			"clglmsync: clEnqueueReleaseGLObjects err %d",err);
	
	} else {

		WARN(__FILE__,__LINE__,"clglmsync: invalid flag, did nothing");

		return((cl_event)0);

	}


	if (flags & CL_EVENT_NOWAIT) {

		cp->mev[devnum].ev[cp->mev[devnum].ev_free++] = ev;
		cp->mev[devnum].ev_free %= STDCL_EVENTLIST_MAX;
		++cp->mev[devnum].nev;

	} else { /* CL_EVENT_WAIT */

		err = clWaitForEvents(1,&ev);

		if ( !(flags & CL_EVENT_NORELEASE) ) {
			clReleaseEvent(ev);
			ev = (cl_event)0;
		}

	}

	return(ev);

}

#endif

