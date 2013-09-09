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

#include "coprthr_device.h"

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif

char* strnlen_ws( char* p, char* s, size_t maxlen)
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
		.devtype = CL_DEVICE_TYPE_CPU,
		.vendorid = 0,				/* vendorid */
		.max_compute_units = 1,				/* max_compute_units */
		.max_wi_dim = 3,
		.max_wi_sz = {1,1,1,0},	/* max_wi_dim,max_wi_sz[] */
		.max_wg_sz = 64,				/* max_wg_sz */
		.pref_charn = 8,
		.pref_shortn = 4,
		.pref_intn = 2,
		.pref_longn = 1,
		.pref_floatn = 2,
		.pref_doublen = 1,	/* pref_char/short/int/long/float/double/n */
		.max_freq = 0,				/* max_freq */
		.addr_bits = 64,			/* bits */
		.max_mem_alloc_sz = 1024*1024*1024,		/* max_mem_alloc_sz */
		.supp_img = CL_FALSE,	/* supp_img */
		.img_max_narg_r = 0, 
		.img_max_narg_w = 0, 			/* img_max_narg_r, img_max_narg_w */
		.img2d_max_width = 0,
		.img2d_max_height = 0,			/* img2d_max_width, img2d_max_height */
		.img3d_max_width = 0,
		.img3d_max_height = 0,
		.img3d_max_depth = 0,/* img3d_max_width,img3d_max_height,img3d_max_depth*/
		.max_samplers = 0, 			/* max_samplers */
		.max_param_sz = 256, 			/* max_param_sz */
		.mem_align_bits = 64, 			/* mem_align_bits */
		.datatype_align_sz = 8, 			/* datatype_align_sz */
		.single_fp_config = CL_FP_ROUND_TO_NEAREST|CL_FP_INF_NAN, /* single_fp_config */
		.global_mem_cache_type = CL_NONE, 	/* global_mem_cache_type */
		.global_mem_cacheline_sz = 0, 			/* global_mem_cacheline_sz */
		.global_mem_cache_sz = 0, 			/* global_mem_cache_sz */
		.global_mem_sz = 0, 			/* global_mem_sz */
		.max_const_buffer_sz = 65536, 		/* cl_ulong max_const_buffer_sz */
		.max_const_narg = 8, 			/* max_const_narg */
		.local_mem_type = CL_GLOBAL, 	/* local_mem_type */
		.local_mem_sz = 0, 			/* local_mem_sz */
		.supp_ec = CL_FALSE,	/* supp_ec */
		.prof_timer_res = 0, 			/* prof_timer_res */
		.endian_little = CL_TRUE,		/* endian_little */
		.supp_exec_cap = CL_EXEC_KERNEL,	/* supp_exec_cap */
		.cmdq_prop = CL_QUEUE_PROFILING_ENABLE, /* cmdq_prop */
		.platformid = (cl_platform_id)(-1),	/* platformid */
		.extensions = "cl_khr_icd",		/* extensions */
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
	codev->devops->v_cmdcall = cmdcall_x86_64_sl;
#elif defined(__arm__)
	codev->devcomp->comp = dlsym(dlh_compiler,"compile_arm32");;
	codev->devcomp->ilcomp = 0;
	codev->devlink->link = 0;
	codev->devlink->bind_ksyms = bind_ksyms_default;
	codev->devops->v_cmdcall = cmdcall_x86_64_sl; /* XXX fix naming -DAR */
#else
#error unsupported architecture
#endif

	if (!codev->devcomp->comp) {
		printcl( CL_WARNING "no compiler, dlsym failure");
		codev->devstate->compiler_avail = CL_FALSE;
	} else {
		codev->devstate->compiler_avail = CL_TRUE;
	}


	int i;

	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);

	if (getenv("COPRTHR_MAX_NUM_ENGINES"))
		ncore = min(ncore,atoi(getenv("COPRTHR_MAX_NUM_ENGINES")));

	codev->devstate->avail = CL_TRUE;
	CPU_ZERO(&(codev->devstate->cpumask));
	for(i=0;i<ncore;i++) CPU_SET(i,&(codev->devstate->cpumask));
	codev->devstate->cpu.veid_base = 0;
	codev->devstate->cpu.nve = ncore;

	codev->devstate->cmdq = 0;

	printcl( CL_DEBUG "returning codev %p",codev);

	return codev;

}


