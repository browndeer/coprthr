/* clcontext.c
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

/* XXX to do, add err code checks, other safety checks * -DAR */
/* XXX to do, clvplat_destroy should automatically release all txts -DAR */

#ifdef _WIN64
#include "fix_windows.h"
#else
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <CL/cl.h>

#include "util.h"

#define __STDCL__
#include "clinit.h"
#include "clfcn.h"
#include "clcontext.h"
#include "printcl.h"
#include "clerrno.h"

#ifdef DEFAULT_OPENCL_PLATFORM
#define DEFAULT_PLATFORM_NAME DEFAULT_OPENCL_PLATFORM
#else
#define DEFAULT_PLATFORM_NAME "AMD"
#endif

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif

#ifndef _WIN64
struct __ctx_lock_struct {
	unsigned int magic;
	unsigned int key;
	pthread_mutex_t mtx;
	int refc;
	char __pad[
		64 - 2*sizeof(unsigned int) - sizeof(pthread_mutex_t) - sizeof(int)
	];
};

struct __ctx_lock_struct* __ctx_lock = 0;
#endif


static cl_platform_id
__get_platformid(
 cl_uint nplatform, cl_platform_id* platforms, const char* platform_name
);

#ifdef _WIN64
#define CLELF_PLATFORM_CODE_AMDAPP	1
#define CLELF_PLATFORM_CODE_NVIDIA	2
#define CLELF_PLATFORM_CODE_COPRTHR	3
#define CLELF_PLATFORM_CODE_INTEL	4
static int clelf_platform_code( char* name )
{
	if (!strncasecmp(name,"AMD",3)) return(CLELF_PLATFORM_CODE_AMDAPP);
	else if (!strncasecmp(name,"Nvidia",6)) return(CLELF_PLATFORM_CODE_NVIDIA);
	else if (!strncasecmp(name,"coprthr",7)) return(CLELF_PLATFORM_CODE_COPRTHR);
	else if (!strncasecmp(name,"Intel",5)) return(CLELF_PLATFORM_CODE_INTEL);
	return(0);
}
#endif


/* XXX note that presently ndevmax is ignored -DAR */

LIBSTDCL_API CONTEXT* 
clcontext_create( 
	const char* platform_select, 
	int devtyp, 
	size_t ndevmax,
	cl_context_properties* ctxprop_ext, 
	int lock_key
)
{

	int n;
	int err = 0;
	int i,j;
	size_t devlist_sz;
	CONTEXT* cp = 0;
	cl_platform_id* platforms = 0;
	cl_uint nplatforms;
	char info[1024];
	cl_platform_id platformid = 0;
	int nctxprop = 0;
	cl_context_properties* ctxprop;
	size_t sz;
	cl_uint ndev = 0;
	cl_command_queue_properties prop = 0;

	printcl( CL_DEBUG "clcontext_create() called");


	/***
	 *** allocate CONTEXT struct
	 ***/

	printcl( CL_DEBUG "clcontext_create: sizeof CONTEXT %d",sizeof(CONTEXT));

	assert(sizeof(CONTEXT)<getpagesize());
#ifdef _WIN64
	cp = (CONTEXT*)_aligned_malloc(sizeof(CONTEXT),getpagesize());
	if (!cp) {
		printcl( CL_WARNING "clcontext_create: memalign failed");
	}
#else
	if (posix_memalign((void**)&cp,getpagesize(),sizeof(CONTEXT))) {
		printcl( CL_WARNING "clcontext_create: posix_memalign failed");
	}
#endif

	printcl( CL_DEBUG "clcontext_create: context_ptr=%p",cp);
	
	if ((intptr_t)cp & (getpagesize()-1)) {
		printcl( CL_ERR "clcontext_create: fatal error: unaligned context_ptr");
		__set_clerrno(-1);
		exit(-1);
	}

//	if (!cp) { errno=ENOMEM; return(0); }
	if (!cp) { __set_clerrno(ENOMEM); return(0); }

	

   /***
    *** get platform id
	 *** 
	 *** New policy:
	 *** 	- User may specify a comma separated list of platform names by setting
	 ***	  the env var STD*_PLATFORM_NAME .  
	 ***	- The first platform to match this prioritized list that supports
	 ***	  at least one device is selected.
	 ***	- If no such platform is found, the platform that supports the most
	 ***	  devices is selected.
	 ***	- If no platform is found supporting at least 1 device, return 0.
    ***/

   err = clGetPlatformIDs(0,0,&nplatforms);
	__set_oclerrno(err);

   if (nplatforms > 0) {

      platforms = (cl_platform_id*)malloc(nplatforms*sizeof(cl_platform_id));
      err = clGetPlatformIDs(nplatforms,platforms,0);
		__set_oclerrno(err);

      for(i=0;i<nplatforms;i++) { 

         char info[1024];

         printcl( CL_DEBUG "clcontext_create: available platform:");

         err = clGetPlatformInfo(platforms[i],CL_PLATFORM_PROFILE,1024,info,0);
			__set_oclerrno(err);

         printcl( CL_DEBUG "clcontext_create: [%p]CL_PLATFORM_PROFILE=%s",
				platforms[i],info);

         err = clGetPlatformInfo(platforms[i],CL_PLATFORM_VERSION,1024,info,0);
			__set_oclerrno(err);
			printcl( CL_DEBUG "clcontext_create: [%p]CL_PLATFORM_VERSION=%s",
				platforms[i],info);

         err = clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1024,info,0);
			__set_oclerrno(err);
			printcl( CL_DEBUG "clcontext_create: [%p]CL_PLATFORM_NAME=%s",
				platforms[i],info);

         err = clGetPlatformInfo(platforms[i],CL_PLATFORM_VENDOR,1024,info,0);
			__set_oclerrno(err);
			printcl( CL_DEBUG "clcontext_create: [%p]CL_PLATFORM_VENDOR=%s",
				platforms[i],info);

         err = clGetPlatformInfo(
				platforms[i],CL_PLATFORM_EXTENSIONS,1024, info,0);
			__set_oclerrno(err);
			printcl( CL_DEBUG "clcontext_create: [%p]CL_PLATFORM_EXTENSIONS=%s",
				platforms[i],info);

      }

   } else {

      printcl( CL_WARNING "clcontext_create: no platforms found!");

		return((CONTEXT*)0);

   }



	/* XXX assume no more than 8 platforms availabile, this is reality -DAR */
 
	char* select_name[8] = { 0,0,0,0,0,0,0,0 };
	int select_code[8] = { 0,0,0,0,0,0,0,0 };
	cl_platform_id select_id[8] = { 0,0,0,0,0,0,0,0 };
	char buf[512];
	char* tmp_ptr;
	int nselect = 0;
	if (platform_select) {
		strncpy(buf,platform_select,512);
		char* tok = strtok_r(buf, ",", &tmp_ptr);
		if (tok) select_name[nselect++] = tok;
		while (tok && nselect < 8) { 
			tok = strtok_r(0, ",", &tmp_ptr); 
			if (tok) select_name[nselect++] = tok;
		}
		for(i=0; i< nselect; i++) {
			if (!select_name[i]) break;
			DEBUG(__FILE__,__LINE__,
				"clcontext_create: platform priority [%d] |%s|",
				i,select_name[i]);
		}
	}

	for(i=0; i< nselect; i++) 
		select_code[i] = clelf_platform_code(select_name[i]);

	cl_uint platform_devcount[8] = { 0,0,0,0,0,0,0,0 };

   for(i=0;i<nplatforms;i++) {

      char name[64];
      clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,64,name,0);
		int code = clelf_platform_code(name);

		err = clGetDeviceIDs(platforms[i],devtyp,0,0,&platform_devcount[i]);
		__set_oclerrno(err);

		for(j=0; j< nselect; j++) {


//			if (!strncasecmp(select_name[j],name,strnlen(select_name[j],64))) {
			if (select_code[j]==code) {

				printcl( CL_DEBUG "platform %d supports %d devices",
					i,platform_devcount[i]);

				if (platform_devcount[i] > 0) {
					select_id[j] = platforms[i];
					break;
				}

			}

		}

	}

	if (nplatforms == 1) {

		if (platform_devcount[0] == 0) {

      	printcl( CL_WARNING
				"clcontext_create: no platforms supporting device type (%d)!",
				devtyp);

			return((CONTEXT*)0);

		} else {

			platformid = platforms[0];

		}

	} else {

		for(i=0; i< nselect; i++) if (select_id[i]) { 

			platformid = select_id[i];
			printcl( CL_DEBUG "selecting platform %d",i);
			break;

		}

		if (!platformid) {

			int nmax = 0;
			for(i=0; i< nplatforms; i++) if (platform_devcount[i] > nmax) {

				nmax = platform_devcount[i];
				platformid = platforms[i];

			}

		}

	}

	if (!platformid) {

//     	WARN(__FILE__,__LINE__, 
//			"clcontext_create: no platforms supporting device type!");

		return((CONTEXT*)0);

	}	



	printcl( CL_DEBUG "clcontext_create: platformid=%p",platformid);

	

	/***
	 *** create context
	 ***/

	while (ctxprop_ext != 0 && ctxprop_ext[nctxprop] != 0) ++nctxprop;

	nctxprop += 3;

	ctxprop = (cl_context_properties*)
		malloc(nctxprop*sizeof(cl_context_properties));

	ctxprop[0] = (cl_context_properties)CL_CONTEXT_PLATFORM;
	ctxprop[1] = (cl_context_properties)platformid;

	for(i=0;i<nctxprop-3;i++) ctxprop[2+i] = ctxprop_ext[i];

	ctxprop[nctxprop-1] =  (cl_context_properties)0;
	

	err = clGetPlatformInfo(platformid,CL_PLATFORM_PROFILE,0,0,&sz);
	__set_oclerrno(err);

	cp->platform_profile = (char*)malloc(sz);

	err = clGetPlatformInfo(
		platformid,CL_PLATFORM_PROFILE,sz,cp->platform_profile,0);
	__set_oclerrno(err);

	clGetPlatformInfo(platformid,CL_PLATFORM_VERSION,0,0,&sz);
	__set_oclerrno(err);


	cp->platform_version = (char*)malloc(sz);

	err = clGetPlatformInfo(
		platformid,CL_PLATFORM_VERSION,sz,cp->platform_version,0);
	__set_oclerrno(err);


	err = clGetPlatformInfo(platformid,CL_PLATFORM_NAME,0,0,&sz);
	__set_oclerrno(err);

	cp->platform_name = (char*)malloc(sz);

	err = clGetPlatformInfo(platformid,CL_PLATFORM_NAME,sz,cp->platform_name,0);
	__set_oclerrno(err);

	err = clGetPlatformInfo(platformid,CL_PLATFORM_VENDOR,0,0,&sz);
	__set_oclerrno(err);

	cp->platform_vendor = (char*)malloc(sz);

	err = clGetPlatformInfo(
		platformid,CL_PLATFORM_VENDOR,sz,cp->platform_vendor,0);
	__set_oclerrno(err);

	err = clGetPlatformInfo(platformid,CL_PLATFORM_EXTENSIONS,0,0,&sz);
	__set_oclerrno(err);

	cp->platform_extensions = (char*)malloc(sz);

	err = clGetPlatformInfo(platformid,CL_PLATFORM_EXTENSIONS,sz,
		cp->platform_extensions,0);
	__set_oclerrno(err);


	printcl( CL_DEBUG "clcontext_create: selected platform: |%s|",
		cp->platform_name);	


#ifdef _WIN64

	cp->ctx = clCreateContextFromType(ctxprop,devtyp,0,0,&err);
	__set_oclerrno(err);

#else

	if (lock_key > 0) {

		if (ndevmax == 0) ndevmax = 1;

		cl_uint platform_ndev;
		err = clGetDeviceIDs(platformid,devtyp,0,0,&platform_ndev);
		 __set_oclerrno(err);
		cl_uint platform_vndev = platform_ndev;

		cl_device_id* platform_dev 
			= (cl_device_id*)malloc(platform_ndev*sizeof(cl_device_id));

		err = clGetDeviceIDs(platformid,devtyp,platform_ndev,platform_dev,0);
		__set_oclerrno(err);

		printcl( CL_DEBUG "clcontext_create: lock_key=%d",lock_key);

		pid_t pid = getpid();

		size_t sz_page = getpagesize();

		char shmobj[64];
		snprintf(shmobj,64,"/stdcl_ctx_lock%d.%d",devtyp,lock_key);

		printcl( CL_DEBUG "clcontext_create: attempt master shm_open %s from %d",
			shmobj,pid);

		int fd = shm_open(shmobj,O_RDWR|O_CREAT|O_EXCL,0);
		void* p0;

		struct timeval t0,t1;
		int timeout = 0;

		int noff = 0;

		if (fd < 0) {
			
			printcl( CL_DEBUG
				"clcontext_create: master shm_open failed from %d (%d)",pid,fd);

			printcl( CL_DEBUG
				"clcontext_create: attempt slave shm_open from %d",pid);

			timeout = 0;
			gettimeofday(&t0,0);
			t0.tv_sec += 10;

			do {
	
				fd = shm_open(shmobj,O_RDWR,0);
				gettimeofday(&t1,0);

				if (t1.tv_sec > t0.tv_sec && t1.tv_usec > t0.tv_usec) timeout = 1;

			} while (fd < 0 && !timeout);

			if (timeout) {

				printcl( CL_ERR "clcontext_create: shm_open timeout");

			}

			ftruncate(fd,sz_page);

			p0 = mmap(0,sz_page,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

			if (!p0) return(0);

			__ctx_lock = (struct __ctx_lock_struct*)p0;

			pthread_mutex_lock(&__ctx_lock->mtx);
			if (__ctx_lock->refc < platform_vndev) {
				noff = __ctx_lock->refc;
				ndev = min(ndevmax,platform_vndev-noff);
				__ctx_lock->refc += ndev;
			}
			pthread_mutex_unlock(&__ctx_lock->mtx);

			close(fd);

		} else {

			printcl( CL_DEBUG
				"clcontext_create: master shm_open succeeded from %d",pid);

			ftruncate(fd,sz_page);

			p0 = mmap(0,sz_page,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

			if (!p0) return(0);

			__ctx_lock = (struct __ctx_lock_struct*)p0;

			__ctx_lock->magic = 20110415;
			__ctx_lock->key = lock_key;
			pthread_mutex_init(&__ctx_lock->mtx,0);
			ndev = min(ndevmax,platform_vndev);
			printcl( CL_DEBUG "ndev=%d %d %d",ndev,ndevmax,platform_vndev);
			__ctx_lock->refc = ndev;

			fchmod(fd,S_IRUSR|S_IWUSR);

			close(fd);

		}

		printcl( CL_DEBUG "ndev=%d",ndev);

		if (noff < platform_vndev) {

			cp->ctx = clCreateContext(ctxprop,ndev,
				platform_dev + noff%platform_ndev,0,0,&err);
			__set_oclerrno(err);

			printcl( CL_DEBUG
				"clcontext_create: platform_ndev=%d ndev=%d noffset=%d",
				platform_ndev,ndev,noff);

			if (platform_dev) free(platform_dev);

		} else {

			cp->ctx = 0;

		}

	} else {

		cp->ctx = clCreateContextFromType(ctxprop,devtyp,0,0,&err);
		__set_oclerrno(err);

	}
#endif

	if (cp->ctx) {

		cp->devtyp = devtyp;
		err = clGetContextInfo(cp->ctx,CL_CONTEXT_DEVICES,0,0,&devlist_sz);
		__set_oclerrno(err);
		cp->ndev = devlist_sz/sizeof(cl_device_id);
		cp->dev = (cl_device_id*)malloc(10*devlist_sz);
		err=clGetContextInfo(cp->ctx,CL_CONTEXT_DEVICES,devlist_sz,cp->dev,0);
		__set_oclerrno(err);

	} else {

		printcl( CL_WARNING "clcontext_create: failed");

#ifndef _WIN64
		if (lock_key > 0 && ndev > 0) {
			pthread_mutex_lock(&__ctx_lock->mtx);
			__ctx_lock->refc -= ndev;
			pthread_mutex_unlock(&__ctx_lock->mtx);
		}

		free(cp);
#else
		_aligned_free(cp);
#endif

      return((CONTEXT*)0);

   }

	printcl( CL_DEBUG "number of devices %d",cp->ndev);

		

	/***
	 *** create command queues
	 ***/

	cp->cmdq = (cl_command_queue*)malloc(sizeof(cl_command_queue)*cp->ndev);

	printcl( CL_DEBUG "will try to create cmdq");

	

		for(i=0;i<cp->ndev;i++) {
#ifdef _WIN64
			cp->cmdq[i] = 0; /* have to defer, dllmain limitations */
#else
			cp->cmdq[i] = clCreateCommandQueue(cp->ctx,cp->dev[i],prop,&err);
			__set_oclerrno(err);

			printcl( CL_DEBUG 
				"clcontext_create: error from create cmdq %d (%p)\n",
				err,cp->cmdq[i]);
#endif
		}



	/***
	 *** init context resources
	 ***/

	LIST_INIT(&cp->prgs_listhead);
	LIST_INIT(&cp->txt_listhead);
	LIST_INIT(&cp->memd_listhead);



	/*** 
	 *** initialize event lists
	 ***/
	
	cp->kev = (struct _event_list_struct*)
		malloc(cp->ndev*sizeof(struct _event_list_struct));

	for(i=0;i<cp->ndev;i++) {
		cp->kev[i].nev = cp->kev[i].ev_first = cp->kev[i].ev_free = 0;
	}

	cp->mev = (struct _event_list_struct*)
		malloc(cp->ndev*sizeof(struct _event_list_struct));

	for(i=0;i<cp->ndev;i++) {
		cp->mev[i].nev = cp->mev[i].ev_first = cp->mev[i].ev_free = 0;
	}


	if (platforms) free(platforms);

	return(cp);

}



LIBSTDCL_API int 
clcontext_destroy(CONTEXT* cp)
{
	printcl( CL_DEBUG "clcontext_destroy() called");

	int i;
	int err = 0;

	if (!cp) return(0);

	size_t ndev = cp->ndev;

	printcl( CL_DEBUG "clcontext_destroy: ndev %d",ndev);

	if (cp->kev) free(cp->kev);

	while (cp->prgs_listhead.lh_first != 0)   
		clclose(cp,cp->prgs_listhead.lh_first);

	/* XXX here force detach of any clmalloc() memory -DAR */

#ifdef _WIN64
	printcl( CL_DEBUG "clcontext_destroy:"
		" cannot release cmdq's, its windows, hope for the best");
#else
	for(i=0;i<ndev;i++) {
		printcl( CL_DEBUG "checking cmdq for release %p",cp->cmdq[i]);
		if (cp->cmdq[i]) {
			err |= clReleaseCommandQueue(cp->cmdq[i]);
			__set_oclerrno(err);
		}
	}

	printcl( CL_DEBUG "clcontext_destroy: released cmdq's");
#endif

	err |= clReleaseContext(cp->ctx);
	__set_oclerrno(err);

	printcl( CL_DEBUG "clcontext_destroy: released ctx");

	if (cp->cmdq) free(cp->cmdq);
	printcl( CL_DEBUG "clcontext_destroy: free'd cmdq\n");

	if (cp->dev) free(cp->dev);
	printcl( CL_DEBUG "clcontext_destroy: free'd dev\n");

	if (cp->platform_profile) free(cp->platform_profile);
	if (cp->platform_version) free(cp->platform_version);
	if (cp->platform_name) free(cp->platform_name);
	if (cp->platform_vendor) free(cp->platform_vendor);
	if (cp->platform_extensions) free(cp->platform_extensions);

#ifndef _WIN64
	if (__ctx_lock) {
		printcl( CL_DEBUG "clcontext_destroy: ctx lock check refc");
		pthread_mutex_lock(&__ctx_lock->mtx);
		int n = __ctx_lock->refc -= cp->ndev;
		printcl( CL_DEBUG "clcontext_destroy: ctx lock refc now %d",n);
		if (n == 0) {
			int key = __ctx_lock->key;
			pthread_mutex_unlock(&__ctx_lock->mtx);
			char shmobj[64];
			snprintf(shmobj,64,"/stdcl_ctx_lock%d.%d",cp->devtyp,key);
			printcl( CL_DEBUG "clcontext_destroy: shm_unlink %s",shmobj);
			err = shm_unlink(shmobj);
			if (err) 
				printcl( CL_WARNING "clcontext_destroy: shm_unlink failed");
		}
		pthread_mutex_unlock(&__ctx_lock->mtx);
	}
#endif

#ifdef _WIN64
	_aligned_free(cp);
#else
	free(cp);
#endif

	return(0);
}



/* XXX there is no check of err code, this should be added -DAR */

LIBSTDCL_API int  
clgetdevinfo( CONTEXT* cp, struct cldev_info* info)
{

	if (!cp) { errno=ENOENT; return(-1); }

	int err;
	size_t ndev = cp->ndev;
	int i,n;
	cl_device_id* d;
	struct cldev_info* di;

	if (cp->dev) {

		for(n=0,d=cp->dev,di=info;n<ndev;n++,d++,di++) {

			err = clGetDeviceInfo( *d,CL_DEVICE_TYPE, sizeof(cl_device_type),
				&di->dev_type,0);
			__set_oclerrno(err);

		   err = clGetDeviceInfo( *d,CL_DEVICE_VENDOR_ID, sizeof(cl_uint),
				&di->dev_vendor_id,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_COMPUTE_UNITS,sizeof(cl_uint),
				&di->dev_max_core,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
				sizeof(cl_uint),&di->dev_max_wi_dim,0);
			__set_oclerrno(err);

			assert(di->dev_max_wi_dim<=4);

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_WORK_ITEM_SIZES,
				di->dev_max_wi_dim*sizeof(size_t),di->dev_max_wi_sz,0);
			__set_oclerrno(err);

			di->dev_max_wi_sz_is_symmetric=1;
     		for(i=0;i<di->dev_max_wi_dim;i++)
     	   	if(di->dev_max_wi_sz[0]!=di->dev_max_wi_sz[i]) 
					di->dev_max_wi_sz_is_symmetric=0;

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_WORK_GROUP_SIZE,
				sizeof(size_t),&di->dev_max_wg_sz,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,
				sizeof(cl_uint),&di->dev_pref_vec_char,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,
				sizeof(cl_uint),&di->dev_pref_vec_short,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
				sizeof(cl_uint),&di->dev_pref_vec_int,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,
				sizeof(cl_uint),&di->dev_pref_vec_long,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
				sizeof(cl_uint),&di->dev_pref_vec_float,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
				sizeof(cl_uint),&di->dev_pref_vec_double,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_CLOCK_FREQUENCY,
				sizeof(cl_uint),&di->dev_max_freq,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_ADDRESS_BITS,
				sizeof(cl_bitfield),&di->dev_addr_bits,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_GLOBAL_MEM_SIZE,
            sizeof(cl_ulong),&di->dev_global_mem_sz,0);	
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_MEM_ALLOC_SIZE,
            sizeof(cl_ulong),&di->dev_max_mem_alloc_sz,0);	
			__set_oclerrno(err);


			/*
			 * check image support
			 */

			err = clGetDeviceInfo( *d,CL_DEVICE_IMAGE_SUPPORT,
				sizeof(cl_bool),&di->dev_img_sup,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_READ_IMAGE_ARGS,
				sizeof(cl_uint),&di->dev_max_img_args_r,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_WRITE_IMAGE_ARGS,
				sizeof(cl_uint),&di->dev_max_img_args_w,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_IMAGE2D_MAX_WIDTH,
				sizeof(size_t),&di->dev_img2d_max_width,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_IMAGE2D_MAX_HEIGHT,
				sizeof(size_t),&di->dev_img2d_max_height,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_IMAGE2D_MAX_WIDTH,
				sizeof(size_t),&di->dev_img3d_max_width,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_IMAGE2D_MAX_HEIGHT,
				sizeof(size_t),&di->dev_img3d_max_height,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_IMAGE3D_MAX_DEPTH,
				sizeof(size_t),&di->dev_img3d_max_height,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_SAMPLERS,
				sizeof(cl_uint),&di->dev_max_samplers,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_MAX_PARAMETER_SIZE,
				sizeof(size_t),&di->dev_max_param_sz,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_MEM_BASE_ADDR_ALIGN,
				sizeof(cl_uint),&di->dev_mem_base_addr_align,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo(*d,CL_DEVICE_NAME,256,di->dev_name,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo(*d,CL_DEVICE_VENDOR,256,di->dev_vendor,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo(*d,CL_DEVICE_VERSION,256,di->dev_version,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo(*d,CL_DRIVER_VERSION,256,di->dev_drv_version,0);
			__set_oclerrno(err);

			err = clGetDeviceInfo( *d,CL_DEVICE_LOCAL_MEM_SIZE,
			 	sizeof(cl_ulong),&di->dev_local_mem_sz,0);
			__set_oclerrno(err);
	
		}

	}

	return(0);
}



LIBSTDCL_API void  
clfreport_devinfo( FILE* fp, size_t ndev, struct cldev_info* info )
{
	int i,n;
	struct cldev_info* di;

	if (info) for(n=0,di=info;n<ndev;n++,di++) {	

   	fprintf(fp,"device %d: ",n);

   	fprintf(fp,"CL_DEVICE_TYPE=");
   	if (di->dev_type&CL_DEVICE_TYPE_CPU) fprintf(fp," CPU");
   	if (di->dev_type&CL_DEVICE_TYPE_GPU) fprintf(fp," GPU");
   	if (di->dev_type&CL_DEVICE_TYPE_ACCELERATOR) fprintf(fp," ACCELERATOR");
   	if (di->dev_type&CL_DEVICE_TYPE_DEFAULT) fprintf(fp," DEFAULT");
   	fprintf(fp,"\n");

   	fprintf(fp,"CL_DEVICE_VENDOR_ID=%d\n",di->dev_vendor_id);
		fprintf(fp,"CL_DEVICE_MAX_COMPUTE_UNITS=%d\n",di->dev_max_core);
		fprintf(fp,"CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS=%d\n",di->dev_max_wi_dim);

   	if (di->dev_max_wi_dim>0) {

      	fprintf(fp,"CL_DEVICE_MAX_WORK_ITEM_SIZES=");
			
      	if (di->dev_max_wi_sz_is_symmetric)
         	fprintf(fp," %d (symmetric)\n",di->dev_max_wi_sz[0]);
      	else
         	for(i=0;i<di->dev_max_wi_dim;i++) 
					fprintf(fp," [%d]%d\n",i,di->dev_max_wi_sz[i]);
   	}

   	fprintf(fp,"CL_DEVICE_MAX_WORK_GROUP_SIZE=%d\n",di->dev_max_wg_sz);
   	fprintf(
			fp,"CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR=%d\n",di->dev_pref_vec_char
		);
   	fprintf(
			fp,"CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT=%d\n",
			di->dev_pref_vec_short
		);
   	fprintf(
			fp,"CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT=%d\n",di->dev_pref_vec_int
		);
   	fprintf(
			fp,"CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG=%d\n",di->dev_pref_vec_long
		);
   	fprintf(
			fp,"CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT=%d\n",di->dev_pref_vec_float
		);
   	fprintf(
			fp,"CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE=%d\n",
			di->dev_pref_vec_double
		);  
   	fprintf(fp,"CL_DEVICE_MAX_CLOCK_FREQUENCY=%d\n",di->dev_max_freq);

		fprintf(fp,"CL_DEVICE_ADDRESS_BITS=0x%x\n",di->dev_addr_bits);

	   if (di->dev_img_sup) {
      	fprintf(fp,"CL_DEVICE_IMAGE_SUPPORT=true\n");
      	fprintf(
				fp,"CL_DEVICE_MAX_READ_IMAGE_ARGS=%d\n",di->dev_max_img_args_r
			);
      	fprintf(
				fp,"CL_DEVICE_MAX_WRITE_IMAGE_ARGS=%d\n",di->dev_max_img_args_w
			);
      	fprintf(
				fp,"CL_DEVICE_IMAGE2D_MAX_WIDTH=%d\n",di->dev_img2d_max_width
			);
      	fprintf(
				fp,"CL_DEVICE_IMAGE2D_MAX_HEIGHT=%d\n",di->dev_img2d_max_height
			);
      	fprintf(
				fp,"CL_DEVICE_IMAGE3D_MAX_WIDTH=%d\n",di->dev_img3d_max_width
			);
      	fprintf(
				fp,"CL_DEVICE_IMAGE3D_MAX_HEIGHT=%d\n",di->dev_img3d_max_height
			);
      	fprintf(
				fp,"CL_DEVICE_IMAGE3D_MAX_DEPTH=%d\n",di->dev_img3d_max_depth
			);
   	} else {
      	fprintf(fp,"CL_DEVICE_IMAGE_SUPPORT=false\n");
   	}

   	fprintf(fp,"CL_DEVICE_MAX_PARAMETER_SIZE=%d\n",di->dev_max_param_sz);
   	fprintf(
			fp,"CL_DEVICE_MEM_BASE_ADDRESS_ALIGN=%d\n",di->dev_mem_base_addr_align
		);

   	fprintf(fp,"CL_DEVICE_NAME=%s\n",di->dev_name);
   	fprintf(fp,"CL_DEVICE_VENDOR=%s\n",di->dev_vendor);
   	fprintf(fp,"CL_DEVICE_VERSION=%s\n",di->dev_version);
   	fprintf(fp,"CL_DRIVER_VERSION=%s\n",di->dev_drv_version);

   	fprintf(fp,"CL_DEVICE_LOCAL_MEM_SIZE=%d\n",di->dev_local_mem_sz);

	}

}

LIBSTDCL_API int  
clstat( CONTEXT* cp, struct clstat_info* st)
{
	if (!cp || !st) return(-1);

	st->impid = cp->impid;
	st->ndev = cp->ndev;
	st->nprg = cp->nprg;
	st->nkrn = cp->nkrn;

	return(0);
}

LIBSTDCL_API cl_uint  
clgetndev( CONTEXT* cp )
{
	if (!cp) return (0);
	return (cp->ndev);
}

static cl_platform_id
__get_platformid(
 cl_uint nplatform, cl_platform_id* platforms, const char* platform_name
)  
{
	int err;

   printcl( CL_DEBUG "__get_platformid: platform_name |%s|",platform_name);

   int i,j;
   char name[256];

   if (platform_name == 0 || platform_name[0] == '\0') {

      strcpy(name,DEFAULT_PLATFORM_NAME);

      printcl( CL_DEBUG "__get_platformid: use default platform_name |%s|",
			name);

   } else {

      strncpy(name,platform_name,256);

   }

   if (platforms) for(i=0;i<nplatform;i++) {

      char info[1024];

      err = clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1024,info,0);
		__set_oclerrno(err);

      printcl( CL_DEBUG "__get_platformid: compare |%s|%s|",name,info);

      if (!strncasecmp(name,info,strlen(name))) {

         printcl( CL_DEBUG "__get_platformid:  match %s %s",name,info);

         return(platforms[i]);

      }

   }

   if (nplatform > 0) return(platforms[0]); /* default to first one */

	__set_clerrno(-1);
   return((cl_platform_id)(-1)); /* none found */

}

