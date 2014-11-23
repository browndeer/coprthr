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

#include "printcl.h"
#include "device.h"
#include "cmdcall.h"
//#include "cmdcall_x86_64_sl.h"
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

struct coprthr_device_commands devcmds_x86_64;
struct coprthr_device_operations devops_x86_64;

int bind_ksyms_default( struct _coprthr_ksyms_struct* ksyms, void* h,
   char* kname );

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
static int called = 0;

static int init_device_x86_64(void)
{
	printcl( CL_DEBUG "init_device_x86_64");
//	if (called) return 0;

	struct coprthr_device* codev = malloc(sizeof(struct coprthr_device));
	codev->devinfo = malloc(sizeof(struct coprthr_device_info));
	codev->devstate = malloc(sizeof(struct coprthr_device_state));
	codev->devops = malloc(sizeof(struct coprthr_device_operations));
	codev->devcomp = malloc(sizeof(struct coprthr_device_compiler));
	codev->devlink = malloc(sizeof(struct coprthr_device_linker));

//	if (called) return 0;

	/***
	 *** set devinfo
	 ***/

	/* assume we have at least one multicore CPU device */

	unsigned int ncpu = 1;

	*codev->devinfo = (struct coprthr_device_info){
		.memsup = COPRTHR_DEVMEM_TYPE_BUFFER
			|COPRTHR_DEVMEM_TYPE_MUTEX,
		.arch_id = COPRTHR_ARCH_ID_X86_64,
		.devsup = COPRTHR_DEVSUP_F_RUNTIME 
			| COPRTHR_DEVSUP_F_STREAM | COPRTHR_DEVSUP_F_THREAD
			| COPRTHR_DEVSUP_F_MEM_BUFFER | COPRTHR_DEVSUP_F_MEM_MUTEX
	};

	codev->devinfo->name = 0;
	codev->devinfo->vendor = 0;
	codev->devinfo->drv_version = 0;
	codev->devinfo->profile = 0;
	codev->devinfo->version = 0;

	FILE* fp;
	struct stat fs;
//	char buf[1024];
	char* buf = malloc(1024);
	size_t sz;

//	if (called) return 0;

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

#if defined(EANBLE_MIC_CROSS_COMPILE)
			sz = 1+strnlen(
				"Intel(R) Many Integrated Core Acceleration Card",__CLMAXSTR_LEN);
			strncpy(dstrtab+dstrtab_sz,
				"Intel(R) Many Integrated Core Acceleration Card",sz);
			codev->devinfo->name 
				= strndup("Intel(R) Many Integrated Core Acceleration Card",sz);
#else
			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			codev->devinfo->name = strndup(right,sz);
#endif

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

		if (!strncasecmp(left,"MemFree",7)) {
			codev->devinfo->global_mem_sz = atoi(right) * 1000;
			break;
		}

	}

	fclose(fp);


#elif defined(__arm__) 

	if (stat("/proc/cpuinfo",&fs)) {
		printcl( CL_WARNING "stat failed on /proc/cpuinfo");
		return;
	}

//	fp = fopen("/proc/cpuinfo","r");
//	fp = popen("cat /proc/cpuinfo","r");

//	printcl( CL_DEBUG "fp=%p",fp);

/*
	while (fgets(buf,1024,fp)) {

printcl( CL_DEBUG "fgets |%s|",buf);

		char* savptr;
		char* left = (char*)strtok_r(buf,":",&savptr);
		char* right = (char*)strtok_r(0,":",&savptr);

		if (!strncmp(left,"processor",9)) {

			codev->devinfo->max_compute_units = atoi(right) + 1;

		} else if (!strncmp(left,"Processor",9)) {

//			right = truncate_ws(right);
//			sz = 1+strnlen(right,__CLMAXSTR_LEN);
//			codev->devinfo->name = strndup(right,sz);

		} else if (!strncasecmp(left,"CPU implementer",15)) {

//			right = truncate_ws(right);
//			sz = 1+strnlen(right,__CLMAXSTR_LEN);
//			codev->devinfo->vendor = strndup(right,sz);
//			codev->devinfo->vendor = strndup("foo",5);
//			codev->devinfo->vendor = strdup("foo");
//printcl( CL_DEBUG "%ld|%s|",sz,right);
		}

	}
*/

//codev->devinfo->vendor = strdup("foo");
//char* pfoo = malloc(10);
//strncpy(pfoo,"foo",10);
//	fclose(fp);
//codev->devinfo->vendor = pfoo;


	if (stat("/proc/meminfo",&fs)) {
		printcl( CL_WARNING "stat failed on /proc/meminfo");
		return;
	}

	fp = fopen("/proc/meminfo","r");

	while (fgets(buf,1024,fp)) {

		char* savptr;
		char* left = (char*)strtok_r(buf,":",&savptr);
		char* right = (char*)strtok_r(0,":",&savptr);

		if (!strncasecmp(left,"MemFree",7)) {
			codev->devinfo->global_mem_sz = atoi(right) * 1000;
//			printf("set global_mem_sz %d\n",codev->devinfo->global_mem_sz);
			break;
		}


	}

	fclose(fp);


codev->devinfo->max_compute_units = 2;
codev->devinfo->name = strdup("ARMv7 Processor rev 0 (v7l)");
codev->devinfo->vendor = strdup("Xilinx");


#endif


	#define __terminate(p) do { if (!p) p = strdup("unknown"); } while(0)
	__terminate(codev->devinfo->name);
	__terminate(codev->devinfo->vendor);
	__terminate(codev->devinfo->drv_version);
	__terminate(codev->devinfo->profile);
	__terminate(codev->devinfo->version);



//	if (called) return 0;

	/***
	 *** set devcomp, devlink, devops, devcmds
	 ***/

	dlh_compiler = dlopen("libcoprthrcc.so",RTLD_LAZY);
	if (!dlh_compiler) 
		printcl( CL_WARNING "no compiler,failed to load libcoprthrcc.so");

#if defined(__x86_64__)	
	codev->devcomp->comp = dlsym(dlh_compiler,"compile_x86_64");
	codev->devcomp->ilcomp = 0;
	codev->devlink->link = 0;
	codev->devlink->bind_ksyms = bind_ksyms_default;
	codev->devops = &devops_x86_64;
	codev->devcmds = &devcmds_x86_64;
#elif defined(__arm__)
	codev->devcomp->comp = dlsym(dlh_compiler,"compile_arm32");;
	codev->devcomp->ilcomp = 0;
	codev->devlink->link = 0;
	codev->devlink->bind_ksyms = bind_ksyms_default;
	codev->devops = &devops_x86_64;
	codev->devcmds = &devcmds_x86_64; 
#else
#error unsupported architecture
#endif

//	if (called) return 0;

	/***
	 *** set devstate
	 ***/

	if (!codev->devcomp->comp) {
		printcl( CL_WARNING "no compiler, dlsym failure");
		codev->devstate->compiler_avail = 0;
	} else {
		codev->devstate->compiler_avail = 1;
		codev->devinfo->devsup |= COPRTHR_DEVSUP_F_COMPILER;
	}

	int i;

	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);

	if (getenv("COPRTHR_MAX_NUM_ENGINES"))
		ncore = min(ncore,atoi(getenv("COPRTHR_MAX_NUM_ENGINES")));

	codev->devstate->avail = 1; /* device is available */
	CPU_ZERO(&(codev->devstate->cpumask));
	for(i=0;i<ncore;i++) CPU_SET(i,&(codev->devstate->cpumask));
	codev->devstate->cpu.veid_base = 0;
	codev->devstate->cpu.nve = ncore;

	codev->devstate->cmdq = 0;

	codev->devstate->locked_pid = 0;


//	printcl( CL_DEBUG "returning codev %p",codev);

//	if (called) return 0;

	coprthr_register_device(codev);

	if (called) return 0;

//	return codev;
	called = 1;
	return 0;

}

coprthr_device_init(init_device_x86_64);

