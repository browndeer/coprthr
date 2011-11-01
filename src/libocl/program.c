/* program.c
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

#include <dlfcn.h>

#include <CL/cl.h>

#include "xcl_structs.h"
#include "program.h"
#include "compiler.h"
#include "elf_cl.h"
#include "cmdcall.h" /* XXX this is only used for backdoor -DAR */
#include "vcore.h"	/* XXX this is only used for backdoor -DAR */


/* XXX this is a hack, copied from xclnm_gram.h, fix this! -DAR */
//#define __TYPE_OPAQUE  262
//#define __TYPE_VOID  263
//#define __TYPE_INT8  264
//#define __TYPE_INT16  265
//#define __TYPE_INT32  266
//#define __TYPE_INT64  267
//#define __TYPE_FLOAT  268
//#define __TYPE_DOUBLE  269
#define __TYPE_OPAQUE  261
#define __TYPE_VOID  262
#define __TYPE_INT8  263
#define __TYPE_INT16  264
#define __TYPE_INT32  265
#define __TYPE_INT64  266
#define __TYPE_UINT8  267
#define __TYPE_UINT16  268
#define __TYPE_UINT32  269
#define __TYPE_UINT64  270
#define __TYPE_FLOAT  271
#define __TYPE_DOUBLE  272




void __do_create_program(cl_program prg) 
{
	DEBUG(__FILE__,__LINE__,"__do_create_program with ndev = %d",prg->ndev);

	prg->imp.v_kbin = (void**)calloc(prg->ndev,sizeof(void*));
	prg->imp.v_ksym = (void***)calloc(prg->ndev,sizeof(void**));
	prg->imp.v_kcall = (void***)calloc(prg->ndev,sizeof(void**));
}

void __do_release_program(cl_program prg) {}

cl_int __do_build_program_from_source(
	cl_program prg,cl_device_id devid, cl_uint devnum
){ 

	DEBUG(__FILE__,__LINE__,"__do_build_program_from_source");

	int i,j;

	compiler_t comp = (compiler_t)__resolve_devid(devid,comp);
	
	DEBUG(__FILE__,__LINE__,
		"__do_build_program_from_source: compiler=%p",comp);	

//	void* h = comp(
//		devid,
//		prg->src,prg->src_sz,
//		prg->bin[devnum],prg->bin_sz[devnum],
//		prg->build_options[devnum],prg->build_log[devnum]
//	);
	struct _elf_data* edata = (struct _elf_data*)comp(
		devid,
		prg->src,prg->src_sz,
		prg->bin[devnum],prg->bin_sz[devnum],
		prg->build_options[devnum],prg->build_log[devnum]
	);
	void* h = edata->dlh;
	Elf* e = (Elf*)edata->map;

//	typedef void*(*get_ptr_func_t)();
//	typedef int(*get_int_func_t)();
//	typedef size_t(*get_sz_func_t)();
//
//	get_ptr_func_t __get_shstrtab = dlsym(h,"__get_shstrtab");
//
//	get_sz_func_t __get_clstrtab_sz = dlsym(h,"__get_clstrtab_sz");
//	get_ptr_func_t __get_clstrtab = dlsym(h,"__get_clstrtab");
//
//	get_int_func_t __get_clsymtab_n = dlsym(h,"__get_clsymtab_n");
//	get_ptr_func_t __get_clsymtab = dlsym(h,"__get_clsymtab");
//
//	get_int_func_t __get_clargtab_n = dlsym(h,"__get_clargtab_n");
//	get_ptr_func_t __get_clargtab = dlsym(h,"__get_clargtab");
//
//	get_ptr_func_t __get_cltextb = dlsym(h,"__get_cltextb");

	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)e;
	Elf64_Shdr* shdr = (Elf64_Shdr*)((intptr_t)e + ehdr->e_shoff);
	char* shstrtab = (char*)((intptr_t)e + shdr[ehdr->e_shstrndx].sh_offset);

	size_t clstrtab_sz = 0;
	char* clstrtab = 0;
	int clsymtab_n = 0;
	struct clsymtab_entry* clsymtab = 0;
	int clargtab_n = 0;
	struct clargtab_entry* clargtab = 0;
	char* cltextb = 0;

	for(i=0;i<ehdr->e_shnum;i++,shdr++) {
		if (!strncmp(shstrtab+shdr->sh_name,".clstrtab",9)) {
			clstrtab_sz =shdr->sh_size;
			clstrtab =(char*)((intptr_t)e + shdr->sh_offset);
		} else if (!strncmp(shstrtab+shdr->sh_name,".clsymtab",9)) {
			clsymtab_n =shdr->sh_size/sizeof(struct clsymtab_entry);
			clsymtab =(struct clsymtab_entry*)((intptr_t)e+shdr->sh_offset);
		} else if (!strncmp(shstrtab+shdr->sh_name,".clargtab",9)) {
			clargtab_n =shdr->sh_size/sizeof(struct clargtab_entry);
			clargtab =(struct clargtab_entry*)((intptr_t)e+shdr->sh_offset);
		} else if (!strncmp(shstrtab+shdr->sh_name,".cltextb",8)) {
			cltextb =(char*)((intptr_t)e+shdr->sh_offset);
		}
	}
	
//	char* shstrtab = __get_shstrtab();
//
//	size_t clstrtab_sz = __get_clstrtab_sz();
//	char* clstrtab = __get_clstrtab();
//
//	int clsymtab_n = __get_clsymtab_n();
//	struct clsymtab_entry* clsymtab = __get_clsymtab();
//
//	int clargtab_n = __get_clargtab_n();
//	struct clargtab_entry* clargtab = __get_clargtab();

	DEBUG(__FILE__,__LINE__,"clstrtab_sz %d\n",clstrtab_sz);


//	char* ppp = (char*)__get_cltextb();
	char* ppp = (char*)cltextb;
	printf("is image an ELF? %s\n",ppp);

	#if defined(XCL_DEBUG)
	fprintf(stdout,"%p\n",shstrtab);
	fprintb(stdout,shstrtab,100); printf("\n");
	fprintf(stdout,"%p\n",clstrtab);
	fprintb(stdout,clstrtab,100); printf("\n");
	#endif

	for(i=0;i<clsymtab_n;i++) {
		DEBUG(__FILE__,__LINE__,
			"[%d] clsym |%s|",i,clstrtab+clsymtab[i].e_name);
	}

	for(i=0;i<clargtab_n;i++) {
		DEBUG(__FILE__,__LINE__,
			"[%d] clarg |%s| (+%d)",i,clstrtab+clargtab[i].e_name,
			clargtab[i].e_name);
	}

	if (prg->imp.clsymtab == 0) {

		prg->imp.nclsym = clsymtab_n;
	
		__clone(prg->imp.clsymtab,clsymtab,clsymtab_n,struct clsymtab_entry);
		__clone(prg->imp.clargtab,clargtab,clargtab_n,struct clargtab_entry);
		__clone(prg->imp.clstrtab,clstrtab,clstrtab_sz,char);

		prg->imp.nkrn = clsymtab_n; /* XXX assumed, revisit in future -DAR */

		prg->imp.kname = (char**)malloc(prg->imp.nkrn*sizeof(char*));
		prg->imp.knarg = (cl_uint*)malloc(prg->imp.nkrn*sizeof(cl_uint));
		prg->imp.karg_buf_sz = (size_t*)malloc(prg->imp.nkrn*sizeof(size_t));
		prg->imp.karg_kind = (cl_uint**)malloc(prg->imp.nkrn*sizeof(cl_uint*));
		prg->imp.karg_sz = (size_t**)malloc(prg->imp.nkrn*sizeof(size_t*));


		/* XXX this is a hack, copied from xclnm_gram.h, fix this! -DAR */

//#define __TYPE_OPAQUE  262
//#define __TYPE_VOID  263
//#define __TYPE_INT8  264
//#define __TYPE_INT16  265
//#define __TYPE_INT32  266
//#define __TYPE_INT64  267
//#define __TYPE_FLOAT  268
//#define __TYPE_DOUBLE  269


		for(i=0;i<clsymtab_n;i++) {

			prg->imp.kname[i] = prg->imp.clstrtab + prg->imp.clsymtab[i].e_name;
			unsigned int arg0 = prg->imp.clsymtab[i].e_arg0;
			int narg = 0; 
			int arg;
			for(arg=arg0;arg;arg=prg->imp.clargtab[arg].e_nxt,narg++);
			DEBUG(__FILE__,__LINE__,"%s has %d args\n",prg->imp.kname[i],narg);
			prg->imp.knarg[i] = narg;
			prg->imp.karg_kind[i] = (cl_uint*)malloc(narg*sizeof(cl_uint));
			prg->imp.karg_sz[i] = (size_t*)malloc(narg*sizeof(size_t));

			j = 0;
			size_t bufsz = 0;
			for(arg=arg0;arg;arg=prg->imp.clargtab[arg].e_nxt,j++) {

				size_t sz;
				size_t sz_ptr = sizeof(void*); /* XXX always true? -DAR */

				switch(prg->imp.clargtab[arg].e_datatype) {

					case __TYPE_INT8:
					case __TYPE_UINT8: 
						sz=1; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_INT8",arg);
						break;
						
					case __TYPE_INT16:
					case __TYPE_UINT16: 
						sz=2; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_INT16",arg);
						break;

					case __TYPE_INT32:
					case __TYPE_UINT32: 
					case __TYPE_FLOAT:
						sz=4; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_INT32",arg);
						break;

					case __TYPE_INT64: 
					case __TYPE_UINT64: 
					case __TYPE_DOUBLE:
						sz=8; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_INT64",arg);
						break;

					case __TYPE_VOID:
					default: sz=0; 
					DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_VOID",arg);
					break;

				}

				DEBUG(__FILE__,__LINE__,"base arg_sz[%d] %d",arg,sz);

				sz *= prg->imp.clargtab[arg].e_vecn;
				sz *= prg->imp.clargtab[arg].e_arrn;
				
				DEBUG(__FILE__,__LINE__,"w/multiplicity arg_sz[%d] %d",arg,sz);

				DEBUG(__FILE__,__LINE__,"e_ptrc=%d e_addrspace=%d",
					prg->imp.clargtab[arg].e_ptrc,
					prg->imp.clargtab[arg].e_addrspace);

				if (prg->imp.clargtab[arg].e_ptrc == 0) {

					prg->imp.karg_kind[i][j] = CLARG_KIND_DATA;
					sz = sz;

				} else if (prg->imp.clargtab[arg].e_ptrc == 1) {

					if (prg->imp.clargtab[arg].e_addrspace == 0) { /* XXX promote */

						prg->imp.karg_kind[i][j] = CLARG_KIND_GLOBAL;
						sz = sz_ptr;

					} else if (prg->imp.clargtab[arg].e_addrspace == 1) {

						prg->imp.karg_kind[i][j] = CLARG_KIND_GLOBAL;
						sz = sz_ptr;

					} else if (prg->imp.clargtab[arg].e_addrspace == 2) {

						prg->imp.karg_kind[i][j] = CLARG_KIND_CONSTANT;
						sz = sz_ptr;

					} else if (prg->imp.clargtab[arg].e_addrspace == 3) {

						prg->imp.karg_kind[i][j] = CLARG_KIND_LOCAL;
						sz = sz_ptr;

					} else {

						prg->imp.karg_kind[i][j] = CLARG_KIND_UNDEFINED;
						sz = 0;

					}

				} else {

					prg->imp.karg_kind[i][j] = CLARG_KIND_UNDEFINED;
					sz = 0;

				}

				DEBUG(__FILE__,__LINE__,"after kind check arg_sz[%d] %d",arg,sz);

				prg->imp.karg_sz[i][j] = sz;
				bufsz += sz;

			}

			prg->imp.karg_buf_sz[i] = bufsz;

		}	


	}


	prg->imp.v_ksym[devnum] = (void**)malloc(prg->imp.nkrn*sizeof(void*));
	prg->imp.v_kcall[devnum] = (void**)malloc(prg->imp.nkrn*sizeof(void*));


	char name[1024];
	int err;

	cl_device_type devtype = __resolve_devid(devid,devtype);

#ifdef ENABLE_ATIGPU
	CALimage calimg = 0;
	CALmodule calmod = 0;
	if (devtype == CL_DEVICE_TYPE_GPU) {
//		calimg = (CALimage)__get_cltextb();
		calimg = (CALimage)cltextb;
		err = calModuleLoad(&calmod,prg->ctx->imp.calctx[devnum],calimg);
		DEBUG(__FILE__,__LINE__,"calModuleLoad returned %d",err);
		DEBUG(__FILE__,__LINE__,"calmod %p",calmod);
	}
#endif

	if (devtype == CL_DEVICE_TYPE_CPU) prg->imp.v_kbin[devnum] = h;
#ifdef ENABLE_ATIGPU
	else if (devtype == CL_DEVICE_TYPE_GPU) {
		DEBUG(__FILE__,__LINE__,"is CALmodule a ptr? %d",sizeof(CALmodule));
		prg->imp.v_kbin[devnum] = (void*)(unsigned long long)calmod;
	}
#endif
	else prg->imp.v_kbin[devnum] = 0;

	DEBUG(__FILE__,__LINE__,"kbin[%d] = %p",devnum,prg->imp.v_kbin[devnum]);

	for(i=0;i<prg->imp.nclsym;i++) {

//		strncpy(name,"__OpenCL_",1024);
//		strncat(name,prg->imp.kname[i],1024);
		strncpy(name,prg->imp.kname[i],1024);
//		strncat(name,"_kernel",1024);

		DEBUG(__FILE__,__LINE__,"devnum knum %d %d",devnum,i);

		if (__resolve_devid(devid,devtype)==CL_DEVICE_TYPE_CPU) {
		   prg->imp.v_ksym[devnum][i] = 0;
			prg->imp.v_ksym[devnum][i] = dlsym(h,name);
		}
#ifdef ENABLE_ATIGPU
		else if (__resolve_devid(devid,devtype)==CL_DEVICE_TYPE_GPU) {

			CALfunc calfunc = 0;
			err = calModuleGetEntry(&calfunc,prg->ctx->imp.calctx[devnum],calmod,"name");
			DEBUG(__FILE__,__LINE__,"calModuleGetEntry returned %d",err);
			DEBUG(__FILE__,__LINE__,"calModuleGetEntry returned %s",calGetErrorString());
			err = calModuleGetEntry(&calfunc,prg->ctx->imp.calctx[devnum],calmod,name);
			DEBUG(__FILE__,__LINE__,"calModuleGetEntry returned %d",err);
			prg->imp.v_ksym[devnum][i] = (void*)(cl_long)calfunc;

		}
#endif
		else prg->imp.v_ksym[devnum][i] = 0;

		DEBUG(__FILE__,__LINE__,"ksym %s -> %p",name,prg->imp.v_ksym[devnum][i]);

		strncpy(name,"__XCL_call_",1024);
		strncat(name,prg->imp.kname[i],1024);

		if (__resolve_devid(devid,devtype)==CL_DEVICE_TYPE_CPU) 
			prg->imp.v_kcall[devnum][i] = dlsym(h,name);
		else prg->imp.v_kcall[devnum][i] = 0;

		DEBUG(__FILE__,__LINE__,"kcall %s -> %p",
			name,prg->imp.v_kcall[devnum][i]);

	}


	return(CL_SUCCESS); 
}


static void __xanadu( cl_int* a, cl_int* b, cl_int* c);
static void __XCL_call_xanadu( void* p );

cl_int __do_build_program_from_binary(
	cl_program prg,cl_device_id devid, cl_uint devnum
)
{
	int i,j;

	DEBUG(__FILE__,__LINE__,"%d %p %d\n",prg->ndev,prg->bin[0],prg->bin_sz[0]);

	for(i=0;i<prg->ndev;i++) {

		if (!strncmp(prg->bin[i],"BC",2)) {

			WARN(__FILE__,__LINE__,"__do_build_program_from_binary:"
				" BC format not presently supported");

			return(CL_BUILD_PROGRAM_FAILURE);

#ifdef ENABLE_ATIGPU
		} else if (!strncmp(prg->bin[i],"il_",3)) {

			WARN(__FILE__,__LINE__,"__do_build_program_from_binary:"
				" IL format not presently supported");

			ircompiler_t ircomp = (ircompiler_t)__resolve_devid(devid,ilcomp);
	
			DEBUG(__FILE__,__LINE__,
				"__do_build_program_from_binary: ircompiler=%p",ircomp);	

//			void* h = ircomp(
//				devid,
//				prg->src,prg->src_sz,
//				prg->bin[devnum],prg->bin_sz[devnum],
//				prg->build_options[devnum],prg->build_log[devnum]
//			);
			struct _elf_data* edata = (struct _elf_data*)comp(
				devid,
				prg->src,prg->src_sz,
				prg->bin[devnum],prg->bin_sz[devnum],
				prg->build_options[devnum],prg->build_log[devnum]
			);
			void* h = edata->dlh;
			Elf* e = (Elf*)edata->map;


			if (!h) {
				WARN(__FILE__,__LINE__,
					"__do_build_program_from_binary: ircompile failed");
				return(CL_BUILD_PROGRAM_FAILURE);
			}

//			typedef void*(*get_ptr_func_t)();
//			typedef int(*get_int_func_t)();
//			typedef size_t(*get_sz_func_t)();
//
//			get_ptr_func_t __get_shstrtab = dlsym(h,"__get_shstrtab");
//
//			get_sz_func_t __get_clstrtab_sz = dlsym(h,"__get_clstrtab_sz");
//			get_ptr_func_t __get_clstrtab = dlsym(h,"__get_clstrtab");
//
//			get_int_func_t __get_clsymtab_n = dlsym(h,"__get_clsymtab_n");
//			get_ptr_func_t __get_clsymtab = dlsym(h,"__get_clsymtab");
//
//			get_int_func_t __get_clargtab_n = dlsym(h,"__get_clargtab_n");
//			get_ptr_func_t __get_clargtab = dlsym(h,"__get_clargtab");
//
//			get_ptr_func_t __get_cltextb = dlsym(h,"__get_cltextb");

	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)e;
	Elf64_Shdr* shdr = (Elf64_Shdr*)((intptr_t)e + ehdr->e_shoff);
	char* shstrtab = (char*)((intptr_t)e + shdr[ehdr->e_shstrndx].sh_offset);

	size_t clstrtab_sz = 0;
	char* clstrtab = 0;
	int clsymtab_n = 0;
	struct clsymtab_entry* clsymtab = 0;
	int clargtab_n = 0;
	struct clargtab_entry* clargtab = 0;
	char* cltextb = 0;

	for(i=0;i<ehdr->e_shnum;i++,shdr++) {
		if (!strncmp(shstrtab+shdr->sh_name,".clstrtab",9)) {
			clstrtab_sz =shdr->sh_size;
			clstrtab =(char*)((intptr_t)e + shdr->sh_offset);
		} else if (!strncmp(shstrtab+shdr->sh_name,".clsymtab",9)) {
			clsymtab_n =shdr->sh_size/sizeof(struct clsymtab_entry);
			clsymtab =(struct clsymtab_entry*)((intptr_t)e+shdr->sh_offset);
		} else if (!strncmp(shstrtab+shdr->sh_name,".clargtab",9)) {
			clargtab_n =shdr->sh_size/sizeof(struct clargtab_entry);
			clargtab =(struct clargtab_entry*)((intptr_t)e+shdr->sh_offset);
		} else if (!strncmp(shstrtab+shdr->sh_name,".cltextb",8)) {
			cltextb =(char*)((intptr_t)e+shdr->sh_offset);
		}
	}
	
//			char* shstrtab = __get_shstrtab();
//
//			size_t clstrtab_sz = __get_clstrtab_sz();
//			char* clstrtab = __get_clstrtab();
//
//			int clsymtab_n = __get_clsymtab_n();
//			struct clsymtab_entry* clsymtab = __get_clsymtab();
//
//			int clargtab_n = __get_clargtab_n();
//			struct clargtab_entry* clargtab = __get_clargtab();
//
//			DEBUG(__FILE__,__LINE__,"clstrtab_sz %d\n",clstrtab_sz);

//			char* ppp = (char*)__get_cltextb();
			char* ppp = (char*)cltextb;
			printf("is image an ELF? %s\n",ppp);

			#if defined(XCL_DEBUG)
			fprintb(stdout,shstrtab,100); printf("\n");
			fprintb(stdout,clstrtab,100); printf("\n");
			#endif

			for(i=0;i<clsymtab_n;i++) {
				DEBUG(__FILE__,__LINE__,
					"[%d] clsym |%s| arg0=%d",i,clstrtab+clsymtab[i].e_name,
					clsymtab[i].e_arg0);
			}

			for(i=0;i<clargtab_n;i++) {
				DEBUG(__FILE__,__LINE__,
					"[%d] clarg |%s| (+%d)",i,clstrtab+clargtab[i].e_name,
					clargtab[i].e_name);
			}

			for(i=0;i<clargtab_n;i++) {
				DEBUG(__FILE__,__LINE__,
					"[%d] clarg |%s| (+%d) nxt=%d",i,clstrtab+clargtab[i].e_name,
					clargtab[i].e_name,clargtab[i].e_nxt);
			}

			
			if (prg->imp.clsymtab == 0) {

				prg->imp.nclsym = clsymtab_n;
	
				__clone(prg->imp.clsymtab,
					clsymtab,clsymtab_n,struct clsymtab_entry);

				__clone(prg->imp.clargtab,
					clargtab,clargtab_n,struct clargtab_entry);

				__clone(prg->imp.clstrtab,clstrtab,clstrtab_sz,char);

				prg->imp.nkrn = 1; /* XXX assume single kernel for ATI IL -DAR */

				prg->imp.kname = (char**)malloc(prg->imp.nkrn*sizeof(char*));

				prg->imp.knarg = (cl_uint*)malloc(prg->imp.nkrn*sizeof(cl_uint));

				prg->imp.karg_buf_sz
					= (size_t*)malloc(prg->imp.nkrn*sizeof(size_t));

				prg->imp.karg_kind 
					= (cl_uint**)malloc(prg->imp.nkrn*sizeof(cl_uint*));

				prg->imp.karg_sz = (size_t**)malloc(prg->imp.nkrn*sizeof(size_t*));


				i = 0; /* assumed to be single kernel in IL */


				/* find IL entry 'main' */
				int imain;
				for(imain=0;imain<clsymtab_n;imain++) 
					if (!strncmp(prg->imp.clstrtab+prg->imp.clsymtab[imain].e_name,
						"main",5)) break;

				if (imain == clsymtab_n) {
					ERROR(__FILE__,__LINE__,"cannot find IL entry 'main'");
					return(CL_BUILD_PROGRAM_FAILURE);
				}

				DEBUG(__FILE__,__LINE__,"main found %d",imain);

				prg->imp.kname[i] 
					= prg->imp.clstrtab + prg->imp.clsymtab[imain].e_name;

				unsigned int arg0 = prg->imp.clsymtab[imain].e_arg0;

				int narg = 0; 
				int arg;
				for(arg=arg0;arg;arg=prg->imp.clargtab[arg].e_nxt,narg++);
				DEBUG(__FILE__,__LINE__,"%s has %d args\n",prg->imp.kname[i],narg);

				prg->imp.knarg[i] = narg;
				prg->imp.karg_kind[i] = (cl_uint*)malloc(narg*sizeof(cl_uint));
				prg->imp.karg_sz[i] = (size_t*)malloc(narg*sizeof(size_t));
		
				j = 0;
				size_t bufsz = 0;
				for(arg=arg0;arg;arg=prg->imp.clargtab[arg].e_nxt,j++) {

					size_t sz;
					size_t sz_ptr = sizeof(void*); /* XXX always true? -DAR */
		
					switch(prg->imp.clargtab[arg].e_datatype) {
		
						case __TYPE_OPAQUE: sz=sizeof(void*);
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_OPAQUE",arg);
						break;

						case __TYPE_INT8: sz=1; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_INT8",arg);
						break;
						
						case __TYPE_INT16: sz=2; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_INT16",arg);
						break;

						case __TYPE_FLOAT:
						case __TYPE_INT32: sz=4; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_INT32",arg);
						break;

						case __TYPE_DOUBLE:
						case __TYPE_INT64: sz=8; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_INT64",arg);
						break;

						case __TYPE_VOID:
						default: sz=0; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] __TYPE_VOID",arg);
						break;
		
					}

					DEBUG(__FILE__,__LINE__,"base arg_sz[%d] %d",arg,sz);

					sz *= prg->imp.clargtab[arg].e_vecn;
					sz *= prg->imp.clargtab[arg].e_arrn;
						
					DEBUG(__FILE__,__LINE__,"w/multiplicity arg_sz[%d] %d",arg,sz);

					if (prg->imp.clargtab[arg].e_ptrc == 0) {

						prg->imp.karg_kind[i][j] = CLARG_KIND_DATA;
						sz = sz;

					} else if (prg->imp.clargtab[arg].e_ptrc == 1) {

						if (prg->imp.clargtab[arg].e_addrspace == 1) {

							prg->imp.karg_kind[i][j] = CLARG_KIND_GLOBAL;
							sz = sz_ptr;

						} else if (prg->imp.clargtab[arg].e_addrspace == 3) {
	
							prg->imp.karg_kind[i][j] = CLARG_KIND_LOCAL;
							sz = sz_ptr;

						} else {

							prg->imp.karg_kind[i][j] = CLARG_KIND_UNDEFINED;
							sz = 0;

						}

					} else {
	
						prg->imp.karg_kind[i][j] = CLARG_KIND_UNDEFINED;
						sz = 0;
	
					}
		
					DEBUG(__FILE__,__LINE__,"after kind check arg_sz[%d] %d",arg,sz);
		
					prg->imp.karg_sz[i][j] = sz;
					bufsz += sz;
		
				}
		
				prg->imp.karg_buf_sz[i] = bufsz;

			}


			prg->imp.v_ksym[devnum] 
				= (void**)malloc(prg->imp.nclsym*sizeof(void*));

			prg->imp.v_kcall[devnum] 
				= (void**)malloc(prg->imp.nclsym*sizeof(void*));


			char name[1024];
			int err;

			cl_device_type devtype = __resolve_devid(devid,devtype);

			if (devtype == CL_DEVICE_TYPE_GPU) {

				CALimage calimg = 0;
				CALmodule calmod = 0;

//				calimg = (CALimage)__get_cltextb();
				calimg = (CALimage)cltextb;
				err = calModuleLoad(&calmod,prg->ctx->imp.calctx[devnum],calimg);
				DEBUG(__FILE__,__LINE__,"calModuleLoad returned %d",err);
				DEBUG(__FILE__,__LINE__,"calmod %p",calmod);

				DEBUG(__FILE__,__LINE__,"is CALmodule a ptr? %d",sizeof(CALmodule));
				prg->imp.v_kbin[devnum] = (void*)(unsigned long long)calmod;

				DEBUG(__FILE__,__LINE__,
					"kbin[%d] = %p",devnum,prg->imp.v_kbin[devnum]);


				for(i=0;i<prg->imp.nclsym;i++) {

					strncpy(name,prg->imp.clstrtab+prg->imp.clsymtab[i].e_name,256);

					if (!strncmp(name,"main",5)) {
				
						CALfunc calfunc = 0;

						err = calModuleGetEntry(
							&calfunc,prg->ctx->imp.calctx[devnum],calmod,name);

						DEBUG(__FILE__,__LINE__,"calModuleGetEntry returned %d",err);

						prg->imp.v_ksym[devnum][i] = 0;

						prg->imp.v_kcall[devnum][i] 
							= (void*)(unsigned long long)calfunc;

					} else {

						CALname calname = 0;	

						err = calModuleGetName(
							&calname,prg->ctx->imp.calctx[devnum],calmod,name);

						DEBUG(__FILE__,__LINE__,"calModuleGetName '%s' returned %d",
							name,err);

						prg->imp.v_ksym[devnum][i] 
							= (void*)(unsigned long long)calname;

						prg->imp.v_kcall[devnum][i] = 0;

					}


				}

				for(i=0;i<prg->imp.nclsym;i++) {
					DEBUG(__FILE__,__LINE__,"clsym '%s' : ksym=%p kcall=%p",
						prg->imp.clstrtab+prg->imp.clsymtab[i].e_name,
						prg->imp.v_ksym[devnum][i],prg->imp.v_kcall[devnum][i]);

				}


			}


//			return(CL_BUILD_PROGRAM_FAILURE);
			return(CL_SUCCESS); /* XXX testing -DAR */


#endif

		} else if (!strncmp(prg->bin[i]+1,"ELF",3)) {

			WARN(__FILE__,__LINE__,"__do_build_program_from_binary:"
				" ELF format not presently supported");

			return(CL_BUILD_PROGRAM_FAILURE);

		} else if (!strncmp(prg->bin[i],"TEST",4)) {

			WARN(__FILE__,__LINE__,"__do_build_program_from_binary:" 
				" xanadu backdoor");

			prg->imp.nclsym = 1; /* XXX no symtab/strtab created, fix -DAR */
			prg->imp.clsymtab = 0;
			prg->imp.clstrtab = (char*)malloc(sizeof("\0__xanadu"));
			prg->imp.clstrtab[0] = '\0';
			strcpy(prg->imp.clstrtab+1,"__xanadu");

			prg->imp.nkrn = 1;

			prg->imp.kname = (char**)malloc(prg->imp.nkrn*sizeof(char*));
			prg->imp.kname[0] = prg->imp.clstrtab + 1;

			prg->imp.knarg = (cl_uint*)malloc(1*sizeof(cl_uint));
			prg->imp.knarg[0] = 3;

			/* XXX this assumes size and padding same per device, fix -DAR */
			prg->imp.karg_buf_sz = (size_t*)malloc(1*sizeof(size_t));
			prg->imp.karg_buf_sz[0] = 3*sizeof(size_t);

			prg->imp.karg_kind = (cl_uint**)malloc(1*sizeof(cl_uint*));
			prg->imp.karg_kind[0] = (cl_uint*)malloc(3*sizeof(cl_uint));
			prg->imp.karg_kind[0][0] = CLARG_KIND_GLOBAL;
			prg->imp.karg_kind[0][1] = CLARG_KIND_GLOBAL;
			prg->imp.karg_kind[0][2] = CLARG_KIND_GLOBAL;

			prg->imp.karg_sz = (size_t**)malloc(1*sizeof(size_t*));
			prg->imp.karg_sz[0] = (size_t*)malloc(3*sizeof(size_t));
			prg->imp.karg_sz[0][0] = sizeof(int*);
			prg->imp.karg_sz[0][1] = sizeof(int*);
			prg->imp.karg_sz[0][2] = sizeof(int*);


			prg->imp.v_ksym = (void***)malloc(prg->ndev*sizeof(void**));
			prg->imp.v_kcall = (void***)malloc(prg->ndev*sizeof(void**));

			for(i=0;i<prg->ndev;i++)
				prg->imp.v_ksym[i] = (void**)malloc(1*sizeof(void*));
			for(i=0;i<prg->ndev;i++)
				prg->imp.v_kcall[i] = (void**)malloc(1*sizeof(void*));

			prg->imp.v_ksym[devnum][0] = (void*)__xanadu;
			prg->imp.v_kcall[devnum][0] = (void*)__XCL_call_xanadu;


DEBUG(__FILE__,__LINE__,"__xanadu sym is at %p",__xanadu);

			return(CL_SUCCESS);

		} else {

			/* XXX should put some checks here to test if .ll file -DAR */

			DEBUG(__FILE__,__LINE__,"__do_build_program_from_binary:"
				" no marking, assume its a .ll file");

			WARN(__FILE__,__LINE__,"__do_build_program_from_binary:"
				" LL format not presently supported");

			return(CL_BUILD_PROGRAM_FAILURE);

		}

	}
	
}


int __do_check_compiler_available( cl_device_id devid )
{

	if (!__resolve_devid(devid,comp)) return(0);
//	if (!__resolve_devid(devid,link)) return(0); /* XXX compiler does link */

	return(1);
}


void __do_compile_and_link( cl_program prg, cl_device_id devid, int devnum )
{

	void*(*comp)(void*) = __resolve_devid(devid,comp);

	void*(*link)(void*) = __resolve_devid(devid,link);

	char options[__CLMAXSTR_BUFSZ];

	if (prg->build_options) {
		strncpy(options,prg->build_options[devnum],__CLMAXSTR_BUFSZ);
	} else {
		options[0] = '\0';
	}

	char* comp_log;
	char* link_log;

//	void* obj = comp(argc,argv,prg->src,comp_log);

//	void* bin = link(argc,argv,obj,link_log);

//	prg->bin[devnum] = bin;
	
}



int __do_find_kernel_in_program( cl_program prg, const char* kname )
{
	int k;

	for(k=0;k<prg->imp.nkrn;k++) {
		DEBUG(__FILE__,__LINE__,"compare |%s|%s\n",prg->imp.kname[k],kname);
		if (!strncmp(prg->imp.kname[k],kname,__CLMAXSTR_LEN)) break;
	}

	if (k==prg->imp.nkrn) return(-1);

	return(k);
}

static size_t get_global_size(uint d) 
{ return((__getvcdata())->workp->gtsz[d]); }

static void barrier( int flags )
{
   struct vc_data* data = __getvcdata();
   int vcid = data->vcid;

	if (!(setjmp(*(data->this_jbufp))))
		longjmp(*(data->next_jbufp),(vcid+1)%4+1);
}

static size_t get_global_id(uint d) {
   struct vc_data* data = __getvcdata();
   return(data->ltid[d] + data->workp->gtid[d]);
}




//extern cl_uint __get_global_id(cl_uint dim);

static
void __xanadu( cl_int* a, cl_int* b, cl_int* c)
{
	/* this testcode expects the data layout to be 128x16 */
	cl_uint ni = get_global_size(0);
	cl_uint i = get_global_id(0);
	cl_uint j = get_global_id(1);
	cl_uint cij = j*ni+i;
	printf("XANADU %d %d %d\n",ni,i,j);
	barrier(0);
	c[cij] = a[cij]*b[cij];
	return;
}

typedef void(*__XCL_func_xanadu_t)(cl_int* a, cl_int* b, cl_int* c);
static void __XCL_call_xanadu( void* p )
{
   DEBUG(__FILE__,__LINE__,"__XCL_call_xanadu");
   struct vcengine_data* edata = (struct vcengine_data*)p;
   struct vc_data* data = __getvcdata();
   int vcid = data->vcid;
   DEBUG(__FILE__,__LINE__,"vcore[%d] running\n",vcid);
   ++edata->vc_runc;
   ((__XCL_func_xanadu_t)(edata->funcp))(
      (void*)edata->pr_arg_vec[0],
      (void*)edata->pr_arg_vec[1],
      (void*)edata->pr_arg_vec[2]
   );
   --edata->vc_runc;
   DEBUG(__FILE__,__LINE__,"vcore[%d] halt\n",vcid);
}



