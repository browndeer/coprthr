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
#include "ocl_types.h"


void __do_create_program(cl_program prg) 
{
	DEBUG(__FILE__,__LINE__,"__do_create_program with ndev = %d",prg->ndev);

	prg->imp.v_kbin = (void**)calloc(prg->ndev,sizeof(void*));
	prg->imp.v_ksym = (void***)calloc(prg->ndev,sizeof(void**));
	prg->imp.v_kcall = (void***)calloc(prg->ndev,sizeof(void**));
}

void __do_release_program(cl_program prg) {}

cl_int __do_build_program_from_binary(
	cl_program prg,cl_device_id devid, cl_uint devnum
){ 

	DEBUG(__FILE__,__LINE__,"__do_build_program_from_binary");

	int i,j;

/*
	compiler_t comp = (compiler_t)__resolve_devid(devid,comp);
	
	DEBUG(__FILE__,__LINE__,
		"__do_build_program_from_source: compiler=%p",comp);	

	struct _elf_data* edata = (struct _elf_data*)comp(
		devid,
		prg->src,prg->src_sz,
		&prg->bin[devnum],&prg->bin_sz[devnum],
		prg->build_options[devnum],&prg->build_log[devnum]
	);
*/

	DEBUG2("program: bin bin_sz %p %d",prg->bin[devnum],prg->bin_sz[devnum]);

   char tmpfile[] = "/tmp/xclXXXXXX";
   int fd = mkstemp(tmpfile);
	write(fd,prg->bin[devnum],prg->bin_sz[devnum]);
	close(fd);

	//void* h = edata->dlh;
	void* h = dlopen(tmpfile,RTLD_LAZY);

	//Elf* e = (Elf*)edata->map;
	Elf* e = (Elf*)prg->bin[devnum];

#if defined(__x86_64__)
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)e;
	Elf64_Shdr* shdr = (Elf64_Shdr*)((intptr_t)e + ehdr->e_shoff);
#elif defined(__i386__) || defined(__arm__)
   Elf32_Ehdr* ehdr = (Elf32_Ehdr*)e;
   Elf32_Shdr* shdr = (Elf32_Shdr*)((intptr_t)e + ehdr->e_shoff);
#else
#error unsupported architecture
#endif
	char* shstrtab = (char*)((intptr_t)e + shdr[ehdr->e_shstrndx].sh_offset);

	size_t clstrtab_sz = 0;
	char* clstrtab = 0;
	int clsymtab_n = 0;
	struct clsymtab_entry* clsymtab = 0;
	int clargtab_n = 0;
	struct clargtab_entry* clargtab = 0;
	char* cltextb = 0;

	for(i=0;i<ehdr->e_shnum;i++,shdr++) {
		DEBUG2("section name |%s|",shstrtab+shdr->sh_name);
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
	

	DEBUG(__FILE__,__LINE__,"clstrtab_sz %d\n",clstrtab_sz);


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

					case TYPEID_CHAR:
					case TYPEID_UCHAR: 
						sz=1; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] ~char",arg);
						break;
						
					case TYPEID_SHORT:
					case TYPEID_USHORT: 
						sz=2; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] ~short",arg);
						break;

					case TYPEID_INT:
					case TYPEID_UINT: 
					case TYPEID_FLOAT:
						sz=4; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] ~int",arg);
						break;

					case TYPEID_LONG: 
					case TYPEID_ULONG: 
					case TYPEID_DOUBLE:
						sz=8; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] ~long",arg);
						break;

					case TYPEID_OPAQUE:
						sz=0; 
						DEBUG(__FILE__,__LINE__,"arg type [%d] ~opaque",arg);
						break;

					case TYPEID_VOID:
					default: sz=0; 
					DEBUG(__FILE__,__LINE__,"arg type [%d] ~void",arg);
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

		strncpy(name,prg->imp.kname[i],1024);

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


cl_int __do_build_program_from_source(
	cl_program prg,cl_device_id devid, cl_uint devnum
){ 

	DEBUG(__FILE__,__LINE__,"__do_build_program_from_source");

	compiler_t comp = (compiler_t)__resolve_devid(devid,comp);
	
	DEBUG(__FILE__,__LINE__,
		"__do_build_program_from_source: compiler=%p",comp);	

	/* XXX should optimize JIT by checking for device equivalence -DAR */

	DEBUG2("build_options[%d] |%s|",devnum,prg->build_options[devnum]);

	struct _elf_data* edata = (struct _elf_data*)comp(
		devid,
		prg->src,prg->src_sz,
		&prg->bin[devnum],&prg->bin_sz[devnum],
		prg->build_options[devnum],&prg->build_log[devnum]
	);

	return __do_build_program_from_binary(prg,devid,devnum);

}



int __do_check_compiler_available( cl_device_id devid )
{

	if (!__resolve_devid(devid,comp)) return(0);

	return(1);
}


/* XXX is this even used? -DAR */
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


