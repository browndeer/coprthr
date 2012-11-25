/* clmalloc.c
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

#include "util.h"

#include "stdcl.h"
#include "clmalloc.h"
#include "clsched.h"
#include "printcl.h"
#include "clerrno.h"

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

#ifndef DISABLE_CLGL
#define __MEMD_F_GLBUF			0x1000
#define __MEMD_F_GLTEX2D		0x2000
#define __MEMD_F_GLTEX3D		0x4000
#define __MEMD_F_GLRBUF			0x8000
#endif


static int 
__create_mem_objects(struct _memd_struct* memd,CLCONTEXT* cp, void* ptr)
{
	int ictx;
	int err, err_ret = 0;

	size_t sz = (cp->nctx + cp->ndev) * sizeof(cl_mem);

	memd->nclbuf = cp->nctx;

	if (sz > MEMD_FREE_SZ) 
		memd->xxxclbuf = (cl_mem*)malloc(sz);
	else
		memd->xxxclbuf = (cl_mem*)&(memd->_free_offset);

	memd->devbuf = memd->xxxclbuf + memd->nclbuf;

	if (memd->flags&__MEMD_F_IMG2D) {

		int idev = 0;
		for(ictx=0; ictx < cp->nctx; ictx++) {
		
			memd->xxxclbuf[ictx] = clCreateImage2D( cp->xxxctx[ictx], 
				CL_MEM_READ_WRITE, 
				&(memd->imgfmt), memd->sz, memd->sz1, memd->sz2, NULL, &err);

			__set_oclerrno(err);

			printcl( CL_DEBUG "create image: %p (%d %d %d)\n",
				memd->xxxclbuf[ictx], memd->sz, memd->sz1, memd->sz2);

			if (err)
				printcl( CL_ERR "clmalloc: err from clCreateImage %d",err);

			err_ret |= err;

			while(idev < cp->ndev && cp->xxxctx[ictx] == cp->devctx[idev]) 
				memd->devbuf[idev++] = memd->xxxclbuf[ictx];

		}

	} else {

		int idev = 0;
		for(ictx=0; ictx < cp->nctx; ictx++) {

			memd->xxxclbuf[ictx] = clCreateBuffer( cp->xxxctx[ictx],
				CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, memd->sz,ptr,&err);

			__set_oclerrno(err);

			printcl( CL_DEBUG "create buffer: %p (%d)\n", 
				memd->xxxclbuf[ictx],memd->sz);

			if (err)
				printcl( CL_ERR "clmalloc: err from clCreateBuffer %d",err);

			err_ret |= err;

			while(idev < cp->ndev && cp->xxxctx[ictx] == cp->devctx[idev]) 
				memd->devbuf[idev++] = memd->xxxclbuf[ictx];

		}

	}

	return(err_ret);
}

static int 
__release_mem_objects(struct _memd_struct* memd)
{
	int ibuf;
	int err, err_ret = 0;

	for(ibuf=0; ibuf < memd->nclbuf; ibuf++) {

		if (memd->xxxclbuf[ibuf]) {

			err = clReleaseMemObject(memd->xxxclbuf[ibuf]);

			__set_oclerrno(err);

			if (err)
				printcl( CL_ERR "clmalloc: err from clReleaseMemObject %d",err);

			err_ret |= err;
		}

	}

	printcl( CL_DEBUG "xxxclbuf %p %p %ld",
		memd->xxxclbuf, memd, (intptr_t)memd->xxxclbuf - (intptr_t)memd);

	if ( (intptr_t)memd->xxxclbuf-(intptr_t)memd >= sizeof(struct _memd_struct))
		free(memd->xxxclbuf);

	memd->xxxclbuf = 0;
	memd->devbuf = 0;
	memd->nclbuf = 0;

	return(err_ret);
}

/* XXX check for corruption (if only it were this simple) -DAR */
static int __test_memd_corrupt( struct _memd_struct* memd )
{
	int i;
	if (memd->flags&__MEMD_F_ATTACHED) 
		for(i=0;i<memd->nclbuf;i++) {
//			printcl( CL_DEBUG "__test_memd_corrupt: attached: %p",
//				memd->xxxclbuf[i]);
			if (!memd->xxxclbuf[i]) return(1);
		}
	else
		for(i=0;i<memd->nclbuf;i++) {
//			printcl( CL_DEBUG "__test_memd_corrupt: detached: %p",
//				memd->xxxclbuf[i]);
			if (memd->xxxclbuf[i]) return(1); 
		}
	return(0);
}


LIBSTDCL_API 
void* clmalloc(CONTEXT* cp, size_t size, int flags)
{

	printcl( CL_DEBUG "clmalloc: size=%d flag=%d",size,flags);

	int err;
	intptr_t ptri = (intptr_t)malloc(size+sizeof(struct _memd_struct));
	intptr_t ptr = ptri+sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;
	memset(memd,0,sizeof(struct _memd_struct));

	printcl( CL_DEBUG "clmalloc: ptri=%p ptr=%p memd=%p",ptri,ptr,memd);

	printcl( CL_DEBUG "clmalloc: sizeof struct _memd_struct %d",
		sizeof(struct _memd_struct));

	if ((flags&CL_MEM_READ_ONLY) || (flags&CL_MEM_WRITE_ONLY)) {
		printcl( CL_WARNING 
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
	memd->usrflags = (cl_mem_flags)0;

	memd->devnum = -1;

	if (flags&CL_MEM_DETACHED) {
	
//		memd->clbuf = (cl_mem)0;
		memd->nclbuf = 0;
		memd->xxxclbuf = 0;

	} else {

/*
		memd->nclbuf = cp->nctx;

		if (memd->nclbuf * sizeof(cl_mem) > MEMD_FREE_SZ) 
			memd->xxxclbuf = (cl_mem*)malloc(memd->nclbuf * sizeof(cl_mem));
		else
			memd->xxxclbuf = (cl_mem*)&(memd->_free_offset);

		if (memd->flags&__MEMD_F_IMG2D) {

//			memd->clbuf = clCreateImage2D( cp->ctx, CL_MEM_READ_WRITE, 
			memd->xxxclbuf[0] = clCreateImage2D( cp->xxxctx[0], CL_MEM_READ_WRITE, 
				&(memd->imgfmt), memd->sz, memd->sz1, memd->sz2, NULL, &err);

			__set_oclerrno(err);

			printcl( CL_DEBUG "create image sz: %d %d %d\n",
				memd->sz, memd->sz1, memd->sz2);

		} else {

			printcl( CL_DEBUG "create buffer");

//			memd->clbuf = clCreateBuffer( cp->ctx,
			memd->xxxclbuf[0] = clCreateBuffer( cp->xxxctx[0],
				CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, size,(void*)ptr,&err);

			__set_oclerrno(err);

		}

		printcl( CL_DEBUG "clmalloc: clCreateBuffer clbuf[0]=%p",
			memd->xxxclbuf[0]);

		printcl( CL_DEBUG "clmalloc: err from clCreateBuffer %d",err);
*/

		err = __create_mem_objects(memd,cp,(void*)ptr);


//		if (!memd->clbuf) {
//		if (!memd->xxxclbuf[0]) {
		if (err) {

			printcl( CL_ERR "clmalloc: create mem objects failed" );

			__set_clerrno(-1);

//			if (memd->xxxclbuf != (cl_mem*)&(memd->_free_offset)) {
//				free(memd->xxxclbuf);
//			}
			__release_mem_objects(memd);

			free((void*)ptri);

			ptr = 0;

		} else {

			memd->flags |= __MEMD_F_ATTACHED;

			LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

		}

	}


	if (memd->flags&__MEMD_F_IMG2D) {

		printcl( CL_DEBUG "clmattach: order = %x",
			memd->imgfmt.image_channel_order);

		printcl( CL_DEBUG "clmattach: type = %x",
			memd->imgfmt.image_channel_data_type);

	}

	if (__test_memd_corrupt(memd))
		printcl( CL_ERR "clmalloc: memd corrupt");

	return((void*)ptr);
}


LIBSTDCL_API
int clfree( void* ptr )
{
	int err;

	printcl( CL_DEBUG "clfree: ptr=%p\n",ptr);

	if (!__test_memd_magic(ptr)) {

		printcl( CL_WARNING "clfree: invalid ptr");

		return 0 ;

	}

	if (!ptr) { printcl( CL_WARNING "clfree: null ptr"); return 0; }

//	if (!assert_cldev_valid(dev)) return (0);

	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	if (__test_memd_corrupt(memd)) {
		printcl( CL_ERR "clfree: memd corrupt");
		__set_clerrno(EFAULT);
		return(EFAULT);
	}

/*
//	if (memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) {
	if (memd->xxxclbuf[0] && (memd->flags&__MEMD_F_ATTACHED)) {

//		err = clReleaseMemObject(memd->clbuf);
		err = clReleaseMemObject(memd->xxxclbuf[0]);

		__set_oclerrno(err);

		LIST_REMOVE(memd, memd_list);

	}

	if (memd->xxxclbuf != (cl_mem*)&(memd->_free_offset)) {
		free(memd->xxxclbuf);
	}
*/

	if (memd->flags&__MEMD_F_ATTACHED) {

		err = __release_mem_objects(memd);

		if (err) {
			
			printcl( CL_ERR "clfree: mem object release failed");
			__set_clerrno(-1);

		}

		LIST_REMOVE(memd, memd_list);

	}

	free((void*)ptri);	

	return 0;	
}


LIBSTDCL_API
int clmattach( CONTEXT* cp, void* ptr )
{
	int i;
	int err;

	printcl( CL_DEBUG "clmattach: ptr=%p",ptr);

	if (!__test_memd_magic(ptr)) {

		printcl( CL_ERR "clmattach: invalid ptr");

		return(EFAULT);

	}
	
	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

/*
//	if ( (!memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) 
//		|| (memd->clbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {
	if ( (!memd->xxxclbuf && (memd->flags&__MEMD_F_ATTACHED)) 
		|| (memd->xxxclbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {

		printcl( CL_DEBUG "%p %d",
//			memd->clbuf,memd->flags&__MEMD_F_ATTACHED);
			memd->xxxclbuf,memd->flags&__MEMD_F_ATTACHED);

		printcl( CL_ERR "clmattach: memd corrupt");

		return(EFAULT);

	}
*/

	if (__test_memd_corrupt(memd)) {
		printcl( CL_ERR "clmattach: memd corrupt");
		__set_clerrno(EFAULT);
		return(EFAULT);
	}

	if (memd->flags&__MEMD_F_ATTACHED) {
		printcl( CL_ERR "clmattach: memory already attached");
		__set_clerrno(EINVAL);
		return(EINVAL);
	}

/*
	memd->nclbuf = cp->nctx;

	if (memd->nclbuf * sizeof(cl_mem) > MEMD_FREE_SZ) 
		memd->xxxclbuf = (cl_mem*)malloc(memd->nclbuf * sizeof(cl_mem));
	else
		memd->xxxclbuf = (cl_mem*)&(memd->_free_offset);


//	if (memd->flags&__MEMD_F_IMG2D) {
	if (memd->flags&__MEMD_F_IMG) {

			cl_image_format fmt = memd->imgfmt;
			printcl( CL_DEBUG "clmattach: order = %x",fmt.image_channel_order);
			printcl( CL_DEBUG "clmattach: type = %x",fmt.image_channel_data_type);

//		memd->clbuf = clCreateImage2D( cp->ctx, CL_MEM_READ_WRITE|memd->usrflags,
		memd->xxxclbuf[0] = clCreateImage2D( cp->xxxctx[0], 
			CL_MEM_READ_WRITE|memd->usrflags,
			&(memd->imgfmt), memd->sz, memd->sz1, memd->sz2, NULL, &err);

		__set_oclerrno(err);

	} else {

//		memd->clbuf = clCreateBuffer( cp->ctx,
		memd->xxxclbuf[0] = clCreateBuffer( cp->xxxctx[0],
			CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR|memd->usrflags,
    	 	memd->sz,(void*)ptr,&err);

		__set_oclerrno(err);

	}

//	printcl( CL_DEBUG "clmattach: clCreateBuffer clbuf=%p",memd->clbuf);
	printcl( CL_DEBUG "clmattach: clCreateBuffer clbuf[0]=%p",memd->xxxclbuf[0]);

	printcl( CL_DEBUG "clmattach: err from clCreateBuffer %d",err);
*/

	err = __create_mem_objects(memd,cp,(void*)ptr);

	if (err) {

		printcl( CL_ERR "clmalloc: create mem objects failed" );

		__set_clerrno(-1);

		__release_mem_objects(memd);


	} else {

		memd->flags |= __MEMD_F_ATTACHED;

		LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

	}

	return(0);
}


LIBSTDCL_API
int clmdetach( void* ptr )
{
	int i;
	int err; 

	if (!__test_memd_magic(ptr)) {

		printcl( CL_ERR "clmdetach: invalid ptr");

		return(EFAULT);

	}

	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

/*
//	if ( (!memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) 
//		|| (memd->clbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {
	if ( (!memd->xxxclbuf && (memd->flags&__MEMD_F_ATTACHED)) 
		|| (memd->xxxclbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {

		printcl( CL_ERR "clmdetach: memd corrupt");

		return(EFAULT);
	}
*/

	if (__test_memd_corrupt(memd)) {
		printcl( CL_ERR "clmdetach: memd corrupt");
		__set_clerrno(EFAULT);
		return(EFAULT);
	}

	if (!(memd->flags&__MEMD_F_ATTACHED)) {
		printcl( CL_ERR "clmdetach: memory not attached");
		__set_clerrno(EINVAL);
		return(EINVAL);
	}

/*
//	err = clReleaseMemObject(memd->clbuf);
	err = clReleaseMemObject(memd->xxxclbuf[0]);

	__set_oclerrno(err);
*/

	LIST_REMOVE(memd, memd_list);

/*
//	memd->clbuf = (cl_mem)0;
	memd->xxxclbuf[0] = (cl_mem)0;

	if (memd->xxxclbuf != (cl_mem*)&(memd->_free_offset)) {
		free(memd->xxxclbuf);
	}

	memd->nclbuf = 0;
	memd->xxxclbuf = 0;
*/
	__release_mem_objects(memd);

	memd->flags &= ~(cl_uint)__MEMD_F_ATTACHED;

	return(0);
}


LIBSTDCL_API
int clmctl_va( void* ptr, int op, va_list ap )
{
	int err; 
	int retval = 0;

	if (!__test_memd_magic(ptr)) {

		printcl( CL_ERR "clmdetach: invalid ptr");

		return(EFAULT);

	}

	intptr_t ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

/*
//	if ( (!memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) 
//		|| (memd->clbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {
	if ( (!memd->xxxclbuf && (memd->flags&__MEMD_F_ATTACHED)) 
		|| (memd->xxxclbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {

		printcl( CL_ERR "clmdetach: memd corrupt");

		return(EFAULT);

	}
*/
	if (__test_memd_corrupt(memd)) {
		printcl( CL_ERR "clmctl: memd corrupt");
		__set_clerrno(EFAULT);
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

				printcl( CL_WARNING "clmctl: operation not permitted");

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

				printcl( CL_DEBUG "clmctl: set image2d");

			}

			break;

		case CL_MCTL_SET_USRFLAGS:

			if (memd->flags&__MEMD_F_ATTACHED) {

				printcl( CL_WARNING "clmctl: operation not permitted");

				retval = EPERM;

			} else {

				cl_mem_flags usrflags = va_arg(ap,cl_mem_flags);

				memd->usrflags |= usrflags;

				printcl( CL_DEBUG "clmctl: set usrflags: 0x%x", memd->usrflags);

			}

		case CL_MCTL_CLR_USRFLAGS:

			if (memd->flags&__MEMD_F_ATTACHED) {

				printcl( CL_WARNING "clmctl: operation not permitted");

				retval = EPERM;

			} else {

				cl_mem_flags usrflags = va_arg(ap,cl_mem_flags);

				memd->usrflags &= ~usrflags;

				printcl( CL_DEBUG "clmctl: clr usrflags: 0x%x", memd->usrflags);

			}

		default:

			printcl( CL_WARNING "clmctl: invalid operation");

			__set_clerrno(EINVAL);

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
               printcl( CL_DEBUG "memd match");
					ptr = (void*)p1;
               break;
            }
      }

		if (memd == 0) {

			printcl( CL_ERR "clmsync: bad pointer");
			return((cl_event)0);

		}

	}

	if (__test_memd_corrupt(memd)) {
		printcl( CL_ERR "clmsync: memd corrupt");
		__set_clerrno(EFAULT);
		return((cl_event)0);		
	}

	printcl( CL_DEBUG "clmsync: memd = %p, base_ptr = %p",
		memd,(intptr_t)memd+sizeof(struct _memd_struct));

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

	if (flags&CL_MEM_DEVICE) {

		/* XXX this is a test for tracking-DAR */
		if (flags&CL_MEM_NOFORCE && memd->devnum == devnum) {
			printcl( CL_WARNING "clmsync/CL_MEM_NOFORCE no transfer");
			return((cl_event)0);
		}

//		if (memd->flags&__MEMD_F_IMG2D) {
		if (memd->flags&__MEMD_F_IMG) {

			printcl( CL_DEBUG "%d %d",memd->sz,memd->sz1);

			size_t origin[3] = {0,0,0};
			size_t region[3] = { memd->sz, memd->sz1, 1 };

//			err = clEnqueueWriteImage( cp->cmdq[devnum],memd->clbuf,CL_FALSE,
//			err = clEnqueueWriteImage( cp->cmdq[devnum],memd->xxxclbuf[0],
			err = clEnqueueWriteImage( cp->cmdq[devnum],memd->devbuf[devnum],
				CL_FALSE,origin,region,0,0,ptr,0,NULL,&ev);

			__set_oclerrno(err);

			printcl( CL_DEBUG "clmsync: clEnqueueWriteImage err %d",err);

		} else {

//			err = clEnqueueWriteBuffer( cp->cmdq[devnum],memd->clbuf,
//			err = clEnqueueWriteBuffer( cp->cmdq[devnum],memd->xxxclbuf[0],
			err = clEnqueueWriteBuffer( cp->cmdq[devnum],memd->devbuf[devnum],
				CL_FALSE,0,memd->sz,ptr,0,0,&ev);

			__set_oclerrno(err);

			printcl( CL_DEBUG "clmsync: clEnqueueWriteBuffer err %d",err);

		}

		memd->devnum = devnum;


	} else if (flags&CL_MEM_HOST) { 

		/* XXX this is a test for tracking-DAR */
		if (flags&CL_MEM_NOFORCE && memd->devnum == -1) {
			printcl( CL_WARNING "clmsync/CL_MEM_NOFORCE no transfer");
			return((cl_event)0);
		}

//		if (memd->flags&__MEMD_F_IMG2D) {
		if (memd->flags&__MEMD_F_IMG) {

			size_t origin[3] = {0,0,0};
			size_t region[3] = { memd->sz, memd->sz1, 1 };

//			err = clEnqueueReadImage( cp->cmdq[devnum],memd->clbuf,CL_FALSE,
//			err = clEnqueueReadImage( cp->cmdq[devnum],memd->xxxclbuf[0],
			err = clEnqueueReadImage( cp->cmdq[devnum],memd->devbuf[devnum],
				CL_FALSE,origin,region,0,0,ptr,0,NULL,&ev);

			__set_oclerrno(err);

			printcl( CL_DEBUG "clmsync: clEnqueueReadImage err %d",err);

		} else {

//			err = clEnqueueReadBuffer( cp->cmdq[devnum],memd->clbuf,
//			err = clEnqueueReadBuffer( cp->cmdq[devnum],memd->xxxclbuf[0],
			err = clEnqueueReadBuffer( cp->cmdq[devnum],memd->devbuf[devnum],
				CL_FALSE,0,memd->sz,ptr,0,0,&ev);

			__set_oclerrno(err);

			printcl( CL_DEBUG "clmsync: clEnqueueReadBuffer err %d",err);

		}

		memd->devnum = -1;

	} else {

		printcl( CL_WARNING "clmsync: no target specified");

		return((cl_event)0);

	}


	/* XXX need to allow either sync or async transfer supp, add this -DAR */

	if (flags & CL_EVENT_NOWAIT) {

		cp->mev[devnum].ev[cp->mev[devnum].ev_free++] = ev;
		cp->mev[devnum].ev_free %= STDCL_EVENTLIST_MAX;
		++cp->mev[devnum].nev;

	} else { /* CL_EVENT_WAIT */

		err = clWaitForEvents(1,&ev);

		__set_oclerrno(err);

		if ( !(flags & CL_EVENT_NORELEASE) ) {

			err = clReleaseEvent(ev);

			__set_oclerrno(err);

			ev = (cl_event)0;

		}

	}

	return(ev);

}


LIBSTDCL_API
cl_event clmcopy(
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
               printcl( CL_DEBUG "memd match");
					src = (void*)p1;
               break;
            }
      }

		if (src_memd == 0) {

			printcl( CL_ERR "clmsync: bad pointer");

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
               printcl( CL_DEBUG "memd match");
					dst = (void*)p1;
               break;
            }
      }

		if (dst_memd == 0) {

			printcl( CL_ERR "clmsync: bad pointer");

			return((cl_event)0);

		}

	}

	if (__test_memd_corrupt(src_memd)) {
		printcl( CL_ERR "clmcopy: src_memd corrupt");
		__set_clerrno(EFAULT);
		return((cl_event)0);
	}

	if (__test_memd_corrupt(dst_memd)) {
		printcl( CL_ERR "clmcopy: dst_memd corrupt");
		__set_clerrno(EFAULT);
		return((cl_event)0);
	}

	printcl( CL_DEBUG "clmcopy: (src) memd = %p, base_ptr = %p",
		src_memd,(intptr_t)src_memd+sizeof(struct _memd_struct));

	printcl( CL_DEBUG "clmsync: (dst) memd = %p, base_ptr = %p",
		dst_memd,(intptr_t)dst_memd+sizeof(struct _memd_struct));

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

//	if (src_memd->flags&__MEMD_F_IMG2D) {
	if (src_memd->flags&__MEMD_F_IMG) {

//		if (dst_memd->flags&__MEMD_F_IMG2D) {
		if (dst_memd->flags&__MEMD_F_IMG) {
			
			printcl( CL_DEBUG "%d %d",src_memd->sz,src_memd->sz1);
			printcl( CL_DEBUG "%d %d",dst_memd->sz,dst_memd->sz1);

			size_t origin[3] = {0,0,0};
			size_t region[3] = { src_memd->sz, src_memd->sz1, 1 };

			err = clEnqueueCopyImage( cp->cmdq[devnum],
//				src_memd->clbuf, dst_memd->clbuf, origin,origin,region, 0,0,&ev);
//				src_memd->xxxclbuf[0], dst_memd->xxxclbuf[0], 
				src_memd->devbuf[devnum], dst_memd->devbuf[devnum], 
				origin,origin,region, 0,0,&ev);

			__set_oclerrno(err);

			printcl( CL_DEBUG "clmsync: clEnqueueCopyImage err %d",err);

		} else {

		}

	} else {

//		if (dst_memd->flags&__MEMD_F_IMG2D) {
		if (dst_memd->flags&__MEMD_F_IMG) {

		} else {

			err = clEnqueueCopyBuffer( cp->cmdq[devnum],
//				src_memd->clbuf,dst_memd->clbuf, 0,0,src_memd->sz,0,0,&ev);
//				src_memd->xxxclbuf[0],dst_memd->xxxclbuf[0], 
				src_memd->devbuf[devnum],dst_memd->devbuf[devnum], 
				0,0,src_memd->sz,0,0,&ev);

			__set_oclerrno(err);

			printcl( CL_DEBUG "clmsync: clEnqueueCopyBuffer err %d",err);

		}

	}

	if (flags & CL_EVENT_NOWAIT) {

		cp->mev[devnum].ev[cp->mev[devnum].ev_free++] = ev;
		cp->mev[devnum].ev_free %= STDCL_EVENTLIST_MAX;
		++cp->mev[devnum].nev;

	} else { /* CL_EVENT_WAIT */

		err = clWaitForEvents(1,&ev);

		__set_oclerrno(err);

		if ( !(flags & CL_EVENT_NORELEASE) ) {

			err = clReleaseEvent(ev);

			__set_oclerrno(err);

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

	if (__test_memd_corrupt(memd)) {
		printcl( CL_ERR "clmemptr: memd corrupt");
		__set_clerrno(EFAULT);
		return(0);
	}

	return(p);

}


LIBSTDCL_API
void* clmrealloc( CONTEXT* cp, void* p, size_t size, int flags )
{
	int err;

	printcl( CL_DEBUG "clmrealloc: p=%p size=%d flag=%d",p,size,flags);

	if (p == 0) return clmalloc(cp,size,flags);

	if (!__test_memd_magic(p)) {
		printcl( CL_ERR "clmrealloc: invalid ptr");
		return(0);
	}

	if ((flags&CL_MEM_READ_ONLY) || (flags&CL_MEM_WRITE_ONLY)) {
		printcl( CL_WARNING 
			"clmrealloc: CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY unsupported");
	} //// XXX CL_MEM_READ_WRITE implied -DAR


	intptr_t ptr = (intptr_t)p;

	intptr_t ptri;
	struct _memd_struct* memd;
	cl_uint memd_flags;


	if (ptr) {

		printcl( CL_DEBUG "ptr != 0");
		ptri = (intptr_t)ptr - sizeof(struct _memd_struct);
		memd = (struct _memd_struct*)ptri;
		memd_flags = memd->flags;

		if (__test_memd_corrupt(memd)) {
			printcl( CL_ERR "clmrealloc: memd corrupt");
			__set_clerrno(EFAULT);
			return(0);
		}

#ifndef DISABLE_CLGL
		if (memd_flags&__MEMD_F_GLBUF) {

			printcl( CL_ERR "clmrealloc: invalid ptr");

			return(0);
		}
#endif

/*
//		if ( (!memd->clbuf && (memd->flags&__MEMD_F_ATTACHED)) 
//			|| (memd->clbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {
		if ( (!memd->xxxclbuf && (memd->flags&__MEMD_F_ATTACHED)) 
			|| (memd->xxxclbuf && !(memd->flags&__MEMD_F_ATTACHED)) ) {

			printcl( CL_ERR "clmrealloc: memd corrupt");

			return(0);
		}
*/

		if (memd->flags&__MEMD_F_ATTACHED) {

/*
//			err = clReleaseMemObject(memd->clbuf);
			err = clReleaseMemObject(memd->xxxclbuf[0]);
			__set_oclerrno(err);
*/
			err = __release_mem_objects(memd);

			__set_clerrno(err);

			LIST_REMOVE(memd, memd_list);

		}

		ptri = (intptr_t)realloc((void*)ptri,size+sizeof(struct _memd_struct));

		printcl( CL_DEBUG "%p %d",ptri,size+sizeof(struct _memd_struct));

	} else {

		ptri = (intptr_t)malloc(size+sizeof(struct _memd_struct));

		memd_flags = __MEMD_F_RW;

	}


	ptr = ptri+sizeof(struct _memd_struct);
	memd = (struct _memd_struct*)ptri;
	printcl( CL_DEBUG "%p %p",ptr,memd);

	memd->magic = CLMEM_MAGIC;
	memd->flags = memd_flags;
	memd->sz = size;


	if (flags&CL_MEM_DETACHED) {
	
		printcl( CL_DEBUG "detached %p %p",ptr,memd);

//		memd->clbuf = (cl_mem)0;
		memd->nclbuf = 0;
		memd->xxxclbuf = 0;

		memd->flags &= ~(__MEMD_F_ATTACHED);

	} else {

/*
		memd->nclbuf = cp->nctx;

		if (memd->nclbuf * sizeof(cl_mem) > MEMD_FREE_SZ) 
			memd->xxxclbuf = (cl_mem*)malloc(memd->nclbuf * sizeof(cl_mem));
		else
			memd->xxxclbuf = (cl_mem*)&(memd->_free_offset);

		if (memd->flags&__MEMD_F_IMG2D) {

//			memd->clbuf = clCreateImage2D( cp->ctx, 
			memd->xxxclbuf[0] = clCreateImage2D( cp->xxxctx[0], 
				CL_MEM_READ_WRITE|memd->usrflags, &(memd->imgfmt), 
				memd->sz, memd->sz1, memd->sz2, NULL, &err);

			__set_oclerrno(err);

		} else {

//			memd->clbuf = clCreateBuffer( cp->ctx,
			memd->xxxclbuf[0] = clCreateBuffer( cp->xxxctx[0],
				CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR|memd->usrflags,
  	  	  		size,(void*)ptr,&err);

			__set_oclerrno(err);

		}

//		printcl( CL_DEBUG "clmrealloc: clCreateBuffer clbuf=%p",memd->clbuf);
		printcl( CL_DEBUG "clmrealloc: clCreateBuffer clbuf=%p",
			memd->xxxclbuf[0]);

		printcl( CL_DEBUG "clmrealloc: err from clCreateBuffer %d",err);
*/

		err = __create_mem_objects(memd,cp,(void*)ptr);

//		memd->flags |= __MEMD_F_ATTACHED;

//		if (!memd->clbuf) {
//		if (!memd->xxxclbuf[0]) {
		if (err) {

/*
			if (memd->xxxclbuf != (cl_mem*)&(memd->_free_offset)) {
				free(memd->xxxclbuf);
			}

			free((void*)ptri);

			ptr = 0;
*/

			printcl( CL_ERR "clmalloc: create mem objects failed" );

			__set_clerrno(-1);

//			if (memd->xxxclbuf != (cl_mem*)&(memd->_free_offset)) {
//				free(memd->xxxclbuf);
//			}
			__release_mem_objects(memd);

			free((void*)ptri);

			ptr = 0;

		} else {

			memd->flags |= __MEMD_F_ATTACHED;

			LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

		}

	}

	/* XXX why am i checking this now? -DAR */
	if (!__test_memd_magic((void*)ptr)) {

		printcl( CL_ERR "clmrealloc: invalid ptr");

		return(0);
	}

	return((void*)ptr);

}


#ifndef DISABLE_CLGL

LIBSTDCL_API
void* clglmalloc(
	CONTEXT* cp, cl_GLuint glbuf, cl_GLenum target, cl_GLint miplevel, int flags
)
{
	int i;

	printcl( CL_DEBUG "clglmalloc: glbuf=%d flag=%d",glbuf,flags);

	int err;

	unsigned int tmp_flags = 0;
	cl_mem tmp_clbuf = 0;

	
	if (flags&CL_MEM_DETACHED) {
		printcl( CL_WARNING "clglmalloc: invalid flag: CL_MEM_DETACHED");
		__set_clerrno(EINVAL);
		return(0);
	}

	if (cp->nctx > 1) {
		printcl( CL_WARNING "clglmalloc: super-contexts not yet supported");
		__set_clerrno(-1);
		return(0);
	}


	if (flags&CL_MEM_GLBUF) {

//		tmp_clbuf = clCreateFromGLBuffer( cp->ctx,CL_MEM_READ_WRITE, glbuf,&err);
		tmp_clbuf = clCreateFromGLBuffer( cp->xxxctx[0],CL_MEM_READ_WRITE, 
			glbuf,&err);

		__set_oclerrno(err);

	} else if (flags&CL_MEM_GLTEX2D) {

//		tmp_clbuf = clCreateFromGLTexture2D( cp->ctx,CL_MEM_READ_WRITE,
		tmp_clbuf = clCreateFromGLTexture2D( cp->xxxctx[0],CL_MEM_READ_WRITE,
			target,miplevel, glbuf,&err);

		__set_oclerrno(err);

		tmp_flags = __MEMD_F_RW|__MEMD_F_IMG2D|__MEMD_F_GLTEX2D|__MEMD_F_ATTACHED;

	} else if (flags&CL_MEM_GLTEX3D) {

//		tmp_clbuf = clCreateFromGLTexture3D( cp->ctx,CL_MEM_READ_WRITE,
		tmp_clbuf = clCreateFromGLTexture3D( cp->xxxctx[0],CL_MEM_READ_WRITE,
			target,miplevel, glbuf,&err);

		__set_oclerrno(err);

		tmp_flags = __MEMD_F_RW|__MEMD_F_IMG3D|__MEMD_F_GLTEX3D|__MEMD_F_ATTACHED;

	} else if (flags&CL_MEM_GLRBUF) {

//		tmp_clbuf = clCreateFromGLRenderbuffer( cp->ctx,CL_MEM_READ_WRITE,
		tmp_clbuf = clCreateFromGLRenderbuffer( cp->xxxctx[0],CL_MEM_READ_WRITE,
    	 	glbuf,&err);

		__set_oclerrno(err);

		tmp_flags = __MEMD_F_RW|__MEMD_F_IMG2D|__MEMD_F_GLRBUF|__MEMD_F_ATTACHED;

	} else { /* default case, consider deprecating to be safe -DAR */

//		printcl( CL_WARNING "clglmalloc: invalid flags");
//		tmp_clbuf = clCreateFromGLBuffer( cp->ctx,CL_MEM_READ_WRITE, glbuf,&err);
		tmp_clbuf = clCreateFromGLBuffer( cp->xxxctx[0],CL_MEM_READ_WRITE, 
			glbuf,&err);

		__set_oclerrno(err);

		return(0);

	}

	printcl( CL_DEBUG "clglmalloc: clCreateFromGL* clbuf=%p",tmp_clbuf);

	printcl( CL_DEBUG  "clglmalloc: err from clCreateFromGL* %d",err);

	size_t size;
	err = clGetMemObjectInfo(tmp_clbuf,CL_MEM_SIZE,sizeof(size_t),&size,0);
	__set_oclerrno(err);

	printcl( CL_DEBUG "clglmalloc: clGetMemObjectInfo err %d",err);

	if (size == 0) {

		printcl( CL_WARNING "clglmalloc: size=0, something went wrong");

		err = clReleaseMemObject(tmp_clbuf);

		__set_oclerrno(err);

		return(0);

	}

	intptr_t ptri = (intptr_t)malloc(size+sizeof(struct _memd_struct));

	if (ptri == 0) {

		printcl( CL_WARNING "clglmalloc: out of memory");

		err = clReleaseMemObject(tmp_clbuf);
		__set_oclerrno(err);

		return(0);

	}

	intptr_t ptr = ptri+sizeof(struct _memd_struct);
	struct _memd_struct* memd = (struct _memd_struct*)ptri;

	printcl( CL_DEBUG "clmalloc: ptri=%p ptr=%p memd=%p",ptri,ptr,memd);

	printcl( CL_DEBUG "clmalloc: sizeof struct _memd_struct %d",
		sizeof(struct _memd_struct));

	if ((flags&CL_MEM_READ_ONLY) || (flags&CL_MEM_WRITE_ONLY)) {
		printcl( CL_WARNING 
			"clmalloc: CL_MEM_READ_ONLY and CL_MEM_WRITE_ONLY unsupported");
	} //// XXX CL_MEM_READ_WRITE implied -DAR


	size_t sz = (cp->nctx + cp->ndev) * sizeof(cl_mem);

	memd->nclbuf = cp->nctx;

	if (sz > MEMD_FREE_SZ) 
		memd->xxxclbuf = (cl_mem*)malloc(sz);
	else
		memd->xxxclbuf = (cl_mem*)&(memd->_free_offset);

	memd->devbuf = memd->xxxclbuf + memd->nclbuf;

//	memd->clbuf = tmp_clbuf;
	memd->xxxclbuf[0] = tmp_clbuf;

	for(i=0; i < cp->ndev; i++) memd->devbuf[i] = tmp_clbuf;

	memd->magic = CLMEM_MAGIC;
	memd->flags = tmp_flags;
	memd->sz = size;


	/* XXX create mem objects */


	LIST_INSERT_HEAD(&cp->memd_listhead, memd, memd_list);

	return((void*)ptr);

}

LIBSTDCL_API
cl_event clglmsync(CONTEXT* cp, unsigned int devnum, void* ptr, int flags )
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
               printcl( CL_DEBUG "memd match");
					ptr = (void*)p1;
               break;
            }
      }
	}


	printcl( CL_DEBUG "clglmsync: memd = %p, base_ptr = %p",
		memd,(intptr_t)memd+sizeof(struct _memd_struct));


	printcl( CL_WARNING "clglmsync: no supp for dev/host sync yet");

#ifdef _WIN64
	__cmdq_create(cp,devnum);
#endif

	if (flags&CL_MEM_CLBUF) {

//		err = clEnqueueAcquireGLObjects( cp->cmdq[devnum],1,&(memd->clbuf),
//		err = clEnqueueAcquireGLObjects( cp->cmdq[devnum],1,&(memd->xxxclbuf[0]),
		err = clEnqueueAcquireGLObjects( cp->cmdq[devnum], 1,
			&(memd->devbuf[devnum]), 0,0,&ev);

		__set_oclerrno(err);

		printcl( CL_DEBUG "clglmsync: clEnqueueAcquireGLObjects err %d",err);

	} else if (flags&CL_MEM_GLBUF) {

//		err = clEnqueueReleaseGLObjects( cp->cmdq[devnum],1,&(memd->clbuf),
//		err = clEnqueueReleaseGLObjects( cp->cmdq[devnum],1,&(memd->xxxclbuf[0]),
		err = clEnqueueReleaseGLObjects( cp->cmdq[devnum],1,
			&(memd->devbuf[devnum]), 0,0,&ev);

		__set_oclerrno(err);

		printcl( CL_DEBUG "clglmsync: clEnqueueReleaseGLObjects err %d",err);
	
	} else {

		printcl( CL_WARNING "clglmsync: invalid flag, did nothing");

		return((cl_event)0);

	}


	if (flags & CL_EVENT_NOWAIT) {

		cp->mev[devnum].ev[cp->mev[devnum].ev_free++] = ev;
		cp->mev[devnum].ev_free %= STDCL_EVENTLIST_MAX;
		++cp->mev[devnum].nev;

	} else { /* CL_EVENT_WAIT */

		err = clWaitForEvents(1,&ev);

		__set_oclerrno(err);

		if ( !(flags & CL_EVENT_NORELEASE) ) {

			err = clReleaseEvent(ev);

			__set_oclerrno(err);

			ev = (cl_event)0;

		}

	}

	return(ev);

}

#endif

