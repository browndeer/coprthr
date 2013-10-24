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

#include <string.h>
#include <dlfcn.h>

#include "program.h"
#include "compiler.h"
#include "elf_cl.h"
//#include "cmdcall.h" /* XXX this is only used for backdoor -DAR */
#include "ocl_types.h"
#include "printcl.h"

#include "coprthr_device.h"
#include "coprthr_program.h"
#include "device.h"

#define _GNU_SOURCE
#include <dlfcn.h>

#define __CLMAXSTR_LEN 1023
#define __CLMAXSTR_BUFSZ (__CLMAXSTR_LEN+1)

void __do_release_program_1(struct coprthr1_program* prg1) 
{
		if (prg1->dlh) dlclose(prg1->dlh);
		if (prg1->dlfile) {
			unlink(prg1->dlfile);
			free(prg1->dlfile);
		}
}

int bind_ksyms_default(struct _coprthr_ksyms_struct* ksyms,void* h,char* kname);

unsigned int __do_build_program_from_binary_1(
	struct coprthr1_program* prg1
){ 

	printcl( CL_DEBUG "__do_build_program_from_binary_1");

	int i,j;


	printcl( CL_DEBUG "program: bin bin_sz %p %d",
		prg1->bin,prg1->bin_sz);

   char tmpfile[] = "/tmp/xclXXXXXX";
   int fd = mkstemp(tmpfile);
	write(fd,prg1->bin,prg1->bin_sz);
	close(fd);

	void* h = dlopen(tmpfile,RTLD_LAZY);

	Elf* e = (Elf*)prg1->bin;

	printcl( CL_DEBUG "is this elf? |%c %c %c %c",
		prg1->bin[1],
		prg1->bin[2],
		prg1->bin[3]);

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

//	struct coprthr1_program* prg1 = prg1;

	prg1->nclsym = clsymtab_n;
	__clone(prg1->clsymtab,clsymtab,clsymtab_n,struct clsymtab_entry);
	__clone(prg1->clargtab,clargtab,clargtab_n,struct clargtab_entry);
	__clone(prg1->clstrtab,clstrtab,clstrtab_sz,char);


	/* XXX adding a section NOT conditional on devnum==0 */
	{
		prg1->nkrn = clsymtab_n; /* XXX assumed, revisit in future -DAR */

		prg1->kname = (char**)malloc(prg1->nkrn*sizeof(char*));
		prg1->knarg = (unsigned int*)malloc(prg1->nkrn*sizeof(unsigned int));
		prg1->karg_buf_sz = (size_t*)malloc(prg1->nkrn*sizeof(size_t));
		prg1->karg_kind 
			= (unsigned int**)malloc(prg1->nkrn*sizeof(unsigned int*));
		prg1->karg_sz = (size_t**)malloc(prg1->nkrn*sizeof(size_t*));


		for(i=0;i<clsymtab_n;i++) {

			prg1->kname[i] = prg1->clstrtab + prg1->clsymtab[i].e_name;
			unsigned int arg0 = prg1->clsymtab[i].e_arg0;
			int narg = 0; 
			int arg;
			for(arg=arg0;arg;arg=prg1->clargtab[arg].e_nxt,narg++);
			printcl( CL_DEBUG "%s has %d args",prg1->kname[i],narg);
			prg1->knarg[i] = narg;
			printcl( CL_DEBUG "narg set %d %d",i,prg1->knarg[i] );
			prg1->karg_kind[i] = (unsigned int*)malloc(narg*sizeof(unsigned int));
			prg1->karg_sz[i] = (size_t*)malloc(narg*sizeof(size_t));

			j = 0;
			size_t bufsz = 0;
			for(arg=arg0;arg;arg=prg1->clargtab[arg].e_nxt,j++) {

				size_t sz;
				size_t sz_ptr = sizeof(void*); /* XXX always true? -DAR */

				switch(prg1->clargtab[arg].e_datatype) {

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

				sz *= prg1->clargtab[arg].e_vecn;
				sz *= prg1->clargtab[arg].e_arrn;
				
				printcl( CL_DEBUG "w/multiplicity arg_sz[%d] %d",arg,sz);

				printcl( CL_DEBUG "e_ptrc=%d e_addrspace=%d",
					prg1->clargtab[arg].e_ptrc,
					prg1->clargtab[arg].e_addrspace);

				if (prg1->clargtab[arg].e_ptrc == 0) {

					prg1->karg_kind[i][j] = CLARG_KIND_DATA;
					sz = sz;

				} else if (prg1->clargtab[arg].e_ptrc == 1) {

					if (prg1->clargtab[arg].e_addrspace == 0) { /* XXX promote */

						prg1->karg_kind[i][j] = CLARG_KIND_GLOBAL;
						sz = sz_ptr;

					} else if (prg1->clargtab[arg].e_addrspace == 1) {

						prg1->karg_kind[i][j] = CLARG_KIND_GLOBAL;
						sz = sz_ptr;

					} else if (prg1->clargtab[arg].e_addrspace == 2) {

						prg1->karg_kind[i][j] = CLARG_KIND_CONSTANT;
						sz = sz_ptr;

					} else if (prg1->clargtab[arg].e_addrspace == 3) {

						prg1->karg_kind[i][j] = CLARG_KIND_LOCAL;
						sz = sz_ptr;

					} else {

						prg1->karg_kind[i][j] = CLARG_KIND_UNDEFINED;
						sz = 0;

					}

				} else {

					prg1->karg_kind[i][j] = CLARG_KIND_UNDEFINED;
					sz = 0;

				}

				printcl( CL_DEBUG "after kind check arg_sz[%d] %d",arg,sz);

				prg1->karg_sz[i][j] = sz;
				bufsz += sz;

			}

			prg1->karg_buf_sz[i] = bufsz;

		}	

	}

#if(0)
	if (devnum == 0) { /* XXX convert into a per-device check -DAR */

		prg->nkrn = clsymtab_n; /* XXX assumed, revisit in future -DAR */

//		prg->imp->kname = (char**)malloc(prg->nkrn*sizeof(char*));
//		prg->imp->knarg = (cl_uint*)malloc(prg->nkrn*sizeof(cl_uint));
//		prg->imp->karg_buf_sz = (size_t*)malloc(prg->nkrn*sizeof(size_t));
//		prg->imp->karg_kind = (cl_uint**)malloc(prg->nkrn*sizeof(cl_uint*));
//		prg->imp->karg_sz = (size_t**)malloc(prg->nkrn*sizeof(size_t*));


		for(i=0;i<clsymtab_n;i++) {

//			prg->imp->kname[i] = prg1->clstrtab + prg1->clsymtab[i].e_name;
			unsigned int arg0 = prg1->clsymtab[i].e_arg0;
			int narg = 0; 
			int arg;
			for(arg=arg0;arg;arg=prg1->clargtab[arg].e_nxt,narg++);
//			printcl( CL_DEBUG "%s has %d args\n",prg->imp->kname[i],narg);
//			prg->imp->knarg[i] = narg;
//			prg->imp->karg_kind[i] = (cl_uint*)malloc(narg*sizeof(cl_uint));
//			prg->imp->karg_sz[i] = (size_t*)malloc(narg*sizeof(size_t));

			j = 0;
			size_t bufsz = 0;
			for(arg=arg0;arg;arg=prg1->clargtab[arg].e_nxt,j++) {

				size_t sz;
				size_t sz_ptr = sizeof(void*); /* XXX always true? -DAR */

				switch(prg1->clargtab[arg].e_datatype) {

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

				sz *= prg1->clargtab[arg].e_vecn;
				sz *= prg1->clargtab[arg].e_arrn;
				
				printcl( CL_DEBUG "w/multiplicity arg_sz[%d] %d",arg,sz);

				printcl( CL_DEBUG "e_ptrc=%d e_addrspace=%d",
					prg1->clargtab[arg].e_ptrc,
					prg1->clargtab[arg].e_addrspace);

				if (prg1->clargtab[arg].e_ptrc == 0) {

//					prg->imp->karg_kind[i][j] = CLARG_KIND_DATA;
					sz = sz;

				} else if (prg1->clargtab[arg].e_ptrc == 1) {

					if (prg1->clargtab[arg].e_addrspace == 0) { /* XXX promote */

//						prg->imp->karg_kind[i][j] = CLARG_KIND_GLOBAL;
						sz = sz_ptr;

					} else if (prg1->clargtab[arg].e_addrspace == 1) {

//						prg->imp->karg_kind[i][j] = CLARG_KIND_GLOBAL;
						sz = sz_ptr;

					} else if (prg1->clargtab[arg].e_addrspace == 2) {

//						prg->imp->karg_kind[i][j] = CLARG_KIND_CONSTANT;
						sz = sz_ptr;

					} else if (prg1->clargtab[arg].e_addrspace == 3) {

//						prg->imp->karg_kind[i][j] = CLARG_KIND_LOCAL;
						sz = sz_ptr;

					} else {

//						prg->imp->karg_kind[i][j] = CLARG_KIND_UNDEFINED;
						sz = 0;

					}

				} else {

//					prg->imp->karg_kind[i][j] = CLARG_KIND_UNDEFINED;
					sz = 0;

				}

				printcl( CL_DEBUG "after kind check arg_sz[%d] %d",arg,sz);

//				prg->imp->karg_sz[i][j] = sz;
				bufsz += sz;

			}

//			prg->imp->karg_buf_sz[i] = bufsz;

		}	


	} else {

		/* XXX should put in a check against devnum=0 here -DAR */
		printcl( CL_WARNING 
			"no binary check against device 0 for multiple devices");

	}
#endif


	prg1->v_ksyms = (struct _coprthr_ksyms_struct*)
		malloc(prg1->nkrn*sizeof(struct _coprthr_ksyms_struct));



	char name[1024];
	int err;

//	cl_device_type devtype = __resolve_devid_devinfo(devid,devtype);
//
//	if (devtype == CL_DEVICE_TYPE_CPU) {

		prg1->dlh = h;
		prg1->dlfile = strdup(tmpfile);

//	} else {

//		prg1->dlh = 0;
//		prg1->dlfile = 0;

//	}

	printcl( CL_DEBUG "kbin = %p",prg1->dlh);

	for(i=0;i<prg1->nkrn;i++) {

//		strncpy(name,prg->imp->kname[i],1024);
		strncpy(name,prg1->kname[i],1024);

		printcl( CL_DEBUG "devnum knum %d",i);

//		if (__resolve_devid_devinfo(devid,devtype)==CL_DEVICE_TYPE_CPU) {

//			__resolve_devid_devlink(devid,bind_ksyms)(
//				&(prg1->v_ksyms[i]), h, prg1->kname[i]);
			bind_ksyms_default( &(prg1->v_ksyms[i]), h, prg1->kname[i]);

//		} else {
//			bzero(&prg1]->v_ksyms[i],
//				sizeof(struct _coprthr_ksyms_struct));
//		}

	}

	return(0); 
}

int bind_ksyms_default( 
	struct _coprthr_ksyms_struct* ksyms, void* h, char* kname )
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


void* coprthr_devcompile( struct coprthr_device* dev, char* src, size_t len, char* opt, char** log )
{
	printcl( CL_DEBUG "coprthr_devcompile");

	if (dev) {

		compiler_t comp = (compiler_t)dev->devcomp->comp;
		
		if (!comp) return(0);
	
		printcl( CL_DEBUG "coprthr_compile: compiler=%p",comp);	

		printcl( CL_DEBUG "build_opt |%s|", opt);

		struct coprthr1_program* prg1
			= (struct coprthr1_program*)malloc(sizeof(struct coprthr1_program));

		prg1->src = src;
		prg1->src_sz = len;
		prg1->build_opt = opt;

		int err = comp( 0, prg1->src,prg1->src_sz, 
			&prg1->bin, &prg1->bin_sz, prg1->build_opt, &prg1->build_log);

		if (!err) err = __do_build_program_from_binary_1(prg1);

		return(prg1);

	} else return(0);

}


void* coprthr_compile( int dd, char* src, size_t len, char* opt, char** log )
{
	printcl( CL_DEBUG "coprthr_compile");

	if (dd < 256 && __ddtab[dd]) 
		return coprthr_devcompile(__ddtab[dd],src,len,opt,log);
	else
		return 0;
}


void* coprthr_devlink( struct coprthr_device* dev, struct coprthr1_program* prg1, const char* kname )
{
	int k;

	for(k=0;k<prg1->nkrn;k++) {
		printcl( CL_DEBUG "compare |%s|%s\n",prg1->kname[k],kname);
		if (!strncmp(prg1->kname[k],kname,__CLMAXSTR_LEN)) break;
	}

	if (k==prg1->nkrn) return((void*)-1);

	struct coprthr1_kernel* krn1 = (struct coprthr1_kernel*)
		malloc(sizeof(struct coprthr1_kernel));

	printcl( CL_DEBUG "coprthr_link: krn1 %p",krn1);

	krn1->prg1 = prg1;
	krn1->knum= k;
	printcl( CL_DEBUG "HERE %d",krn1->prg1->knarg[0]);
	__do_create_kernel_1(krn1);

	printcl( CL_DEBUG "HERE %p",krn1->arg_buf);

	return(krn1);
}

void* coprthr_link( int dd, struct coprthr1_program* prg1, const char* kname )
{
	if (dd < 256 && __ddtab[dd]) 
		return coprthr_devlink(__ddtab[dd],prg1,kname);
	else
		return 0;
}

