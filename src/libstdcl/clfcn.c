/* clfcn.c
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "CL/cl.h"
//#include "stdcl.h"
#include "clfcn.h"
#include "util.h"


/*
 * NOTES:
 * 	-Right now one-to-one mapping between clopen() and clclose() calls,
 * 	 should extend to use ref count.
 *
 *		-Supports simple CL build program from source semantic, should extend.
 *
 */


void* 
clload( CONTEXT* cp, void* ptr, size_t len, int flags )
{
	int n;
	int err;

	DEBUG(__FILE__,__LINE__," checking cp ");

	if (!cp) return(0);

	DEBUG(__FILE__,__LINE__," cp ok ");


	struct _prgs_struct* prgs
		= (struct _prgs_struct*)malloc(sizeof(struct _prgs_struct));

	prgs->fname = 0;
	prgs->fd = -1;
	prgs->len = len;
	prgs->ptr = ptr;
	prgs->refc = 1;

	LIST_INSERT_HEAD(&cp->prgs_listhead, prgs, prgs_list);


	struct _txt_struct* txt
		= (struct _txt_struct*)malloc(sizeof(struct _txt_struct));
	
	txt->prgs = prgs;
	txt->krn = 0;

	txt->prg = clCreateProgramWithSource(
		cp->ctx,1,(const char**)&ptr,&len,&err
	);
	DEBUG(__FILE__,__LINE__,"clload: err from clCreateProgramWithSource %d",err);

	LIST_INSERT_HEAD(&cp->txt_listhead, txt, txt_list);



	if (!(flags&CLLD_NOBUILD)) {
		DEBUG(__FILE__,__LINE__,"clload: calling clbuild");
		clbuild(cp,prgs,0,flags);
	}


//	LIST_INSERT_HEAD(&cp->txt_listhead, txt, txt_list);

	
	return((void*)prgs);

}


void* 
clbuild( CONTEXT* cp, void* handle, char* uopts, int flags )
{
	int n;
	int err;
	struct _prgs_struct* prgs;

	DEBUG(__FILE__,__LINE__," checking cp ");

	if (!cp) return(0);

	DEBUG(__FILE__,__LINE__," cp ok ");

	prgs = (struct _prgs_struct*)handle;

	DEBUG(__FILE__,__LINE__,"prgs %p",prgs);

	struct _txt_struct* txt;
   for (
      txt = cp->txt_listhead.lh_first; txt != 0;
      txt = txt->txt_list.le_next
   ) {
		if (prgs==0 || txt->prgs == prgs) break;
   }

	if (!txt) {
		DEBUG(__FILE__,__LINE__,"clbuild: bad handle");
		return((void*)-1);
	}

//// begin compile

	char opts[1024] = "-D __STDCL__";

	if (cp==stdcpu) {
		strcat(opts," -D __CPU__");
	} else if (cp==stdgpu) {	
		strcat(opts," -D __GPU__");
	}

	if (!strcasecmp(cp->platform_vendor,"Advanced Micro Devices, Inc.")) 
		strcat(opts," -D __AMD__");
	else if (!strcasecmp(cp->platform_vendor,"NVIDIA Corporation")) 
		strcat(opts," -D __NVIDIA__");
	else if (!strcasecmp(cp->platform_vendor,"Brown Deer Technology, LLC.")) 
		strcat(opts," -D __coprthr__");

	char cwd[1024];
	if ( getcwd(cwd,1024) ) {
		n = strnlen(cwd,1024);
		if (n-strnlen(opts,1024)>1022) {
			WARN(__FILE__,__LINE__,
				"clbuild: cwd too long to include as header search path.");
		} else {
			strcat(opts," -I");
			strncat(opts,cwd,n);
		}
	}

	if (uopts) {

		n = strnlen(uopts,64);
		DEBUG(__FILE__,__LINE__,"%d",n);
		if (n-strnlen(opts,1024)>1022) {
			WARN(__FILE__,__LINE__,
				"clbuild: options string too long, ignoring it.");
		} else {
			strcat(opts," ");
			strncat(opts,uopts,n);
		}

	}
	DEBUG(__FILE__,__LINE__,"%s",opts);

	err = clBuildProgram(txt->prg,cp->ndev,cp->dev,opts,0,0);
	DEBUG(__FILE__,__LINE__,"clbuild: err from clBuildProgram %d",err);

	{
//	char buf[256];
//	clGetProgramBuildInfo(txt->prg,cp->dev[0],CL_PROGRAM_BUILD_LOG,256,&buf,0);
//	DEBUG(__FILE__,__LINE__,"clld: log from clBuildProgram %s",buf);
//	clGetProgramBuildInfo(txt->prg,cp->dev[0],CL_PROGRAM_BUILD_OPTIONS,256,&buf,0);
//	DEBUG(__FILE__,__LINE__,"clbuild: log from clBuildProgram %s",buf);

	size_t sz;
	clGetProgramBuildInfo(txt->prg,cp->dev[0],CL_PROGRAM_BUILD_LOG,0,0,&sz);
	char* buf = (char*)malloc(sz*sizeof(char));
	clGetProgramBuildInfo(txt->prg,cp->dev[0],CL_PROGRAM_BUILD_LOG,sz,buf,0);

	DEBUG(__FILE__,__LINE__,"clld: log from clBuildProgram %s",buf);

	if (buf) free(buf);

	}

	err = clCreateKernelsInProgram(txt->prg,0,0,&txt->nkrn);
	DEBUG(__FILE__,__LINE__,"clbuild: NUMBER OF KERNELS %d",txt->nkrn);
	txt->krn = (cl_kernel*)malloc(sizeof(cl_kernel)*txt->nkrn);
	err = clCreateKernelsInProgram(txt->prg,txt->nkrn,txt->krn,0);
	txt->krntab = (struct _krntab_struct*)malloc(
		sizeof(struct _krntab_struct)*txt->nkrn
	);

	for(n=0;n<txt->nkrn;n++) {
		clGetKernelInfo(
			txt->krn[n],CL_KERNEL_FUNCTION_NAME,256,txt->krntab[n].kname,0
		);
		clGetKernelInfo(
			txt->krn[n],CL_KERNEL_NUM_ARGS,sizeof(cl_uint),&txt->krntab[n].nargs,0
		);
		clGetKernelInfo(
			txt->krn[n],CL_KERNEL_REFERENCE_COUNT,sizeof(cl_uint),
			&txt->krntab[n].refc,0
		);
		clGetKernelInfo(
			txt->krn[n],CL_KERNEL_CONTEXT,sizeof(cl_context),&txt->krntab[n].kctx,0
		);
		clGetKernelInfo(
			txt->krn[n],CL_KERNEL_PROGRAM,sizeof(cl_program),&txt->krntab[n].kprg,0
		);
		DEBUG(__FILE__,__LINE__,"clbuild: %s %d %p %p\n",
			txt->krntab[n].kname,txt->krntab[n].nargs,
			txt->krntab[n].kctx,txt->krntab[n].kprg);
	}

	return(0);

}


void* 
clopen( CONTEXT* cp, const char* fname, int flags )
{
	int n;
	int err;
	struct stat fs;
	int fd;
	size_t len;
	void* ptr;
	struct _prgs_struct* prgs;

DEBUG(__FILE__,__LINE__," checking cp ");

	if (!cp) return(0);

DEBUG(__FILE__,__LINE__," cp ok ");

	if (!fname) {

		DEBUG(__FILE__,__LINE__," fname null, search _proc_cl");

		fd = -1;
		ptr = 0;
		len = 0;

		DEBUG(__FILE__,__LINE__,"clopen: %p searching _proc_cl for prgs...",
			_proc_cl.clstrtab);

 		DEBUG(__FILE__,__LINE__,"clopen: %s",&_proc_cl.clstrtab[1]);

		struct clprgs_entry* sp;
		for(n=0,sp=_proc_cl.clprgs;n<_proc_cl.clprgs_n;n++,sp++) {
			DEBUG(__FILE__,__LINE__,"found %s (%d bytes)\n",
				&_proc_cl.clstrtab[sp->e_name],sp->e_size);
			ptr = _proc_cl.cltexts+sp->e_offset;
			len = sp->e_size;
		
			prgs = clload(cp,ptr,len,flags);
			prgs->fname = fname;
			prgs->fd = fd;
		}

//		prgs = clload(cp,ptr,len,flags);
//		prgs->fname = fname;
//		prgs->fd = fd;
		
		return(0);

	} else {

		if (stat(fname,&fs)) return(0);

		DEBUG(__FILE__,__LINE__," stat ok ");
		DEBUG(__FILE__,__LINE__," file size = %d ",fs.st_size);

		if (fs.st_size==0 || !S_ISREG(fs.st_mode)) return(0);

		/* XXX any more checks that should be done? -DAR */

		DEBUG(__FILE__,__LINE__," file ok ");

		len = fs.st_size;

		if ((fd = open(fname,O_RDONLY)) == -1) return(0); 

		DEBUG(__FILE__,__LINE__," file open ");

		if (!(ptr = mmap(0,len,PROT_READ,MAP_SHARED,fd,0))) {
			close(fd);
			return(0);
		}

		prgs = clload(cp,ptr,len,flags);
		prgs->fname = fname;
		prgs->fd = fd;

		return((void*)prgs);
	}

}


int 
clclose(CONTEXT* cp, void* handle)
{
	int n;
	int err;

	if (!handle) return(-1);

	struct _prgs_struct* prgs = (struct _prgs_struct*)handle;


	/* XXX search all txt to release resources.  NOTE refc is ignored. -DAR */

	struct _txt_struct* txt;
//	struct _txt_struct* txt_next;
	struct _txt_struct* txt_next = 0;
	txt = cp->txt_listhead.lh_first;
	while(txt) {
		txt_next;
		if (txt->prgs == prgs) {
			DEBUG(__FILE__,__LINE__," removing txt from list\n");
			LIST_REMOVE(txt, txt_list);
			for(n=0;n<txt->nkrn;n++) {
				err = clReleaseKernel(txt->krn[n]);
			}
			free(txt->krn);
			free(txt->krntab);
			err = clReleaseProgram(txt->prg);
			free(txt);
			txt = 0;
		}
		txt = txt_next;
	}



	DEBUG(__FILE__,__LINE__," removing prgs from list\n");
	LIST_REMOVE(prgs, prgs_list);

	if (prgs->fd >= 0) close(prgs->fd);

	free(prgs);

	return(0);

}



cl_kernel 
clsym( CONTEXT* cp, void* handle, const char* sname, int flags )
{
	int n;
	struct _prgs_struct* prgs;

	if (!sname) return(0);

//	if (!handle) {
//		ERROR(__FILE__,__LINE__,"null handle not supported");
//		return(0);
//	}

	prgs = (struct _prgs_struct*)handle;

	cl_kernel krn;
	char buf[256];
	size_t sz;

	struct _txt_struct* txt;
   for (
      txt = cp->txt_listhead.lh_first; txt != 0;
      txt = txt->txt_list.le_next
   ) {
		if (prgs==0 || txt->prgs == prgs) {
			DEBUG(__FILE__,__LINE__,"clsym: txt->krn %p",txt->krn);
			DEBUG(__FILE__,__LINE__,"clsym: searching kernels ...");
			for(n=0;n<txt->nkrn;n++) {

				DEBUG(__FILE__,__LINE__,"checking |%s|%s|",
					txt->krntab[n].kname,sname);

				if (!strncmp(sname,txt->krntab[n].kname,256)) return(txt->krn[n]);

			}
		}
   }

	DEBUG(__FILE__,__LINE__,"clsym: symbol not found");

	return((cl_kernel)0);

}


char* clerror(void)
{
	return(0);
}



void* 
clsopen( CONTEXT* cp, const char* srcstr, int flags )
{
	int n;
	int err;
	size_t len;
	void* ptr;
	struct _prgs_struct* prgs;

DEBUG(__FILE__,__LINE__," checking cp ");

	if (!cp) return(0);

DEBUG(__FILE__,__LINE__," cp ok ");

	if (srcstr) {

		ptr = (void*)srcstr;
		len = strlen(srcstr);

		prgs = clload(cp,ptr,len,flags);
		prgs->fname = 0;
		prgs->fd = -1;

		return((void*)prgs);
	}

}



