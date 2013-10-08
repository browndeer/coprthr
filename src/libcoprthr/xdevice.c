/* xdevice.c
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
#include <dlfcn.h>

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif


#include <CL/cl.h>

#include "xcl_structs.h"
#include "device.h"
#include "cmdcall.h"
#include "cmdcall_x86_64_sl.h"
#include "compiler.h"
#include "program.h"


#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif


int getenv_token( const char* name, const char* token, char* value, size_t n );

void* dlh_compiler = 0;


static char* strnlen_ws( char* p, char* s, size_t maxlen)
{
	size_t sz = strnlen(s,maxlen);
	char* p1 = s;
	char* p2 = s + sz;
	while(p1 < p2 && (*p1==' ' || *p1=='\t' || *p1=='\n')) ++p1;
	while(--p2 > s && (*p2==' ' || *p2=='\t' || *p2=='\n')) *p2='\0';
	return(p1);
}

static char* truncate_ws(char* buf)
{
	size_t sz = strnlen(buf,__CLMAXSTR_LEN);
	char* p = buf + sz - 1;
	while( p>buf && (*p==' '||*p=='\t'||*p=='\n')) *(p--) = '\0'; 
	p = buf;
	while( p<buf+sz && (*p==' '||*p=='\t'||*p=='\n')) ++p; 
	return(p);
}

#define COPRTHR_DEVICE_NSUPP_MAX 2

struct coprthr_device* __coprthr_do_discover_device_x86_64(void);
struct coprthr_device* __coprthr_do_discover_device_i386(void);

void __do_discover_devices(
	unsigned int* p_ndevices, 
	struct _cl_device_id** p_dtab, 
	int flag
)
{
	int i;

	if (*p_dtab) return;

	int nsupp = 0;
	struct coprthr_device** codevtab = (struct coprthr_device**)
		malloc(COPRTHR_DEVICE_NSUPP_MAX*sizeof(struct coprthr_device*));

	codevtab[nsupp++] = __coprthr_do_discover_device_x86_64();
	codevtab[nsupp++] = 0; // __coprthr_do_discover_device_i386();

	int navail = 0;
	int nall = 0;
	for(i=0; i<nsupp; i++) {
		struct coprthr_device* codev = codevtab[i];
		if (codev) {
			if (codev->devstate->avail) ++navail;
			if (codev->devstate->compiler_avail) ++nall;
		}
	}

	printcl( CL_DEBUG "navail nall %d %d",navail,nall);

	int ndev = (flag==1)? nall : navail;

	*p_ndevices = ndev;

	struct _cl_device_id* dtab = *p_dtab = (struct _cl_device_id*)
		malloc(*p_ndevices*sizeof(struct _cl_device_id));

	printcl( CL_DEBUG "ndevices %d",*p_ndevices);

	int devnum = 0;

	for(i = 0; i<nsupp; i++) {
		struct coprthr_device* codev = codevtab[i];
		if (codev && codev->devstate->avail) {
			__init_device_id(dtab+devnum);
			dtab[devnum++].codev = codevtab[i];
			codevtab[i] = 0;
		}
	}

	for(i = 0; i<nsupp; i++) {
		struct coprthr_device* codev = codevtab[i];
		if (codev && codev->devstate->compiler_avail) {
			__init_device_id(dtab+devnum);
			dtab[devnum++].codev = codevtab[i];
			codevtab[i] = 0;
		}
	}

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


void __do_get_ndevices(
	cl_platform_id platformid, cl_device_type devtype, cl_uint* ndev 
)
{
	unsigned int ndevices = __resolve_platformid(platformid,ndevices);
	struct _cl_device_id* dtab = __resolve_platformid(platformid,dtab);

	printcl( CL_DEBUG "ndevices = %d",ndevices);

	int devnum;
	unsigned int n = 0;

	for(devnum=0;devnum<ndevices;devnum++) {
		printcl( CL_DEBUG "match devtype %d %d",
			dtab[devnum].codev->devinfo->devtype,devtype);
		if (dtab[devnum].codev->devinfo->devtype & devtype) n++;
	}

	printcl( CL_DEBUG "n = %d",n);

	*ndev = n;
}



void __do_get_devices(
	cl_platform_id platformid, cl_device_type devtype, 
	cl_uint ndev, cl_device_id* devices)
{
	unsigned int ndevices = __resolve_platformid(platformid,ndevices);
	struct _cl_device_id* dtab = __resolve_platformid(platformid,dtab);

	int devnum;
	int n = 0;

	for(devnum=0;devnum<ndevices;devnum++) 
		if (n<ndev && dtab[devnum].codev->devinfo->devtype & devtype) 
			devices[n++] = &__resolve_platformid(platformid,dtab[devnum]);

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

struct coprthr_device* __devtab = 0;
unsigned int __ndev = 0;
struct coprthr_device* __ddtab[256] = { [0 ... 255] = 0 };
int __ddtab_nxt = 0;

#if(0)
int coprthr_dopen( const char* name, int flags )
{

	if (!__devtab) {
		__do_discover_devices(&__ndev,&__devtab,1);
	}

	if (_ddtab_nxt == 256) return -1; /* XXX out of resources */

	int idev = -1;

	intptr_t iname = (intptr_t)name;

	if (iname < 256 && iname < __ndev)
		idev = (int)iname;

	_ddtab[_ddtab_nxt] = __devtab[idev];

	int dd = _ddtab_nxt;

	do {
		++_ddtab_nxt;
	} while(_ddtab_sz<256 && _ddtab[_ddtab_nxt]);

	return dd;	
}

int coprthr_dclose(int dd)
{
	if (dd>=0 && dd<256) {
		// XXX if last ref shutdown the device 
		_ddtab[dd] = 0;
		if (dd<_ddtab_nxt) _ddtab_nxt = dd;
		return 0;
	} else return -1;
}


#endif

