/* clfcn.c
 *
 * Copyright (c) 2009-2011 Brown Deer Technology, LLC.  All Rights Reserved.
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


/*
 * NOTES:
 * 	-Right now one-to-one mapping between clopen() and clclose() calls,
 * 	 should extend to use ref count.
 *
 *		-Supports simple CL build program from source semantic, should extend.
 *
 */


LIBSTDCL_API void* 
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


LIBSTDCL_API void* 
clloadb( CONTEXT* cp, int nbin, char** bin, size_t* bin_sz, int flags )
{
	int n;
	int err;

	DEBUG(__FILE__,__LINE__,"clloadb: checking cp ");

	if (!cp) return(0);

	DEBUG(__FILE__,__LINE__,"clloadb: cp ok ");


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

	txt->prg = clCreateProgramWithBinary(cp->ctx,cp->ndev,cp->dev,
		bin_sz,(const unsigned char**)bin,bin_stat,&err);

	DEBUG2("clloadb: err from clCreateProgramWithBinary %d (%p)",err,txt->prg);

	for(n=0;n<cp->ndev;n++) {
		DEBUG2("clloadb: bin stat [%d] %d",n,bin_stat[n]);
	}
	DEBUG2("clloadb: err from clCreateProgramWithBinary %d",err);

	LIST_INSERT_HEAD(&cp->txt_listhead, txt, txt_list);



//	if (!(flags&CLLD_NOBUILD)) {
//		DEBUG(__FILE__,__LINE__,"clload: calling clbuild");
//		clbuild(cp,prgs,0,flags);
//	}

	err = clBuildProgram(txt->prg,cp->ndev,cp->dev,0,0,0);
	DEBUG2("clloadb: err from clBuildProgram %d",err);


//	LIST_INSERT_HEAD(&cp->txt_listhead, txt, txt_list);


	/* extract the kernels */

	err = clCreateKernelsInProgram(txt->prg,0,0,&txt->nkrn);
	DEBUG2("clloadb: err from clCreateKernelsInProgram %d",err);
	DEBUG2("clloadb: NUMBER OF KERNELS %d",txt->nkrn);

	txt->krn = (cl_kernel*)malloc(sizeof(cl_kernel)*txt->nkrn);

	err = clCreateKernelsInProgram(txt->prg,txt->nkrn,txt->krn,0);


	txt->krntab = (struct _krntab_struct*)malloc(
		sizeof(struct _krntab_struct)*txt->nkrn
	);

	for(n=0;n<txt->nkrn;n++) {

		clGetKernelInfo( txt->krn[n],CL_KERNEL_FUNCTION_NAME,256,
			txt->krntab[n].kname,0);

		clGetKernelInfo( txt->krn[n],CL_KERNEL_NUM_ARGS,sizeof(cl_uint),
			&txt->krntab[n].nargs,0);

		clGetKernelInfo( txt->krn[n],CL_KERNEL_REFERENCE_COUNT,
			sizeof(cl_uint), &txt->krntab[n].refc,0);

		clGetKernelInfo( txt->krn[n],CL_KERNEL_CONTEXT,sizeof(cl_context),
				&txt->krntab[n].kctx,0);

		clGetKernelInfo(txt->krn[n],CL_KERNEL_PROGRAM,sizeof(cl_program),
				&txt->krntab[n].kprg,0);

		DEBUG(__FILE__,__LINE__,"clloadb: %s %d %p %p\n",
			txt->krntab[n].kname,txt->krntab[n].nargs,
			txt->krntab[n].kctx,txt->krntab[n].kprg);

	}
	
	return((void*)prgs);

}


static int _clbuild_zero = 0; 

LIBSTDCL_API void* 
clbuild( CONTEXT* cp, void* handle, char* uopts, int flags )
{
	int n;
	int err;
	struct _prgs_struct* prgs;

	DEBUG(__FILE__,__LINE__," checking cp ");

	if (!cp) return(0);

	if (_clbuild_zero) return(0);

	DEBUG(__FILE__,__LINE__," cp ok ");

	prgs = (struct _prgs_struct*)handle;

	DEBUG(__FILE__,__LINE__,"prgs %p",prgs);

	struct _txt_struct* txt;
   for (
      txt = cp->txt_listhead.lh_first; txt != 0;
      txt = txt->txt_list.le_next
   ) {
//		if (prgs==0 || txt->prgs == prgs) break;
//   }

//	if (!txt) {
//		DEBUG(__FILE__,__LINE__,"clbuild: bad handle");
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

DEBUG2("clbuild: platform_vendor '%s'",cp->platform_vendor);

	if (!strcasecmp(cp->platform_vendor,"Advanced Micro Devices, Inc.")) 
		strcat(opts," -D __AMD__");
	else if (!strcasecmp(cp->platform_vendor,"NVIDIA Corporation")) 
		strcat(opts," -D __NVIDIA__");
	else if (!strcasecmp(cp->platform_vendor,"Brown Deer Technology, LLC.")) 
		strcat(opts," -D __coprthr__");

printf("opts |%s|\n",opts);
#ifdef DEFAULT_STDCL_INCLUDE
	strcat(opts," -I" DEFAULT_STDCL_INCLUDE " -D __stdcl_kernel__ ");
#endif
	strcat(opts,"\0");

printf("opts |%s|\n",opts);


//	char cwd[1024];
	char* cwd = (char*)malloc(1024);
	if ( getcwd(cwd,1024) ) {
		n = strnlen(cwd,1024);
		printf("len of cwd %d\n",n);
		printf("len of opts %d\n",strnlen(opts,1024));
		if (n+strnlen(opts,1024)>1022) {
			printf("opts |%s|\n",opts);
			WARN(__FILE__,__LINE__,
				"clbuild: cwd too long to include as header search path.");
		} else {
			strcat(opts," -I");
			strncat(opts,cwd,n);
		}
	}

	if (cwd) free(cwd);

	if (uopts) {

//		n = strnlen(uopts,64);
		n = strnlen(uopts,1024);
		DEBUG(__FILE__,__LINE__,"%d",n);
		if (n+strnlen(opts,1024)>1022) {
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

	if (opts) free(opts);

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

DEBUG(__FILE__,__LINE__," checking cp ");

	if (!cp) return(0);

DEBUG(__FILE__,__LINE__," cp ok ");

	if (!fname) {

		if (_clopen_zero) {
			DEBUG(__FILE__,__LINE__,"already here");
			return(0);
		}

#ifdef _WIN64

		WARN(__FILE__,__LINE__,"embedded kernels not supported for Windows");

#else

		DEBUG(__FILE__,__LINE__," fname null, search _proc_clelf_sect");

		if (!_proc_clelf_sect) {
			DEBUG2("clopen: _proc_clelf_sect is null");
			return(0);
		}

		struct clelf_sect_struct* sect = _proc_clelf_sect;

		if (!sect->has_any_clelf_section) {
			DEBUG2("clopen: no CLELF sections found");
			return(0);
		}

		fd = -1;
		ptr = 0;
		len = 0;

//		DEBUG(__FILE__,__LINE__,"clopen: %p searching _proc_cl for prgs...",
//			_proc_cl.clstrtab);
//
// 		DEBUG(__FILE__,__LINE__,"clopen: %s",&_proc_cl.clstrtab[1]);
//
//		DEBUG(__FILE__,__LINE__,"clopen: _proc_cl.clprgs_n=%d",_proc_cl.clprgs_n);
//
//		struct clprgs_entry* sp;
//		for(n=0,sp=_proc_cl.clprgs;n<_proc_cl.clprgs_n;n++,sp++) {
//			DEBUG(__FILE__,__LINE__,"found %s (%d bytes)\n",
//				&_proc_cl.clstrtab[sp->e_name],sp->e_size);
//			ptr = _proc_cl.cltexts+sp->e_offset;
//			len = sp->e_size;
//		
//			prgs = clload(cp,ptr,len,flags);
//			prgs->fname = fname;
//			prgs->fd = fd;
//		}

		DEBUG2("clopen: need %d devices for binary kernels",cp->ndev);
		int m;
		struct cldev_info* di
			= (struct cldev_info*)malloc(cp->ndev * sizeof(struct cldev_info));
		clgetdevinfo(cp,di);
		char** dev_alias = (char**)malloc(cp->ndev * sizeof(char*));
		for(m=0; m<cp->ndev; m++) {
			dev_alias[m] = (char*)malloc(strlen(di[m].dev_name));
			strcpy(dev_alias[m],di[m].dev_name);
			if ( clelf_device_name_alias(dev_alias[m]) ) {
				DEBUG2("clopen: dev[%d] name |%s| (%s)",
					m,di[m].dev_name,dev_alias[m]);
			} else {
				DEBUG2("clopen: dev[%d] name |%s|",m,di[m].dev_name);
			}
		}
	
		DEBUG2("clopen: _proc_clelf_sect %p",_proc_clelf_sect);

		int platform_code = clelf_platform_code(cp->platform_name);
		/* XXX if 0 things are really not going to work out well -DAR */

		if (sect->has_clprgtab) {

			DEBUG2("clopen: searching clprgtab (%d entries) ...",sect->clprgtab_n);

			char** bin = (char**)calloc( sizeof(char*), cp->ndev );
			size_t* bin_sz = (size_t*)calloc( sizeof(size_t), cp->ndev );
//			int nbin = 0;

			struct clprgtab_entry* p = sect->clprgtab;

         for(n=0; n<sect->clprgtab_n; n++,p++) {

				DEBUG2("clopen: checking %d binaries ...",p->e_nprgbin);

				int nbin = 0;

				for(m=0;m<cp->ndev; m++) {

					int nb;
					struct clprgbin_entry* pb = sect->clprgbin + p->e_prgbin;
					for(nb=0; nb < p->e_nprgbin; nb++,pb++ ) {

						DEBUG2("clopen: test %d:|%s| ? %d:|%s|",
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

				DEBUG2("clopen: nbin=%d ndev=%d",nbin,cp->ndev);

				if (nbin == cp->ndev) {

					DEBUG2("clopen: found all binaries");

					prgs = clloadb(cp,nbin,bin,bin_sz,flags);
					prgs->fname = fname;
					prgs->fd = fd;


				} else { 

					DEBUG2("clopen: binary load failed, check for source");

					if (p->e_nprgsrc) {

						/* XXX here we only take first source for now -DAR */

//						struct clprgbin_entry* ps = sect->clprgsrc + p->e_prgsrc;
						struct clprgsrc_entry* ps = sect->clprgsrc + p->e_prgsrc;
						ptr = sect->cltextsrc + ps->e_offset;
						len = ps->e_size;

						DEBUG2("clopen: source %p (%d bytes)",ptr,len);

						prgs = clload(cp,ptr,len,flags);
						prgs->fname = fname;
						prgs->fd = fd;

					} else {

						DEBUG2("clopen: no source found");

					}

				}

			}

		} else if (_proc_clelf_sect->has_text) { /* might be legacy format */

		} else {

			DEBUG2("clopen: proc contains no CLELF sections");

		}

		free(di);
		for(m=0; m<cp->ndev; m++) if (dev_alias[m]) free(dev_alias[m]);
		free(dev_alias);

#endif

		_clopen_zero = 1;

		return(0);

	} else {

		if (stat(fname,&fs)) return(0);

		DEBUG(__FILE__,__LINE__," stat ok ");
		DEBUG(__FILE__,__LINE__," file size = %d ",fs.st_size);

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

		DEBUG(__FILE__,__LINE__," file open ");

		if (!(ptr = mmap(0,len,PROT_READ,MAP_SHARED,fd,0))) {
			close(fd);
			return(0);
		}


		prgs = (struct _prgs_struct *)clload(cp,ptr,len,flags);
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


LIBSTDCL_API char*  clerror(void)
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

DEBUG(__FILE__,__LINE__," checking cp ");

	if (!cp) return(0);

DEBUG(__FILE__,__LINE__," cp ok ");

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



