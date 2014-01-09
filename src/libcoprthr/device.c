/* device.c
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

//#define ENABLE_OFFLINE_DEVICE

#define _GNU_SOURCE
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include "printcl.h"
#include "coprthr_arch.h"
#include "device.h"
#include "cmdcall.h"
#include "compiler.h"
#include "program.h"

#include "coprthr.h"

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif

struct __coprthr_supptab_entry {
	int id;
	const char* name;
	const char* libname;
};

/*
 *	The following devices are currently supported:
 * x86_64 
 * i386
 * arm32
 * e32
 */
struct __coprthr_supptab_entry __coprthr_supptab[] = {
	{ COPRTHR_ARCH_ID_X86_64, "x86_64", "libcoprthr_arch_x86_64.so" },
	{ COPRTHR_ARCH_ID_I386, "i386", 0 },
	{ COPRTHR_ARCH_ID_ARM32, "ARM (32-bit)", 0 },
	{ COPRTHR_ARCH_ID_E32_EMEK, "Epiphany EMEK (32-bit)", 0 },
	{ COPRTHR_ARCH_ID_E32, "Epiphany (32-bit)", "libcoprthr_arch_e32.so" }
};


int getenv_token( const char* name, const char* token, char* value, size_t n );

void* dlh_compiler = 0;

#define COPRTHR_DEVICE_NSUP_MAX 16

static int nsupp = 0;
static struct coprthr_device** codevtab = 0;

int coprthr_register_device( struct coprthr_device* dev ) 
{
	codevtab[nsupp++] = dev;
	return 0;
}

void __do_discover_devices_1(
	unsigned int* p_ndevices, 
	struct coprthr_device*** p_devtab, 
	int flag
)
{
	int i;

	printcl( CL_DEBUG "__do_discover_devices_1 %p",*p_devtab);

	if (*p_devtab) return;

	codevtab = (struct coprthr_device**)
		malloc(COPRTHR_DEVICE_NSUP_MAX*sizeof(struct coprthr_device*));

	void* h0 = dlopen("libcoprthr.so",RTLD_NOW|RTLD_GLOBAL);

	void* hh[COPRTHR_DEVICE_NSUP_MAX];

	/* for each supported device */
	{
		hh[0] = dlopen(__coprthr_supptab[0].libname,RTLD_LAZY);
		if (!hh[0]) printcl( CL_WARNING "%s", dlerror() );

		coprthr_init_device_call_t init_device = dlsym(hh[0],
			"coprthr_init_device");
		printcl( CL_DEBUG "h=%p init_device=%p",hh[0],init_device);

		init_device();
	}
//	codevtab[nsupp++] = 0; // __coprthr_do_discover_device_i386();

	if (1) {
		hh[4] = dlopen(__coprthr_supptab[4].libname,RTLD_LAZY);
		if (hh[4]) {
			coprthr_init_device_call_t init_device = dlsym(hh[4],
				"coprthr_init_device");
			printcl( CL_DEBUG "h=%p init_device=%p",hh[4],init_device);
			init_device();
		} else {
//			printcl( CL_WARNING "%s", dlerror() );
		}
	}

/* XXX HACK */
//nsupp=1;

	printcl( CL_DEBUG "codevtab[0]=%p", codevtab[0] );
	printcl( CL_DEBUG "codevtab[1]=%p", codevtab[1] );

	int navail = 0;
	int nall = 0;
	for(i=0; i<nsupp; i++) {
		struct coprthr_device* codev = codevtab[i];
		if (codev) {
			if (codev->devstate->avail) ++navail;
			if (codev->devstate->compiler_avail) ++nall;
		}
		printcl( CL_DEBUG "%d %d",navail,nall);
	}

	printcl( CL_DEBUG "navail nall %d %d",navail,nall);

	int ndev = (flag==1)? nall : navail;

	*p_ndevices = ndev;

	struct coprthr_device** devtab = *p_devtab = (struct coprthr_device**)
		malloc(*p_ndevices*sizeof(struct coprthr_device*));

	printcl( CL_DEBUG "ndevices %d",*p_ndevices);

	int devnum = 0;

	for(i = 0; i<nsupp; i++) {
		struct coprthr_device* codev = codevtab[i];
		if (codev && codev->devstate->avail) {
			printcl( CL_DEBUG "devtab add %p", codevtab[i] );
			devtab[devnum++] = codevtab[i];
		}
	}

	for(i = 0; i<nsupp; i++) {
		struct coprthr_device* codev = codevtab[i];
		if (codev && codev->devstate->compiler_avail
			&& !codev->devstate->avail ) {
				printcl( CL_DEBUG "devtab add %p", codevtab[i] );
				devtab[devnum++] = codevtab[i];
		}
	}

/*
	for(i = 0; i<nsupp; i++) {
		struct coprthr_device* codev = codevtab[i];
		if (codev && codev->devstate->compiler_avail) {
			devtab[devnum++] = codevtab[i];
		}
	}
*/

	printcl( CL_DEBUG "__do_discover_devices_1 ndevices %d",*p_ndevices);

//	int i;
//
//	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);
//
//	if (getenv("COPRTHR_MAX_NUM_ENGINES"))
//		ncore = min(ncore,atoi(getenv("COPRTHR_MAX_NUM_ENGINES")));
//
//#ifdef ENABLE_NCPU
//
//	printcl( CL_DEBUG "checking for force_ncpu");
//
//	if (!getenv_token("COPRTHR_OCL","force_ncpu",buf,256)) ncpu = atoi(buf);
//
//	if (ncore%ncpu) {
//		printcl( CL_WARNING "force_ncpu ignored, must be multiple of ncore");
//		ncpu = 1;
//	}
//
//	if (ncpu > 1) {
//
//		printcl( CL_WARNING "force_ncpu %d",ncpu);
//
//		printcl( CL_DEBUG "force_ncpu = %d",ncpu);
//
//		*p_dtab = (struct _cl_device_id*)
//			realloc(*p_dtab,(ncpu-1)*sizeof(struct _cl_device_id));
//
//		for(devnum=1;devnum<ncpu;devnum++) 
//			memcpy((*p_dtab)+devnum,(*p_dtab),sizeof(struct _cl_device_id));
//
//		int cpd = ncore/ncpu;
//		for(devnum=0;devnum<ncpu;devnum++) {
//			CPU_ZERO(&dtab[devnum].imp->cpumask);
//			for(i=devnum*cpd;i<(devnum+1)*cpd;i++) 
//				CPU_SET(i,&dtab[devnum].imp->cpumask);
//			dtab[devnum].imp->cpu.veid_base = devnum*cpd;
//			dtab[devnum].imp->cpu.nve = cpd;
//			
//			printcl( CL_DEBUG "devnum base nve %d %d %d",
//				devnum,dtab[devnum].imp->cpu.veid_base,dtab[devnum].imp->cpu.nve);
//		}
//
//		*p_ndevices = ncpu;
//
//	} else {
//		CPU_ZERO(&dtab[0].imp->cpumask);
//		for(i=0;i<ncore;i++) CPU_SET(i,&dtab[0].imp->cpumask);
//		dtab[0].imp->cpu.veid_base = 0;
//		dtab[0].imp->cpu.nve = ncore;
//	}
//#else
//	CPU_ZERO(&dtab[0].imp->cpumask);
//	for(i=0;i<ncore;i++) CPU_SET(i,&dtab[0].imp->cpumask);
//	dtab[0].imp->cpu.veid_base = 0;
//	dtab[0].imp->cpu.nve = ncore;
//#endif
//

//	dtab[0].codev = __coprthr_do_discover_device_x86_64();

}


int getenv_token( const char* name, const char* token, char* value, size_t n )
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

      } else if (!token) {

         strncpy(value,clause,min(strlen(clause)+1,n));
         return(0);

      }

      clause = strtok_r(0,":",&ptr);
   }

   return(2);

}

// XXX instead of static use default + env var and dynamically allocate
// XXX on first call when a discover devices is also done

struct coprthr_device** __devtab = 0;
unsigned int __ndev = 0;
struct coprthr_device* __ddtab[256] = { [0 ... 255] = 0 };
int __ddtab_nxt = 0;

void* coprthr_getdev( const char* name, int flags )
{

	if (!__devtab) {
		__do_discover_devices_1(&__ndev,&__devtab,1);
	}

	int idev = -1;

	intptr_t iname = (intptr_t)name;

	printcl( CL_DEBUG "iname %d", iname );

	if (iname < 256 && iname < __ndev)
		idev = (int)iname;

	struct coprthr_device* dev = __devtab[idev];

	printcl( CL_DEBUG "coprthr_devget: dev->devstate->cmdq %p",
		dev->devstate->cmdq);

	__do_create_command_queue_1(dev);

	printcl( CL_DEBUG "coprthr_devget: dev->devstate->cmdq %p",
		dev->devstate->cmdq);

	return dev;
}


int coprthr_dopen( const char* name, int flags )
{
	int i;

	if (!__devtab) {
		__do_discover_devices_1(&__ndev,&__devtab,1);
	}

	if (__ddtab_nxt == 256) return -1; /* XXX out of resources */

	int idev = -1;

	intptr_t iname = (intptr_t)name;

	printcl( CL_DEBUG "iname=%d",iname);

//	if (iname < 256 && iname < __ndev) {
	if (iname < 256) {
//		idev = (int)iname;

/*
		int n = sizeof(__coprthr_supptab)
			/sizeof(struct __coprthr_supptab_entry);
		for(i=0; i<n; i++) {
			printcl( CL_DEBUG "checking id %d %d",i,__coprthr_supptab[i].id);
			if (__coprthr_supptab[i].id == (int)iname) break;
		}
		if (i<n) idev = i;
*/
		for(i=0; i<nsupp; i++) {
			printcl( CL_DEBUG "checking id %d %d",i,__devtab[i]->devinfo->arch_id);
			if (__devtab[i]->devinfo->arch_id == (int)iname) break;
		}
		if (i<nsupp) idev = i;

	}

	printcl( CL_DEBUG "idev=%d",idev);

	if (idev == -1) {
		errno = ENODEV;
		return -1;
	}

	__ddtab[__ddtab_nxt] = __devtab[idev];

	int dd = __ddtab_nxt;

	do {
		++__ddtab_nxt;
	} while(__ddtab_nxt<256 && __ddtab[__ddtab_nxt]);

	struct coprthr_device* dev = __ddtab[dd];
	printcl( CL_DEBUG "coprthr_dopen: dev %p", dev);
	printcl( CL_DEBUG "coprthr_dopen: dev->devstate %p", dev->devstate);
	printcl( CL_DEBUG "coprthr_dopen: dev->devstate->cmdq %p",
		dev->devstate->cmdq);
	__do_create_command_queue_1(dev);
	printcl( CL_DEBUG "coprthr_dopen: dev->devstate->cmdq %p",
		dev->devstate->cmdq);

	return dd;	
}

int coprthr_dclose(int dd)
{
	if (dd>=0 && dd<256) {
		// XXX if last ref shutdown the device 
		__ddtab[dd] = 0;
		if (dd<__ddtab_nxt) __ddtab_nxt = dd;
		return 0;
	} else return EBADF;
}


#define __coprthr_is_locked(dev) 0
#define __coprthr_lock(dev) do {} while(0)
#define __coprthr_device_init(dev) do {} while(0)

pid_t __coprthr_lock_pid = 0;

#define __return_errno(e) do { errno = e; return e; } while(0)

int coprthr_devlock( struct coprthr_device* dev, int flags )
{
	if (!dev)
		__return_errno(EINVAL);

	if (__coprthr_is_locked(dev) || (flags&COPRTHR_DEVLOCK_NOWAIT) )
		__return_errno(EAGAIN);

	while( __coprthr_is_locked(dev) );

	__coprthr_lock(dev);

	dev->devstate->locked_pid = getpid();

	if (!(flags&COPRTHR_DEVLOCK_NOINIT))
		__coprthr_device_init(dev);

	return 0;

}


int coprthr_devunlock( struct coprthr_device* dev, int flags )
{
	if (!dev)
		__return_errno(EINVAL);

	if (!__coprthr_is_locked(dev))
		__return_errno(EPERM);

	if (dev->devstate->locked_pid != getpid())
		__return_errno(EACCES);

	dev->devstate->locked_pid = 0;

	__coprthr_unlock(dev);

	return 0;
}

