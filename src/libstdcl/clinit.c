/* clinit.c
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


/* XXX to do, add err code checks, other safety checks * -DAR */
/* XXX to do, clvplat_destroy should automatically release all txts -DAR */

#ifdef _WIN64
#include "fix_windows.h"
#else
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>



#include <CL/cl.h>

#include "util.h"
#include "clinit.h"
#include "clcontext.h"


#ifdef DEFAULT_OPENCL_PLATFORM
#define DEFAULT_PLATFORM_NAME DEFAULT_OPENCL_PLATFORM
#else
#define DEFAULT_PLATFORM_NAME "ATI"
#endif


/* 
 * global CONTEXT structs 
 */

LIBSTDCL_API CONTEXT* stddev = 0;
LIBSTDCL_API CONTEXT* stdcpu = 0;
LIBSTDCL_API CONTEXT* stdgpu = 0;
LIBSTDCL_API CONTEXT* stdrpu = 0;

int procelf_fd = -1;
void* procelf = 0;
size_t procelf_sz = 0;

struct _proc_cl_struct _proc_cl = { 0,0, 0,0, 0,0, 0,0, 0,0 };

char* __log_automatic_kernels_filename = 0;

#define min(a,b) ((a<b)?a:b)

static int __getenv_token( 
	const char* name, const char* token, char* value, size_t n
);

//static cl_platform_id _select_platformid( 
//	cl_uint nplatforms, cl_platform_id* platforms, const char* env_var
//);


/* 
 * libstdcl initialization ctor and dtor 
 */

#ifdef _WIN64
//LIBSTDCL_API void _libstdcl_init()
void _libstdcl_init()
#else
void __attribute__((__constructor__)) _libstdcl_init()
#endif
{

	int i;
	int n;

	cl_platform_id platformid;

	int enable;
	cl_uint ndev;
	char env_max_ndev[256];
	int lock_key;


	DEBUG(__FILE__,__LINE__,"_libstdcl_init() called");

	/*
	 * set _proc_cl struct
 	 */

#ifndef _WIN64

	pid_t pid = getpid();
	DEBUG(__FILE__,__LINE__,"_libstdcl_init: pid=%d\n",pid);

	char procexe[256];
	snprintf(procexe,256,"/proc/%d/exe",pid);

	struct stat st;
	if (stat(procexe,&st)) ERROR(__FILE__,__LINE__,"stat procexe failed");

	procelf_fd = open(procexe,O_RDONLY);

	if (procelf_fd < 0) { 

		ERROR(__FILE__,__LINE__,"opening procexe failed");

	} else {

		procelf = mmap(0,st.st_size,PROT_READ,MAP_PRIVATE,procelf_fd,0);
		procelf_sz = st.st_size;

		DEBUG(__FILE__,__LINE__,"_libstdcl_init: procelf size %d bytes\n",
			st.st_size);

		// printf("procelf ptr %p %d\n",procelf,errno); fflush(stdout);

#if defined(__x86_64__)
		Elf64_Ehdr* elf = (Elf64_Ehdr*)procelf;
		Elf64_Shdr* p_shdr = procelf + elf->e_shoff;
#elif defined(__i386__)
		Elf32_Ehdr* elf = (Elf32_Ehdr*)procelf;
		Elf32_Shdr* p_shdr = procelf + elf->e_shoff;
#endif

		char buf[EI_NIDENT+1];
		strncpy(buf,elf->e_ident,EI_NIDENT);
		DEBUG(__FILE__,__LINE__,"_libstdcl_init: e_ident|%s|\n",buf);

		// printf("number of section headers %d\n",elf->e_shnum);

		char* shstr = (char*)procelf + p_shdr[elf->e_shstrndx].sh_offset;
	
		// printf("sh str table index %d\n",elf->e_shstrndx);
	
		// p_shdr += 1; /* skip first section */
	
		for(n=1;n<elf->e_shnum;n++) {

			DEBUG(__FILE__,__LINE__,
				"section offset in img %d bytes (%s) size %d\n", 
				p_shdr->sh_offset,
				shstr+p_shdr->sh_name,p_shdr->sh_size
			);

			if (!strncmp(shstr+p_shdr->sh_name,".clprgs",7)) {

				_proc_cl.clprgs=(struct clprgs_entry*)(procelf+p_shdr->sh_offset);
				_proc_cl.clprgs_n=p_shdr->sh_size/__clprgs_entry_sz;

			} else if (!strncmp(shstr+p_shdr->sh_name,".cltexts",8)) {

				_proc_cl.cltexts = (char*)(procelf + p_shdr->sh_offset);
				_proc_cl.cltexts_sz = p_shdr->sh_size;

			} else if (!strncmp(shstr+p_shdr->sh_name,".clprgb",7)) {

				_proc_cl.clprgb=(struct clprgb_entry*)(procelf+p_shdr->sh_offset);
				_proc_cl.clprgb_n=p_shdr->sh_size/__clprgb_entry_sz;

			} else if (!strncmp(shstr+p_shdr->sh_name,".cltextb",8)) {

				_proc_cl.cltextb = (char*)(procelf + p_shdr->sh_offset);
				_proc_cl.cltextb_sz = p_shdr->sh_size;

			} else if (!strncmp(shstr+p_shdr->sh_name,".clstrtab",9)) {

				_proc_cl.clstrtab = (char*)(procelf + p_shdr->sh_offset);
				_proc_cl.clstrtab_sz = p_shdr->sh_size;

			}
		
			p_shdr += 1;
		}

	}

	DEBUG(__FILE__,__LINE__,"_libstdcl_init: procelf cl sections:"
		" %p %p %p %p %p\n",
		_proc_cl.clprgs,
		_proc_cl.cltexts,
		_proc_cl.clprgb,
		_proc_cl.cltextb,_proc_cl.clstrtab
	);

#endif

#if(0)
	/*
	 * get platform information
	 */

	cl_platform_id* platforms = 0;
   cl_uint nplatforms;

   char info[1024];

   clGetPlatformIDs(0,0,&nplatforms);

//printf("XXX %d\n",nplatforms);

	if (nplatforms) {

		platforms = (cl_platform_id*)malloc(nplatforms*sizeof(cl_platform_id));
   	clGetPlatformIDs(nplatforms,platforms,0);

		for(i=0;i<nplatforms;i++) {

			char info[1024];

			DEBUG(__FILE__,__LINE__,"_libstdcl_init: available platform:");

			clGetPlatformInfo(platforms[i],CL_PLATFORM_PROFILE,1024,info,0);
			DEBUG(__FILE__,__LINE__,
				"_libstdcl_init: [%p]CL_PLATFORM_PROFILE=%s",platforms[i],info);

			clGetPlatformInfo(platforms[i],CL_PLATFORM_VERSION,1024,info,0);
			DEBUG(__FILE__,__LINE__,
				"_libstdcl_init: [%p]CL_PLATFORM_VERSION=%s",platforms[i],info);

			clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1024,info,0);
			DEBUG(__FILE__,__LINE__,
				"_libstdcl_init: [%p]CL_PLATFORM_NAME=%s",platforms[i],info);

			clGetPlatformInfo(platforms[i],CL_PLATFORM_VENDOR,1024,info,0);
			DEBUG(__FILE__,__LINE__,
				"_libstdcl_init: [%p]CL_PLATFORM_VENDOR=%s",platforms[i],info);

			clGetPlatformInfo(platforms[i],CL_PLATFORM_EXTENSIONS,1024,info,0);
			DEBUG(__FILE__,__LINE__,
				"_libstdcl_init: [%p]CL_PLATFORM_EXTENSIONS=%s",platforms[i],info);

		}

	} else {

		WARN(__FILE__,__LINE__,
			"_libstdcl_init: no platforms found, continue and hope for the best");

	}
#endif



	/*
	 * initialize stddev (all CL devices)
	 */

	DEBUG(__FILE__,__LINE__,"clinit: initialize stddev");

/*
	if (!__getenv_token("STDDEV",0,env_max_ndev,256)) {
		enable = ndev = atoi(env_max_ndev);
	} else {
		ndev = 0;
		enable = 1;
		//enable = 0;
	}

	stddev = 0;

	if (enable) {

//		platformid = _select_platformid(nplatforms,platforms,"STDDEV");
		char name[256];
		__getenv_token("STDDEV","platform_name",name,256);

//		if (platformid != (cl_platform_id)(-1)) {

//			DEBUG(__FILE__,__LINE__,
//				"_libstdcl_init: stddev platformid %p",platformid);

//			stddev = clcontext_create(platformid,CL_DEVICE_TYPE_ALL,ndev,0);
			stddev = clcontext_create(name,CL_DEVICE_TYPE_ALL,ndev,0,0);

//		} 

	}
*/

	stddev = 0;
	ndev = 0; /* this is a special case that implies all available -DAR */
	enable = 1;
	lock_key = 0;

	if (getenv("STDDEV")) enable = atoi(getenv("STDDEV"));

	if (enable) {

		char name[256];
		if (getenv("STDDEV_PLATFORM_NAME"))
			strncpy(name,getenv("STDDEV_PLATFORM_NAME"),256);
		else name[0]='\0';

		if (getenv("STDDEV_MAX_NDEV"))
			ndev = atoi(getenv("STDDEV_MAX_NDEV"));

		if (getenv("STDDEV_LOCK"))
			lock_key = atoi(getenv("STDDEV_LOCK"));

		stddev = clcontext_create(name,CL_DEVICE_TYPE_ALL,ndev,0,lock_key);

	}

	DEBUG(__FILE__,__LINE__,"back from clcontext_create\n");






	/*
	 * initialize stdcpu (all CPU CL devices)
	 */

	DEBUG(__FILE__,__LINE__,"clinit: initialize stdcpu");

/*
	if (!__getenv_token("STDCPU",0,env_max_ndev,256)) {
		enable = ndev = atoi(env_max_ndev);
	} else {
		ndev = 0;
		enable = 1;
		//enable = 0;
	}

	stdcpu = 0;

	if (enable) {

//		platformid = _select_platformid(nplatforms,platforms,"STDCPU");
		char name[256];
		__getenv_token("STDCPU","platform_name",name,256);

//		if (platformid != (cl_platform_id)(-1)) {

//			DEBUG(__FILE__,__LINE__,
//				"_libstdcl_init: stdcpu platformid %p",platformid);

//			stdcpu = clcontext_create(platformid,CL_DEVICE_TYPE_CPU,ndev,0);
			stdcpu = clcontext_create(name,CL_DEVICE_TYPE_CPU,ndev,0,0);

//		}

	}
*/

	stdcpu = 0;
	ndev = 0; /* this is a special case that implies all available -DAR */
	enable = 1;
	lock_key = 0;

	if (getenv("STDCPU")) enable = atoi(getenv("STDCPU"));

	if (enable) {

		char name[256];
		if (getenv("STDCPU_PLATFORM_NAME"))
			strncpy(name,getenv("STDCPU_PLATFORM_NAME"),256);
		else name[0]='\0';

		if (getenv("STDCPU_MAX_NDEV"))
			ndev = atoi(getenv("STDCPU_MAX_NDEV"));

		if (getenv("STDCPU_LOCK"))
			lock_key = atoi(getenv("STDCPU_LOCK"));

		stdcpu = clcontext_create(name,CL_DEVICE_TYPE_CPU,ndev,0,lock_key);

	}

	DEBUG(__FILE__,__LINE__,"back from clcontext_create\n");




	/*
	 * initialize stdgpu (all GPU CL devices)
	 */

	DEBUG(__FILE__,__LINE__,"clinit: initialize stdgpu");

/*
	if (!__getenv_token("STDGPU",0,env_max_ndev,256)) {
		enable = ndev = atoi(env_max_ndev);
	} else {
		ndev = 0;
		enable = 1;
	}
*/

	stdgpu = 0;
	ndev = 0; /* this is a special case that implies all available -DAR */
	enable = 1;
	lock_key = 0;

	if (getenv("STDGPU")) enable = atoi(getenv("STDGPU"));

	if (enable) {

		char name[256];
		if (getenv("STDGPU_PLATFORM_NAME"))
			strncpy(name,getenv("STDGPU_PLATFORM_NAME"),256);
		else name[0]='\0';

		if (getenv("STDGPU_MAX_NDEV"))
			ndev = atoi(getenv("STDGPU_MAX_NDEV"));

		if (getenv("STDGPU_LOCK"))
			lock_key = atoi(getenv("STDGPU_LOCK"));

		stdgpu = clcontext_create(name,CL_DEVICE_TYPE_GPU,ndev,0,lock_key);

	}

	DEBUG(__FILE__,__LINE__,"back from clcontext_create\n");


	/*
	 * initialize stdrpu (all RPU CL devices)
	 */

/* XXX old style, need to update -DAR
	if (!__getenv_token("STDRPU",0,env_max_ndev,256)) {
		enable = ndev = atoi(env_max_ndev);
	} else {
		ndev = 0;
		enable = 1;
	}

	stdrpu = 0;

	if (enable) {

		platformid = _select_platformid(nplatforms,platforms,"STDRPU");

		if (platformid != (cl_platform_id)(-1)) {

			DEBUG(__FILE__,__LINE__,
				"_libstdcl_init: stdrpu platformid %p",platformid);

			stdrpu = clcontext_create(platformid,CL_DEVICE_TYPE_RPU,ndev,0);

		}

	}
*/


	char buf[256];
	if (!__getenv_token("COPRTHR","log_automatic_kernels",buf,256)) {
		__log_automatic_kernels_filename = (char*)malloc(256+6);
		if (!strncasecmp(buf,"log_automatic_kernels",256)) {
			snprintf(
				__log_automatic_kernels_filename,256+6,
				"coprthr.autokern.log.%d",getpid());
		} else {
			snprintf(__log_automatic_kernels_filename,256+6,"%s.%d",buf,getpid());
		}
		DEBUG(__FILE__,__LINE__,"log_automatic_kernels written to %s",
			__log_automatic_kernels_filename);
	}

	clUnloadCompiler();	

}


#ifdef _WIN64
void _libstdcl_fini()
#else
void __attribute__((__destructor__)) _libstdcl_fini()
#endif
{
	DEBUG(__FILE__,__LINE__,"_libstdcl_fini() called");

	if (stdgpu) clcontext_destroy(stdgpu);

#ifndef _WIN64
	munmap(procelf,procelf_sz);
	close(procelf_fd);
#endif

/* Dangerous, order of destructors not well-controled, just let them die -DAR 
	if (stddev) clcontext_destroy(stddev);
	if (stdcpu) clcontext_destroy(stdcpu);
	if (stdgpu) clcontext_destroy(stdgpu);
//	if (stdrpu) clcontext_destroy(stdrpu);
*/

}


void _assert_proto_stub(void) {  }


static int 
__getenv_token( const char* name, const char* token, char* value, size_t n )
{
	char* envstr = (char*)getenv(name);

	*value  = '\0';

	if (!envstr) return(1); 

	char* ptr;
	char* clause = strtok_r(envstr,":",&ptr);

   while (clause) {

		char* sep = strchr(clause,'=');

      if (sep) {

			if (token && !strncasecmp(token,clause,strlen(token))) {

				strncpy(value,sep+1,min(strlen(sep+1)+1,n));
				return(0);

			}

      } else if (!token) { /* this is a legacy case -DAR */

			strncpy(value,clause,min(strlen(clause)+1,n));
			return(0);

      } else {

			if ( strlen(token) == strlen(clause) 
				&& !strncasecmp(token,clause,strlen(token))) {

				strncpy(value,clause,strlen(clause)+1);
				return(0);

			}

		}

      clause = strtok_r(0,":",&ptr);
   }

	return(2);
	
}


static cl_platform_id
_select_platformid( 
	cl_uint nplatform, cl_platform_id* platforms, const char* env_var 
)
{

	DEBUG(__FILE__,__LINE__,"_select_platformid: env_var |%s|",env_var);

	int i,j;
	char name[256];
	__getenv_token(env_var,"platform_name",name,256);

	if (name[0] == '\0') {

		strcpy(name,DEFAULT_PLATFORM_NAME);

		DEBUG(__FILE__,__LINE__, 
			"_select_platformid: use default platform_name |%s|",name);

	} else {

		DEBUG(__FILE__,__LINE__, 
			"_select_platformid: environment platform_name |%s|",name);

	}

	if (platforms) for(i=0;i<nplatform;i++) {

		char info[1024];
		clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1024,info,0);

//		DEBUG(__FILE__,__LINE__,
//			"_select_platformid: [%p]CL_PLATFORM_PROFILE=%s",platforms[i],info);

		DEBUG(__FILE__,__LINE__,"_select_platformid: compare |%s|%s|",name,info);

		if (!strncasecmp(name,info,strlen(name))) {

			DEBUG(__FILE__,__LINE__,"_select_platformid: "
				" match %s %s",name,info);

			return(platforms[i]);

		}

	}

	if (nplatform > 0) return(platforms[0]); /* default to first one */

	return((cl_platform_id)(-1));	/* none found */

}





