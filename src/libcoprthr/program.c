/* program.c
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

#include <dlfcn.h>

#include <CL/cl.h>

#include "xcl_structs.h"
#include "program.h"
#include "compiler.h"
#include "elf_cl.h"
#include "cmdcall.h" /* XXX this is only used for backdoor -DAR */
#include "ocl_types.h"
#include "printcl.h"

#include "coprthr_device.h"
#include "coprthr_program.h"

#define _GNU_SOURCE
#include <dlfcn.h>


void __do_create_program(cl_program prg) 
{
	printcl( CL_DEBUG "__do_create_program with ndev = %d",prg->ndev);

	prg->imp->v_kbin = (void**)calloc(prg->ndev,sizeof(void*));
	prg->imp->v_kbin_tmpfile = (char**)calloc(prg->ndev,sizeof(char*));

	prg->imp->v_ksyms = (struct _coprthr_ksyms_struct**)
		calloc(prg->ndev,sizeof(struct _coprthr_ksyms_struct*));

//	prg->prg1 = (struct coprthr1_program**)
//		calloc(prg->ndev,sizeof(struct coprthr1_program*));
//	int i;
//	for(i=0;i<prg->ndev;i++) 
//		prg->prg1[i] = (struct coprthr1_program*)
//			calloc(1,sizeof(struct coprthr1_program));
		
}


void __do_release_program(cl_program prg) 
{
	int i;
	for(i=0;i<prg->ndev;i++) {
		if (prg->imp->v_kbin[i]) dlclose(prg->imp->v_kbin[i]);
		if (prg->imp->v_kbin_tmpfile[i]) {
			unlink(prg->imp->v_kbin_tmpfile[i]);
			free(prg->imp->v_kbin_tmpfile[i]);
		}
	}

/*
	for(i=0;i<prg->ndev;i++) {
		if (prg->prg1->dlh) dlclose(prg->prg1->dlh);
		if (prg->prg1->dlfile) {
			unlink(prg->prg1->dlfile);
			free(prg->prg1->dlfile);
		}
		if (prg->prg1[i]) free(prg->prg1[i]);
		if (prg->prg1) free(prg->prg1);
	}
*/

}

cl_int __do_build_program_from_binary(
	cl_program prg,cl_device_id devid, cl_uint devnum
){ 

	printcl( CL_DEBUG "__do_build_program_from_binary");

	int i,j;


	printcl( CL_DEBUG "program: bin bin_sz %p %d",
//		prg->bin[devnum],prg->bin_sz[devnum]);
		prg->prg1[devnum]->bin,prg->prg1[devnum]->bin_sz);

   char tmpfile[] = "/tmp/xclXXXXXX";
   int fd = mkstemp(tmpfile);
//	write(fd,prg->bin[devnum],prg->bin_sz[devnum]);
	write(fd,prg->prg1[devnum]->bin,prg->prg1[devnum]->bin_sz);
	close(fd);

	void* h = dlopen(tmpfile,RTLD_LAZY);

//	Elf* e = (Elf*)prg->bin[devnum];
	Elf* e = (Elf*)prg->prg1[devnum]->bin;

	printcl( CL_DEBUG "is this elf? |%c %c %c %c",
		prg->prg1[devnum]->bin[1],
		prg->prg1[devnum]->bin[2],
		prg->prg1[devnum]->bin[3]);

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
	

	printcl( CL_DEBUG "clstrtab_sz %d\n",clstrtab_sz);


	char* ppp = (char*)cltextb;

	for(i=0;i<clsymtab_n;i++) {
		printcl( CL_DEBUG 
			"[%d] clsym |%s|",i,clstrtab+clsymtab[i].e_name);
	}

	for(i=0;i<clargtab_n;i++) {
		printcl( CL_DEBUG 
			"[%d] clarg |%s| (+%d)",i,clstrtab+clargtab[i].e_name,
			clargtab[i].e_name);
	}

	if (prg->imp->clsymtab == 0) {

		prg->imp->nclsym = clsymtab_n;
	
		__clone(prg->imp->clsymtab,clsymtab,clsymtab_n,struct clsymtab_entry);
		__clone(prg->imp->clargtab,clargtab,clargtab_n,struct clargtab_entry);
		__clone(prg->imp->clstrtab,clstrtab,clstrtab_sz,char);

		prg->imp->nkrn = clsymtab_n; /* XXX assumed, revisit in future -DAR */

		prg->imp->kname = (char**)malloc(prg->imp->nkrn*sizeof(char*));
		prg->imp->knarg = (cl_uint*)malloc(prg->imp->nkrn*sizeof(cl_uint));
		prg->imp->karg_buf_sz = (size_t*)malloc(prg->imp->nkrn*sizeof(size_t));
		prg->imp->karg_kind = (cl_uint**)malloc(prg->imp->nkrn*sizeof(cl_uint*));
		prg->imp->karg_sz = (size_t**)malloc(prg->imp->nkrn*sizeof(size_t*));


		for(i=0;i<clsymtab_n;i++) {

			prg->imp->kname[i] = prg->imp->clstrtab + prg->imp->clsymtab[i].e_name;
			unsigned int arg0 = prg->imp->clsymtab[i].e_arg0;
			int narg = 0; 
			int arg;
			for(arg=arg0;arg;arg=prg->imp->clargtab[arg].e_nxt,narg++);
			printcl( CL_DEBUG "%s has %d args\n",prg->imp->kname[i],narg);
			prg->imp->knarg[i] = narg;
			prg->imp->karg_kind[i] = (cl_uint*)malloc(narg*sizeof(cl_uint));
			prg->imp->karg_sz[i] = (size_t*)malloc(narg*sizeof(size_t));

			j = 0;
			size_t bufsz = 0;
			for(arg=arg0;arg;arg=prg->imp->clargtab[arg].e_nxt,j++) {

				size_t sz;
				size_t sz_ptr = sizeof(void*); /* XXX always true? -DAR */

				switch(prg->imp->clargtab[arg].e_datatype) {

					case TYPEID_CHAR:
					case TYPEID_UCHAR: 
						sz=1; 
						printcl( CL_DEBUG "arg type [%d] ~char",arg);
						break;
						
					case TYPEID_SHORT:
					case TYPEID_USHORT: 
						sz=2; 
						printcl( CL_DEBUG "arg type [%d] ~short",arg);
						break;

					case TYPEID_INT:
					case TYPEID_UINT: 
					case TYPEID_FLOAT:
						sz=4; 
						printcl( CL_DEBUG "arg type [%d] ~int",arg);
						break;

					case TYPEID_LONG: 
					case TYPEID_ULONG: 
					case TYPEID_DOUBLE:
						sz=8; 
						printcl( CL_DEBUG "arg type [%d] ~long",arg);
						break;

					case TYPEID_OPAQUE:
						sz=0; 
						printcl( CL_DEBUG "arg type [%d] ~opaque",arg);
						break;

					case TYPEID_VOID:
					default: sz=0; 
					printcl( CL_DEBUG "arg type [%d] ~void",arg);
					break;

				}

				printcl( CL_DEBUG "base arg_sz[%d] %d",arg,sz);

				sz *= prg->imp->clargtab[arg].e_vecn;
				sz *= prg->imp->clargtab[arg].e_arrn;
				
				printcl( CL_DEBUG "w/multiplicity arg_sz[%d] %d",arg,sz);

				printcl( CL_DEBUG "e_ptrc=%d e_addrspace=%d",
					prg->imp->clargtab[arg].e_ptrc,
					prg->imp->clargtab[arg].e_addrspace);

				if (prg->imp->clargtab[arg].e_ptrc == 0) {

					prg->imp->karg_kind[i][j] = CLARG_KIND_DATA;
					sz = sz;

				} else if (prg->imp->clargtab[arg].e_ptrc == 1) {

					if (prg->imp->clargtab[arg].e_addrspace == 0) { /* XXX promote */

						prg->imp->karg_kind[i][j] = CLARG_KIND_GLOBAL;
						sz = sz_ptr;

					} else if (prg->imp->clargtab[arg].e_addrspace == 1) {

						prg->imp->karg_kind[i][j] = CLARG_KIND_GLOBAL;
						sz = sz_ptr;

					} else if (prg->imp->clargtab[arg].e_addrspace == 2) {

						prg->imp->karg_kind[i][j] = CLARG_KIND_CONSTANT;
						sz = sz_ptr;

					} else if (prg->imp->clargtab[arg].e_addrspace == 3) {

						prg->imp->karg_kind[i][j] = CLARG_KIND_LOCAL;
						sz = sz_ptr;

					} else {

						prg->imp->karg_kind[i][j] = CLARG_KIND_UNDEFINED;
						sz = 0;

					}

				} else {

					prg->imp->karg_kind[i][j] = CLARG_KIND_UNDEFINED;
					sz = 0;

				}

				printcl( CL_DEBUG "after kind check arg_sz[%d] %d",arg,sz);

				prg->imp->karg_sz[i][j] = sz;
				bufsz += sz;

			}

			prg->imp->karg_buf_sz[i] = bufsz;

		}	


	}


	prg->imp->v_ksyms[devnum] = (struct _coprthr_ksyms_struct*)
		malloc(prg->imp->nkrn*sizeof(struct _coprthr_ksyms_struct));

	char name[1024];
	int err;

	cl_device_type devtype = __resolve_devid_devinfo(devid,devtype);

	if (devtype == CL_DEVICE_TYPE_CPU) {

		prg->imp->v_kbin[devnum] = h;
		prg->imp->v_kbin_tmpfile[devnum] = (char*)malloc(32); /* XXX */
		strncpy(prg->imp->v_kbin_tmpfile[devnum],tmpfile,32);

	}

	else prg->imp->v_kbin[devnum] = 0;

	printcl( CL_DEBUG "kbin[%d] = %p",devnum,prg->imp->v_kbin[devnum]);

	for(i=0;i<prg->imp->nclsym;i++) {

		strncpy(name,prg->imp->kname[i],1024);

		printcl( CL_DEBUG "devnum knum %d %d",devnum,i);

		if (__resolve_devid_devinfo(devid,devtype)==CL_DEVICE_TYPE_CPU) 
			__resolve_devid_devlink(devid,bind_ksyms)(
				&(prg->imp->v_ksyms[devnum][i]), h, prg->imp->kname[i]);
		else
			bzero(&prg->imp->v_ksyms[devnum][i],sizeof(struct _coprthr_ksyms_struct));

	}

	return(CL_SUCCESS); 
}


cl_int __do_build_program_from_source(
	cl_program prg,cl_device_id devid, cl_uint devnum
){ 

	printcl( CL_DEBUG "__do_build_program_from_source");

	compiler_t comp = (compiler_t)__resolve_devid_devcomp(devid,comp);

	if (!comp) return(CL_COMPILER_NOT_AVAILABLE);
	
	printcl( CL_DEBUG 
		"__do_build_program_from_source: compiler=%p",comp);	

	/* XXX should optimize JIT by checking for device equivalence -DAR */

	printcl( CL_DEBUG "build_options[%d] |%s|",
		devnum,prg->build_options[devnum]);

//	int err = comp( devid, prg->src,prg->src_sz, &prg->bin[devnum],
	int err = comp( devid, prg->prg1[devnum]->src,prg->prg1[devnum]->src_sz, 
//		&prg->bin[devnum],
//		&prg->bin_sz[devnum], prg->build_options[devnum],
//		&prg->build_log[devnum]);
		&prg->prg1[devnum]->bin,
		&prg->prg1[devnum]->bin_sz, prg->prg1[devnum]->build_opt,
		&prg->prg1[devnum]->build_log);

	if (!err) err = __do_build_program_from_binary(prg,devid,devnum);

	return((cl_int)err);
}


int __do_check_compiler_available( cl_device_id devid )
{

	if (!__resolve_devid_devcomp(devid,comp)) return(0);

	return(1);
}


int __do_find_kernel_in_program( cl_program prg, const char* kname )
{
	int k;

	for(k=0;k<prg->imp->nkrn;k++) {
		printcl( CL_DEBUG "compare |%s|%s\n",prg->imp->kname[k],kname);
		if (!strncmp(prg->imp->kname[k],kname,__CLMAXSTR_LEN)) break;
	}

	if (k==prg->imp->nkrn) return(-1);

	return(k);
}

int bind_ksyms_default( struct _coprthr_ksyms_struct* ksyms, void* h, char* kname )
{

	char name[1024];

	strncpy(name,kname,1024);
	ksyms->kthr = dlsym(h,name);
	printcl( CL_DEBUG "kthr %s -> %p", name,ksyms->kthr);

	strncpy(name,"__XCL_ser_",1024);
	strncat(name,kname,1024);
	ksyms->kthr2 = dlsym(h,name);
	printcl( CL_DEBUG "kthr2 %s -> %p", name,ksyms->kthr2);

	strncpy(name,"__XCL_call_",1024);
	strncat(name,kname,1024);
	ksyms->kcall = dlsym(h,name);
	printcl( CL_DEBUG "kcall %s -> %p", name,ksyms->kcall);

	strncpy(name,"__XCL_call2_",1024);
	strncat(name,kname,1024);
	ksyms->kcall2 = dlsym(h,name);
	printcl( CL_DEBUG "kcall2 %s -> %p", name,ksyms->kcall2);

}

