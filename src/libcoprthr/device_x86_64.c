/* device_x86_64.c
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

#define ENABLE_OFFLINE_DEVICE

#define _GNU_SOURCE
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include "device.h"
#include "cmdcall.h"
#include "cmdcall_x86_64_sl.h"
#include "compiler.h"
#include "program.h"
#include "memobj.h"

#include "coprthr_arch.h"
#include "coprthr_device.h"
#include "coprthr.h"

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif

#define __CLMAXSTR_LEN 1023
#define __CLMAXSTR_BUFSZ (__CLMAXSTR_LEN+1)

//char* strnlen_ws( char* p, char* s, size_t maxlen)
//{
//   size_t sz = strnlen(s,maxlen);
//   char* p1 = s;
//   char* p2 = s + sz;
//   while(p1 < p2 && (*p1==' ' || *p1=='\t' || *p1=='\n')) ++p1;
//   while(--p2 > s && (*p2==' ' || *p2=='\t' || *p2=='\n')) *p2='\0';
//   return(p1);
//}

static char* truncate_ws(char* buf)
{
   size_t sz = strnlen(buf,__CLMAXSTR_LEN);
   char* p = buf + sz - 1;
   while( p>buf && (*p==' '||*p=='\t'||*p=='\n')) *(p--) = '\0';
   p = buf;
   while( p<buf+sz && (*p==' '||*p=='\t'||*p=='\n')) ++p;
   return(p);
}

static void* dlh_compiler = 0;

struct coprthr_device* __coprthr_do_discover_device_x86_64(void)
{
	printcl( CL_DEBUG "__coprthr_do_discover_device_x86_64");

	struct coprthr_device* codev = malloc(sizeof(struct coprthr_device));
	codev->devinfo = malloc(sizeof(struct coprthr_device_info));
	codev->devstate = malloc(sizeof(struct coprthr_device_state));
	codev->devops = malloc(sizeof(struct coprthr_device_operations));
	codev->devcomp = malloc(sizeof(struct coprthr_device_compiler));
	codev->devlink = malloc(sizeof(struct coprthr_device_linker));

	/* assume we have at least one multicore CPU device */

	unsigned int ncpu = 1;

	*codev->devinfo = (struct coprthr_device_info){
		.memsup = COPRTHR_DEVMEM_TYPE_BUFFER,
		.arch_id = COPRTHR_ARCH_ID_X86_64
	};

	codev->devinfo->name = 0;
	codev->devinfo->vendor = 0;
	codev->devinfo->drv_version = 0;
	codev->devinfo->profile = 0;
	codev->devinfo->version = 0;

	FILE* fp;
	struct stat fs;
	char buf[1024];
	size_t sz;

#if defined(__FreeBSD__)

	int val=0;
	sz=4;
	sysctlbyname("hw.ncpu",&val,&sz,0,0);
	codev->devinfo->max_compute_units = val;

	sz=4;
	sysctlbyname("hw.clockrate",&val,&sz,0,0);
	codev->devinfo->max_freq = val;

	sz=1024;
	sysctlbyname("hw.model",buf,&sz,0,0);

	char* bufp = truncate_ws(buf);
	sz = 1+strnlen(bufp,__CLMAXSTR_LEN);
	codev->devinfo->name = strndup(bufp,sz);

#elif defined(__x86_64__) || defined(__i386__)

	if (stat("/proc/cpuinfo",&fs)) {
		printcl( CL_WARNING "stat failed on /proc/cpuinfo");
		return;
	}

	fp = fopen("/proc/cpuinfo","r");

	while (fgets(buf,1024,fp)) {

		char* savptr;
		char* left = (char*)strtok_r(buf,":",&savptr);
		char* right = (char*)strtok_r(0,":",&savptr);

		if (!strncasecmp(left,"processor",9)) {

			if (atoi(right) > 0) break;

		} else if (!strncasecmp(left,"cpu count",8)) {

			codev->devinfo->max_compute_units = atoi(right);

		} else if (!strncasecmp(left,"cpu MHz",7)) {

			codev->devinfo->max_freq = atoi(right);

		} else if (!strncasecmp(left,"model name",10)) {

			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			codev->devinfo->name = strndup(right,sz);

		} else if (!strncasecmp(left,"vendor_id",9)) {

			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			codev->devinfo->vendor = strndup(right,sz);

		}

	}

	fclose(fp);


	if (stat("/proc/meminfo",&fs)) {
		printcl( CL_WARNING "stat failed on /proc/meminfo");
		return;
	}


	fp = fopen("/proc/meminfo","r");

	while (fgets(buf,1024,fp)) {

		char* savptr;
		char* left = (char*)strtok_r(buf,":",&savptr);
		char* right = (char*)strtok_r(0,":",&savptr);

	}

	fclose(fp);

#elif defined(__arm__) 

	if (stat("/proc/cpuinfo",&fs)) {
		printcl( CL_WARNING "stat failed on /proc/cpuinfo");
		return;
	}

	fp = fopen("/proc/cpuinfo","r");

	while (fgets(buf,1024,fp)) {

		char* savptr;
		char* left = (char*)strtok_r(buf,":",&savptr);
		char* right = (char*)strtok_r(0,":",&savptr);

		if (!strncmp(left,"processor",9)) {

			codev->devinfo->max_compute_units = atoi(right) + 1;

		} else if (!strncmp(left,"Processor",9)) {

			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			codev->devinfo->name = strndup(right,sz);

		} else if (!strncasecmp(left,"CPU implementer",15)) {

			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			codev->devinfo->vendor = strndup(right,sz);

		}

	}

	fclose(fp);


	if (stat("/proc/meminfo",&fs)) {
		printcl( CL_WARNING "stat failed on /proc/meminfo");
		return;
	}

	fp = fopen("/proc/meminfo","r");

	while (fgets(buf,1024,fp)) {

		char* savptr;
		char* left = (char*)strtok_r(buf,":",&savptr);
		char* right = (char*)strtok_r(0,":",&savptr);

	}

	fclose(fp);

#endif

	#define __terminate(p) do { if (!p) p = strdup("unknown"); } while(0)
	__terminate(codev->devinfo->name);
	__terminate(codev->devinfo->vendor);
	__terminate(codev->devinfo->drv_version);
	__terminate(codev->devinfo->profile);
	__terminate(codev->devinfo->version);


	dlh_compiler = dlopen("libcoprthrcc.so",RTLD_LAZY);
	if (!dlh_compiler) 
		printcl( CL_WARNING "no compiler,failed to load libcoprthrcc.so");

#if defined(__x86_64__)	
	codev->devcomp->comp = dlsym(dlh_compiler,"compile_x86_64");
	codev->devcomp->ilcomp = 0;
	codev->devlink->link = 0;
	codev->devlink->bind_ksyms = bind_ksyms_default;
	*(codev->devops) = (struct coprthr_device_operations){
		0,
		__coprthr_memalloc, __coprthr_memrealloc, __coprthr_memfree,
		__coprthr_memread, __coprthr_memwrite, __coprthr_memcopy
	};
	codev->devops->v_cmdcall = cmdcall_x86_64_sl;
#elif defined(__arm__)
	codev->devcomp->comp = dlsym(dlh_compiler,"compile_arm32");;
	codev->devcomp->ilcomp = 0;
	codev->devlink->link = 0;
	codev->devlink->bind_ksyms = bind_ksyms_default;
	*(codev->devops) = (struct coprthr_device_operations){
		0,
		__coprthr_memalloc, __coprthr_memrealloc, __coprthr_memfree,
		__coprthr_memread, __coprthr_memwrite, __coprthr_memcopy
	};
	codev->devops->v_cmdcall = cmdcall_x86_64_sl; /* XXX fix naming -DAR */
#else
#error unsupported architecture
#endif

	if (!codev->devcomp->comp) {
		printcl( CL_WARNING "no compiler, dlsym failure");
		codev->devstate->compiler_avail = 0;
	} else {
		codev->devstate->compiler_avail = 1;
	}


	int i;

	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);

	if (getenv("COPRTHR_MAX_NUM_ENGINES"))
		ncore = min(ncore,atoi(getenv("COPRTHR_MAX_NUM_ENGINES")));

	codev->devstate->avail = 1;
	CPU_ZERO(&(codev->devstate->cpumask));
	for(i=0;i<ncore;i++) CPU_SET(i,&(codev->devstate->cpumask));
	codev->devstate->cpu.veid_base = 0;
	codev->devstate->cpu.nve = ncore;

	codev->devstate->cmdq = 0;

	codev->devstate->locked_pid = 0;

	printcl( CL_DEBUG "returning codev %p",codev);

	return codev;

}


