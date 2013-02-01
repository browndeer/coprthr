/* clfcn.c
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
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>


#include "CL/cl.h"

#define __STDCL__
#include "clfcn.h"
#include "util.h"
#include "printcl.h"
#include "clerrno.h"

/*
 * NOTES:
 * 	-Right now one-to-one mapping between clopen() and clclose() calls,
 * 	 should extend to use ref count.
 *
 *		-Supports simple CL build program from source semantic, should extend.
 *
 */


#define __inc_idev(cp,idev) \
		while( idev < cp->ndev && cp->xxxctx[ictx] == cp->devctx[idev++])

LIBSTDCL_API void* 
clload( CONTEXT* cp, void* ptr, size_t len, int flags )
{
	int n;
	int err;

	printcl( CL_DEBUG " checking cp ");

	if (!cp) return(0);

	printcl( CL_DEBUG " cp ok ");


	struct _prgs_struct* prgs
		= (struct _prgs_struct*)malloc(sizeof(struct _prgs_struct));

	prgs->fname = 0;
#ifdef _WIN64
	prgs->fp = 0;
#else
	prgs->fd = -1;
#endif
	prgs->len = len;
	prgs->ptr = ptr;
	prgs->refc = 1;

	LIST_INSERT_HEAD(&cp->prgs_listhead, prgs, prgs_list);


	struct _txt_struct* txt
		= (struct _txt_struct*)malloc(sizeof(struct _txt_struct));
	
	txt->prgs = prgs;
	txt->krn = 0;

/*
	txt->prg = clCreateProgramWithSource(
//		cp->ctx,1,(const char**)&ptr,&len,&err
		cp->xxxctx[0],1,(const char**)&ptr,&len,&err
	);
	printcl( CL_DEBUG "clload: err from clCreateProgramWithSource %d",err);
*/

	txt->nprg = cp->nctx;

	txt->xxxprg = (cl_program*)malloc(txt->nprg * sizeof(cl_program));

	int ictx;
	for(ictx=0; ictx < cp->nctx; ictx++) {
		txt->xxxprg[ictx] = clCreateProgramWithSource( cp->xxxctx[ictx],
			1,(const char**)&ptr,&len,&err );
		printcl( CL_DEBUG "clload: err from clCreateProgramWithSource %d",err);
	}

	LIST_INSERT_HEAD(&cp->txt_listhead, txt, txt_list);


	if (!(flags&CLLD_NOBUILD)) {
		printcl( CL_DEBUG "clload: calling clbuild");
		clbuild(cp,prgs,0,flags);
	}


//	LIST_INSERT_HEAD(&cp->txt_listhead, txt, txt_list);

	
	return((void*)prgs);

}


#define __txt_get_krn(txt,n,ictx) ((cl_kernel*)(txt)->krn[(n)])[(ictx)]

LIBSTDCL_API void* 
clloadb( CONTEXT* cp, int nbin, char** bin, size_t* bin_sz, int flags )
{
	int n;
	int err;
	int ictx,idev;

	printcl( CL_DEBUG "clloadb: checking cp ");

	if (!cp) return(0);

	printcl( CL_DEBUG "clloadb: cp ok ");


	struct _prgs_struct* prgs
		= (struct _prgs_struct*)malloc(sizeof(struct _prgs_struct));

	prgs->fname = 0;
#ifdef _WIN64
	prgs->fp = 0;
#else
	prgs->fd = -1;
#endif
//	prgs->len = len;
//	prgs->ptr = ptr;
	prgs->nbin = nbin;
	prgs->bin = bin;
	prgs->bin_sz = bin_sz;
	prgs->refc = 1;

	LIST_INSERT_HEAD(&cp->prgs_listhead, prgs, prgs_list);


	struct _txt_struct* txt
		= (struct _txt_struct*)malloc(sizeof(struct _txt_struct));
	
	txt->prgs = prgs;
	txt->krn = 0;

//	txt->prg = clCreateProgramWithSource(
//		cp->ctx,1,(const char**)&ptr,&len,&err
//	);

	cl_int* bin_stat = (cl_int*)calloc(sizeof(cl_int),cp->ndev);

/*
//	txt->prg = clCreateProgramWithBinary(cp->ctx,cp->ndev,cp->dev,
	txt->prg = clCreateProgramWithBinary(cp->xxxctx[0],cp->ndev,cp->dev,
		bin_sz,(const unsigned char**)bin,bin_stat,&err);
	__set_oclerrno(err);
	printcl( CL_DEBUG "clloadb: err from clCreateProgramWithBinary %d (%p)",
		err,txt->prg);
*/

	txt->nprg = cp->nctx;

	txt->xxxprg = (cl_program*)malloc(txt->nprg * sizeof(cl_program));

	idev = 0;
	for(ictx=0; ictx < cp->nctx; ictx++) {

		int idev0 = idev;
		__inc_idev(cp,idev);

//		printcl( CL_DEBUG "%d %d %p",idev0,idev,cp->xxxctx[ictx]);

		txt->xxxprg[ictx] = clCreateProgramWithBinary(cp->xxxctx[ictx],
			idev-idev0,cp->dev+idev0,
			bin_sz+idev0,((const unsigned char**)bin)+idev0,bin_stat+idev0,&err);

		__set_oclerrno(err);

		printcl( CL_DEBUG "clloadb: err from clCreateProgramWithBinary %d (%p)",
			err,txt->xxxprg[ictx]);
	}


	for(n=0; n<cp->ndev; n++) {
		printcl( CL_DEBUG "clloadb: bin stat [%d] %d",n,bin_stat[n]);
	}
	printcl( CL_DEBUG "clloadb: err from clCreateProgramWithBinary %d",err);



	LIST_INSERT_HEAD(&cp->txt_listhead, txt, txt_list);



//	if (!(flags&CLLD_NOBUILD)) {
//		printcl( CL_DEBUG "clload: calling clbuild");
//		clbuild(cp,prgs,0,flags);
//	}

	idev = 0;
	for(ictx=0; ictx < cp->nctx; ictx++) {

		int idev0 = idev;
		__inc_idev(cp,idev);

		err = clBuildProgram(txt->xxxprg[ictx],idev-idev0,cp->dev+idev0 ,0,0,0);

		__set_oclerrno(err);
	
		printcl( CL_DEBUG "clloadb: err from clBuildProgram %d",err);
	}


//	LIST_INSERT_HEAD(&cp->txt_listhead, txt, txt_list);


	/* extract the kernels */

   err = clCreateKernelsInProgram(txt->xxxprg[0],0,0,&txt->nkrn);
   __set_oclerrno(err);
   printcl( CL_DEBUG "clloadb: err from clCreateKernelsInProgram %d",err);
   printcl( CL_DEBUG "clloadb: NUMBER OF KERNELS %d",txt->nkrn);

   txt->krn = (cl_kernel*)malloc(sizeof(cl_kernel)*txt->nkrn);

   err = clCreateKernelsInProgram(txt->xxxprg[0],txt->nkrn,txt->krn,0);
   __set_oclerrno(err);


	txt->krntab = (struct _krntab_struct*)malloc(
		sizeof(struct _krntab_struct)*txt->nkrn);

	for(n=0;n<txt->nkrn;n++) {

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_FUNCTION_NAME,256,
			txt->krntab[n].kname,0);
		__set_oclerrno(err);

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_NUM_ARGS,sizeof(cl_uint),
			&txt->krntab[n].nargs,0);
		__set_oclerrno(err);

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_REFERENCE_COUNT,
			sizeof(cl_uint), &txt->krntab[n].refc,0);
		__set_oclerrno(err);

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_CONTEXT,sizeof(cl_context),
			&txt->krntab[n].kctx,0);
		__set_oclerrno(err);

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_PROGRAM,sizeof(cl_program),
			&txt->krntab[n].kprg,0);
		__set_oclerrno(err);

		printcl( CL_DEBUG "clloadb: %s %d %p %p\n",
			txt->krntab[n].kname,txt->krntab[n].nargs,
			txt->krntab[n].kctx,txt->krntab[n].kprg);

	}


   if (cp->nctx > 1) {

      for(n=0;n<txt->nkrn;n++) {
         cl_kernel krn = txt->krn[n];
         txt->krn[n] = (cl_kernel)calloc(cp->nctx,sizeof(cl_kernel));
         *(cl_kernel*)txt->krn[n] = krn;
      }

		cl_uint nkrn;
   	cl_kernel* krn = (cl_kernel*)malloc(sizeof(cl_kernel)*txt->nkrn);

      for(ictx=1; ictx < cp->nctx; ictx++) {

			err = clCreateKernelsInProgram(txt->xxxprg[ictx],txt->nkrn,krn,&nkrn);

			__set_oclerrno(err);

      	for(n=0;n<txt->nkrn;n++) {

//         	((cl_kernel*)txt->krn[n])[ictx] = krn[n];
				__txt_get_krn(txt,n,ictx) = krn[n];

			}		
	
			
			/* XXX here we just check consistency of kernels -DAR */

			struct _krntab_struct kinfo;

      	for(n=0;n<txt->nkrn;n++) {

				cl_kernel krn = __txt_get_krn(txt,n,ictx);

				err = clGetKernelInfo( krn, CL_KERNEL_FUNCTION_NAME,256, 
					kinfo.kname,0);

				__set_oclerrno(err);

				if (strcmp(kinfo.kname,txt->krntab[n].kname)) 
					printcl( CL_ERR "clloadb: kernel info mismatch" );				

				err = clGetKernelInfo( krn,CL_KERNEL_NUM_ARGS,sizeof(cl_uint),
					&kinfo.nargs,0);

				__set_oclerrno(err);

				if (kinfo.nargs != txt->krntab[n].nargs) 
					printcl( CL_ERR "clloadb: kernel info mismatch" );

				err = clGetKernelInfo( krn,CL_KERNEL_REFERENCE_COUNT,
					sizeof(cl_uint), &kinfo.refc,0);

				__set_oclerrno(err);

				if (kinfo.refc != txt->krntab[n].refc) 
					printcl( CL_ERR "clloadb: kernel info mismatch" );

				err = clGetKernelInfo( krn,CL_KERNEL_CONTEXT,sizeof(cl_context),
					&kinfo.kctx,0);

				__set_oclerrno(err);

				if (kinfo.kctx != txt->krntab[n].kctx)
					printcl( CL_ERR "clloadb: kernel info mismatch" );

				err = clGetKernelInfo( krn,CL_KERNEL_PROGRAM,sizeof(cl_program),
					&kinfo.kprg,0);

				__set_oclerrno(err);

				if (kinfo.kprg != txt->krntab[n].kprg) 
					printcl( CL_ERR "clloadb: kernel info mismatch" );

			}		
      }
   }

	
	return((void*)prgs);

}


static int _clbuild_zero = 0; 

LIBSTDCL_API void* 
clbuild( CONTEXT* cp, void* handle, char* uopts, int flags )
{
	int n;
	int err;
	int ictx, idev;
	struct _prgs_struct* prgs;

	printcl( CL_DEBUG " checking cp ");

	if (!cp) return(0);

	if (_clbuild_zero) return(0);

	printcl( CL_DEBUG " cp ok ");

	prgs = (struct _prgs_struct*)handle;

	printcl( CL_DEBUG "prgs %p",prgs);

	struct _txt_struct* txt;
   for (
      txt = cp->txt_listhead.lh_first; txt != 0;
      txt = txt->txt_list.le_next
   ) {
//		if (prgs==0 || txt->prgs == prgs) break;
//   }

//	if (!txt) {
//		printcl( CL_DEBUG "clbuild: bad handle");
//		return((void*)-1);
//	}

	if ( (prgs==0 && txt->prgs->fname==0) || txt->prgs == prgs) {

//// begin compile

//	char opts[1024] = "-D __STDCL__";
	char* opts = (char*)malloc(1024);
	strcpy(opts,"-D __STDCL__");

	if (cp==stdcpu) {
		strcat(opts," -D __CPU__");
	} else if (cp==stdgpu) {	
		strcat(opts," -D __GPU__");
	}

	printcl( CL_DEBUG "clbuild: platform_vendor '%s'",cp->platform_vendor);

	if (!strcasecmp(cp->platform_vendor,"Advanced Micro Devices, Inc.")) 
		strcat(opts," -D __AMD__");
	else if (!strcasecmp(cp->platform_vendor,"NVIDIA Corporation")) 
		strcat(opts," -D __NVIDIA__");
	else if (!strcasecmp(cp->platform_vendor,"Brown Deer Technology, LLC.")) 
		strcat(opts," -D __coprthr__");

//printf("opts |%s|\n",opts);
#ifdef DEFAULT_STDCL_INCLUDE
	strcat(opts," -I" DEFAULT_STDCL_INCLUDE " -D __stdcl_kernel__ ");
#endif
	strcat(opts,"\0");

//printf("opts |%s|\n",opts);


/* XXX path returned by getcwd() is useless on Windows */
#ifndef _WIN64
//	char cwd[1024];
	char* cwd = (char*)malloc(1024);
	if ( getcwd(cwd,1024) ) {
		n = strnlen(cwd,1024);
//		printf("len of cwd %d\n",n);
//		printf("len of opts %d\n",strnlen(opts,1024));
		if (n+strnlen(opts,1024)>1022) {
//			printf("opts |%s|\n",opts);
			printcl( CL_WARNING 
				"clbuild: cwd too long to include as header search path.");
		} else {
			strcat(opts," -I");
			strncat(opts,cwd,n);
		}
	}

	if (cwd) free(cwd);
#endif

	if (uopts) {

//		n = strnlen(uopts,64);
		n = strnlen(uopts,1024);
		printcl( CL_DEBUG "%d",n);
		if (n+strnlen(opts,1024)>1022) {
			printcl( CL_WARNING "clbuild: options string too long, ignoring it.");
		} else {
			strcat(opts," ");
			strncat(opts,uopts,n);
		}

	}
	printcl( CL_DEBUG "%s",opts);


/*
	err = clBuildProgram(txt->prg,cp->ndev,cp->dev,opts,0,0);
	__set_oclerrno(err);
	printcl( CL_DEBUG "clbuild: err from clBuildProgram %d",err);
*/

	idev = 0;
	for(ictx=0; ictx < cp->nctx; ictx++) {

		int idev0 = idev;
		__inc_idev(cp,idev);

		err = clBuildProgram(txt->xxxprg[ictx],idev-idev0,cp->dev+idev0,
			opts,0,0);

		__set_oclerrno(err);
	
		printcl( CL_DEBUG "clbuild: err from clBuildProgram %d",err);
	}





	if (opts) free(opts);



//	char buf[256];
//	clGetProgramBuildInfo(txt->prg,cp->dev[0],CL_PROGRAM_BUILD_LOG,256,&buf,0);
//	printcl( CL_DEBUG "clld: log from clBuildProgram %s",buf);
//	clGetProgramBuildInfo(txt->prg,cp->dev[0],CL_PROGRAM_BUILD_OPTIONS,256,&buf,0);
//	printcl( CL_DEBUG "clbuild: log from clBuildProgram %s",buf);

/* XXX THIS IS CAUSING/EXPOSING MEMORY CORRUPTION ISSUE -DAR
	idev = 0;
	for(ictx=0; ictx < cp->nctx; ictx++) {

		int idev0 = idev;
		__inc_idev(cp,idev);

		size_t sz;

		err = clGetProgramBuildInfo(
			txt->xxxprg[ictx],cp->dev[idev0],CL_PROGRAM_BUILD_LOG,0,0,&sz);
	
		__set_oclerrno(err);

		char* buf = (char*)malloc(sz*sizeof(char));

		err = clGetProgramBuildInfo(
			txt->xxxprg[ictx],cp->dev[idev0],CL_PROGRAM_BUILD_LOG,sz,buf,0);

		__set_oclerrno(err);

		printcl( CL_DEBUG "clld: log from clBuildProgram %s",buf);

		if (buf) free(buf);

	}
*/


	err = clCreateKernelsInProgram(txt->xxxprg[0],0,0,&txt->nkrn);
	__set_oclerrno(err);
	printcl( CL_DEBUG "clbuild: NUMBER OF KERNELS %d",txt->nkrn);

	txt->krn = (cl_kernel*)malloc(sizeof(cl_kernel)*txt->nkrn);

	err = clCreateKernelsInProgram(txt->xxxprg[0],txt->nkrn,txt->krn,0);
	__set_oclerrno(err);

	txt->krntab = (struct _krntab_struct*)malloc(
		sizeof(struct _krntab_struct)*txt->nkrn );

	for(n=0;n<txt->nkrn;n++) {

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_FUNCTION_NAME,256,
			txt->krntab[n].kname,0);
		__set_oclerrno(err);

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_NUM_ARGS,sizeof(cl_uint),
			&txt->krntab[n].nargs,0);
		__set_oclerrno(err);

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_REFERENCE_COUNT,
			sizeof(cl_uint), &txt->krntab[n].refc,0);
		__set_oclerrno(err);

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_CONTEXT,sizeof(cl_context),
			&txt->krntab[n].kctx,0);
		__set_oclerrno(err);

		err = clGetKernelInfo( txt->krn[n],CL_KERNEL_PROGRAM,sizeof(cl_program),
			&txt->krntab[n].kprg,0);
		__set_oclerrno(err);

		printcl( CL_DEBUG "clbuild: %s %d %p %p\n",
			txt->krntab[n].kname,txt->krntab[n].nargs,
			txt->krntab[n].kctx,txt->krntab[n].kprg);

	}


   if (cp->nctx > 1) {

      for(n=0;n<txt->nkrn;n++) {
         cl_kernel krn = txt->krn[n];
         txt->krn[n] = (cl_kernel)calloc(cp->nctx,sizeof(cl_kernel));
         *(cl_kernel*)txt->krn[n] = krn;
      }

		cl_uint nkrn;
   	cl_kernel* krn = (cl_kernel*)malloc(sizeof(cl_kernel)*txt->nkrn);

      for(ictx=1; ictx < cp->nctx; ictx++) {

			err = clCreateKernelsInProgram(txt->xxxprg[ictx],txt->nkrn,krn,&nkrn);

			__set_oclerrno(err);

      	for(n=0;n<txt->nkrn;n++) {

//         	((cl_kernel*)txt->krn[n])[ictx] = krn[n];
				__txt_get_krn(txt,n,ictx) = krn[n];

			}		
	
			
			/* XXX here we just check consistency of kernels -DAR */

			struct _krntab_struct kinfo;

      	for(n=0;n<txt->nkrn;n++) {

				cl_kernel krn = __txt_get_krn(txt,n,ictx);

				err = clGetKernelInfo( krn, CL_KERNEL_FUNCTION_NAME,256, 
					kinfo.kname,0);

				__set_oclerrno(err);

				if (strcmp(kinfo.kname,txt->krntab[n].kname)) 
					printcl( CL_ERR "clloadb: kernel info mismatch" );				

				err = clGetKernelInfo( krn,CL_KERNEL_NUM_ARGS,sizeof(cl_uint),
					&kinfo.nargs,0);

				__set_oclerrno(err);

				if (kinfo.nargs != txt->krntab[n].nargs) 
					printcl( CL_ERR "clloadb: kernel info mismatch" );

				err = clGetKernelInfo( krn,CL_KERNEL_REFERENCE_COUNT,
					sizeof(cl_uint), &kinfo.refc,0);

				__set_oclerrno(err);

				if (kinfo.refc != txt->krntab[n].refc) 
					printcl( CL_ERR "clloadb: kernel info mismatch" );

				err = clGetKernelInfo( krn,CL_KERNEL_CONTEXT,sizeof(cl_context),
					&kinfo.kctx,0);

				__set_oclerrno(err);

				if (kinfo.kctx != txt->krntab[n].kctx)
					printcl( CL_ERR "clloadb: kernel info mismatch" );

				err = clGetKernelInfo( krn,CL_KERNEL_PROGRAM,sizeof(cl_program),
					&kinfo.kprg,0);

				__set_oclerrno(err);

				if (kinfo.kprg != txt->krntab[n].kprg) 
					printcl( CL_ERR "clloadb: kernel info mismatch" );

			}		
      }
   }

	}

	}

	if (prgs==0) _clbuild_zero = 1;

	return(0);

}


static int _clopen_zero = 0;

LIBSTDCL_API void* 
clopen( CONTEXT* cp, const char* fname, int flags )
{
	int n;
	int err;
	struct stat fs;
#ifdef _WIN64
	FILE* fp;
#else
	int fd;
#endif
	size_t len;
	void* ptr;
	struct _prgs_struct* prgs;

printcl( CL_DEBUG " checking cp ");

	if (!cp) return(0);

printcl( CL_DEBUG " cp ok ");

	if (!fname) {

		if (_clopen_zero) {
			printcl( CL_DEBUG "already here");
			return(0);
		}

#ifdef _WIN64

		printcl( CL_WARNING "embedded kernels not supported for Windows");

#else

		printcl( CL_DEBUG " fname null, search _proc_clelf_sect");

		if (!_proc_clelf_sect) {
			printcl( CL_DEBUG "clopen: _proc_clelf_sect is null");
			return(0);
		}

		struct clelf_sect_struct* sect = _proc_clelf_sect;

		if (!sect->has_any_clelf_section) {
			printcl( CL_DEBUG "clopen: no CLELF sections found");
			return(0);
		}

		fd = -1;
		ptr = 0;
		len = 0;

//		printcl( CL_DEBUG "clopen: %p searching _proc_cl for prgs...",
//			_proc_cl.clstrtab);
//
// 		printcl( CL_DEBUG "clopen: %s",&_proc_cl.clstrtab[1]);
//
//		printcl( CL_DEBUG "clopen: _proc_cl.clprgs_n=%d",_proc_cl.clprgs_n);
//
//		struct clprgs_entry* sp;
//		for(n=0,sp=_proc_cl.clprgs;n<_proc_cl.clprgs_n;n++,sp++) {
//			printcl( CL_DEBUG "found %s (%d bytes)\n",
//				&_proc_cl.clstrtab[sp->e_name],sp->e_size);
//			ptr = _proc_cl.cltexts+sp->e_offset;
//			len = sp->e_size;
//		
//			prgs = clload(cp,ptr,len,flags);
//			prgs->fname = fname;
//			prgs->fd = fd;
//		}

		printcl( CL_DEBUG "clopen: need %d devices for binary kernels",cp->ndev);
		int m;
		struct cldev_info* di
			= (struct cldev_info*)malloc(cp->ndev * sizeof(struct cldev_info));
		clgetdevinfo(cp,di);
		char** dev_alias = (char**)malloc(cp->ndev * sizeof(char*));
		for(m=0; m<cp->ndev; m++) {
			dev_alias[m] = (char*)malloc(1+strlen(di[m].dev_name));
			strcpy(dev_alias[m],di[m].dev_name);
			if ( clelf_device_name_alias(dev_alias[m]) ) {
				printcl( CL_DEBUG "clopen: dev[%d] name |%s| (%s)",
					m,di[m].dev_name,dev_alias[m]);
			} else {
				printcl( CL_DEBUG "clopen: dev[%d] name |%s|",m,di[m].dev_name);
			}
		}
	
		printcl( CL_DEBUG "clopen: _proc_clelf_sect %p",_proc_clelf_sect);

		int platform_code = clelf_platform_code(cp->platform_name);
		/* XXX if 0 things are really not going to work out well -DAR */

		if (sect->has_clprgtab) {

			printcl( CL_DEBUG "clopen: searching clprgtab (%d entries) ...",
				sect->clprgtab_n);

			char** bin = (char**)calloc( sizeof(char*), cp->ndev );
			size_t* bin_sz = (size_t*)calloc( sizeof(size_t), cp->ndev );
//			int nbin = 0;

			struct clprgtab_entry* p = sect->clprgtab;

         for(n=0; n<sect->clprgtab_n; n++,p++) {

				printcl( CL_DEBUG "clopen: checking %d binaries ...",p->e_nprgbin);

				int nbin = 0;

				for(m=0;m<cp->ndev; m++) {

					int nb;
					struct clprgbin_entry* pb = sect->clprgbin + p->e_prgbin;
					for(nb=0; nb < p->e_nprgbin; nb++,pb++ ) {

						printcl( CL_DEBUG "clopen: test %d:|%s| ? %d:|%s|",
							platform_code,dev_alias[m],
							pb->e_platform,sect->clstrtab + pb->e_device);

//						if (!strncmp(di[m].dev_name, sect->clstrtab+pb->e_device,256)
						if (!strncmp(dev_alias[m], sect->clstrtab+pb->e_device,256)
							&& platform_code == pb->e_platform ) {

//continue;

								bin[m] = sect->cltextbin + pb->e_offset;
								bin_sz[m] = pb->e_size;
								++nbin;
								break;

						}

					}

				}

				printcl( CL_DEBUG "clopen: nbin=%d ndev=%d",nbin,cp->ndev);

				if (nbin == cp->ndev) {

					printcl( CL_DEBUG "clopen: found all binaries");

					prgs = clloadb(cp,nbin,bin,bin_sz,flags);
					prgs->fname = fname;
					prgs->fd = fd;


				} else { 

					printcl( CL_DEBUG 
						"clopen: binary load failed, check for source");

					if (p->e_nprgsrc) {

						/* XXX here we only take first source for now -DAR */

//						struct clprgbin_entry* ps = sect->clprgsrc + p->e_prgsrc;
						struct clprgsrc_entry* ps = sect->clprgsrc + p->e_prgsrc;
						ptr = sect->cltextsrc + ps->e_offset;
						len = ps->e_size;

						printcl( CL_DEBUG "clopen: source %p (%d bytes)",ptr,len);

						prgs = clload(cp,ptr,len,flags);
						prgs->fname = fname;
						prgs->fd = fd;

					} else {

						printcl( CL_DEBUG "clopen: no source found");

					}

				}

			}

		} else if (_proc_clelf_sect->has_text) { /* might be legacy format */

		} else {

			printcl( CL_DEBUG "clopen: proc contains no CLELF sections");

		}

		free(di);
		for(m=0; m<cp->ndev; m++) if (dev_alias[m]) free(dev_alias[m]);
		free(dev_alias);

#endif

		_clopen_zero = 1;

		return(0);

	} else {

		if (stat(fname,&fs)) return(0);

		printcl( CL_DEBUG " stat ok ");
		printcl( CL_DEBUG " file size = %d ",fs.st_size);

		if (fs.st_size==0 || !S_ISREG(fs.st_mode)) return(0);

		/* XXX any more checks that should be done? -DAR */

		len = fs.st_size;

#ifdef _WIN64

		/* windows implementation of stat() is broken, try something else */
		fp = fopen(fname,"r");
		fseek(fp, 0L, SEEK_END); 
		len = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		ptr = malloc(len+1);
		memset(ptr,'\0',len+1);
		fread(ptr,1,len,fp);
		fclose(fp);
		
		len = strlen((char*)ptr);
		
		prgs = (struct _prgs_struct *)clload(cp,ptr,len,flags);
		prgs->fname = fname;
		prgs->fp = fp;

#else

		if ((fd = open(fname,O_RDONLY)) == -1) return(0); 

		printcl( CL_DEBUG " file open ");

		if (!(ptr = mmap(0,len,PROT_READ,MAP_SHARED,fd,0))) {
			close(fd);
			return(0);
		}

//		prgs = (struct _prgs_struct *)clload(cp,ptr,len,flags);
//		prgs->fname = fname;
//		prgs->fd = fd;

      char* pext = strrchr(fname,'.');

      if (
         pext && strcmp(pext,".cl") && ((char*)ptr)[0]==(char)0x7f
         && ((char*)ptr)[1]=='E' && ((char*)ptr)[2]=='L' && ((char*)ptr)[3]=='F'
      ) {
         printcl( CL_DEBUG "detected binary file");
         prgs = (struct _prgs_struct *)clloadb(cp,1,(char**)&ptr,&len,flags);
      } else {
         prgs = (struct _prgs_struct *)clload(cp,ptr,len,flags);
      }

      prgs->fname = fname;
      prgs->fd = fd;

#endif

		return((void*)prgs);
	}

}


LIBSTDCL_API int 
clclose(CONTEXT* cp, void* handle)
{
	int n;
	int err;
	int ictx;

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

			printcl( CL_DEBUG " removing txt from list\n");

			LIST_REMOVE(txt, txt_list);

			for(n=0;n<txt->nkrn;n++) {
				err = clReleaseKernel(txt->krn[n]);
				__set_oclerrno(err);
			}

			free(txt->krn);
			free(txt->krntab);

			for(ictx=0; ictx < cp->nctx; ictx++) {
				err = clReleaseProgram(txt->xxxprg[ictx]);
				__set_oclerrno(err);
			}

			free(txt);

			txt = 0;
		}
		txt = txt_next;
	}



	printcl( CL_DEBUG " removing prgs from list\n");
	LIST_REMOVE(prgs, prgs_list);

#ifndef _WIN64
	if (prgs->fd >= 0) close(prgs->fd);
#endif

	free(prgs);

	return(0);

}



LIBSTDCL_API cl_kernel 
clsym( CONTEXT* cp, void* handle, const char* sname, int flags )
{
	int n;
	struct _prgs_struct* prgs;

	if (!sname) return(0);

//	if (!handle) {
//		ERROR(__FILE__,__LINE__,"null handle not supported");
//		return(0);
//	}

	if ( !handle && !_clopen_zero ) {
		printcl( CL_DEBUG "need to force call to clopen");
		clopen(cp,0,flags);
	}

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
			printcl( CL_DEBUG "clsym: txt->krn %p",txt->krn);
			printcl( CL_DEBUG "clsym: searching kernels ...");
			for(n=0;n<txt->nkrn;n++) {

				printcl( CL_DEBUG "checking |%s|%s|",
					txt->krntab[n].kname,sname);

				if (!strncmp(sname,txt->krntab[n].kname,256)) return(txt->krn[n]);

			}
		}
   }

	printcl( CL_DEBUG "clsym: symbol not found");

	return((cl_kernel)0);

}


LIBSTDCL_API char*  
clerror(void)
{
	return(0);
}



LIBSTDCL_API void* 
clsopen( CONTEXT* cp, const char* srcstr, int flags )
{
	int n;
	int err;
	size_t len;
	void* ptr;
	struct _prgs_struct* prgs;

printcl( CL_DEBUG " checking cp ");

	if (!cp) return(0);

printcl( CL_DEBUG " cp ok ");

	if (srcstr) {

		ptr = (void*)srcstr;
		len = strlen(srcstr);

		prgs = (struct _prgs_struct *)clload(cp,ptr,len,flags);
		prgs->fname = 0;
#ifdef _WIN64
		prgs->fp = 0;
#else
		prgs->fd = -1;
#endif

		return((void*)prgs);
	}

}



