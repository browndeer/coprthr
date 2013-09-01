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

//#define _GNU_SOURCE
//#include <sched.h>

#include <CL/cl.h>

#include "xcl_structs.h"
#include "platform.h"
#include "device.h"
#include "cmdcall.h"
//#if defined(__x86_64__)
#include "cmdcall_x86_64.h"
#include "cmdcall_x86_64_sl.h"
#include "cmdcall_x86_64_ser.h"
//#elif defined(__arm__)
//#include "cmdcall_arm.h"
//#else
//#error unsupported architecture
//#endif
//#include "cmdcall_atigpu.h"
#include "compiler.h"
#include "program.h"

//#ifdef ENABLE_ATIGPU
//#include "cal.h"
//#include "calcl.h"
//#include "compiler_atigpu.h"
//#endif

void* dlh_compiler = 0;


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

//void __do_discover_devices_atigpu(
//	unsigned int* p_ndevices, 
//	struct _cl_device_id** p_dtab, 
//	struct _strtab_entry* p_dstrtab 
//);

void __do_discover_devices(
	unsigned int* p_ndevices, 
	struct _cl_device_id** p_dtab, 
	struct _strtab_entry* p_dstrtab 
)
{
	int devnum;

	if (*p_dtab) return;

	p_dstrtab->alloc_sz = 1024;
	p_dstrtab->buf = (char*)malloc(p_dstrtab->alloc_sz);
	p_dstrtab->buf[0] = '\0';
	p_dstrtab->sz = 1;


	/* assume we have at least one multicore CPU device */

	unsigned int ncpu = 1;
	*p_ndevices = ncpu;

#ifdef ENABLE_OFFLINE_DEVICES
	*p_ndevices += 1;
#endif

	struct _cl_device_id* dtab = *p_dtab = (struct _cl_device_id*)
		malloc(*p_ndevices*sizeof(struct _cl_device_id));

	__init_device_id(dtab); // XXX this only initializesfirst entry

#ifdef ENABLE_OFFLINE_DEVICES
	__init_device_id(dtab+1);
#endif

	dtab[0].imp = (struct _imp_device){
		CL_DEVICE_TYPE_CPU,
		0,				/* vendorid */
		1,				/* max_compute_units */
		3,1,1,1,0,	/* max_wi_dim,max_wi_sz[] */
		64,				/* max_wg_sz */
		8,4,2,1,2,1,	/* pref_char/short/int/long/float/double/n */
		0,				/* max_freq */
		64,			/* bits */
		1024*1024*1024,		/* max_mem_alloc_sz */
		CL_FALSE,	/* supp_img */
		0,0, 			/* img_max_narg_r, img_max_narg_w */
		0,0,			/* img2d_max_width, img2d_max_height */
		0,0,0,		/*  img3d_max_width, img3d_max_height, img3d_max_depth */
		0, 			/* max_samplers */
		256, 			/* max_param_sz */
		64, 			/* mem_align_bits */
		8, 			/* datatype_align_sz */
		CL_FP_ROUND_TO_NEAREST|CL_FP_INF_NAN, /* single_fp_config */
		CL_NONE, 	/* global_mem_cache_type */
		0, 			/* global_mem_cacheline_sz */
		0, 			/* global_mem_cache_sz */
		0, 			/* global_mem_sz */
		65536, 		/* cl_ulong max_const_buffer_sz */
		8, 			/* max_const_narg */
		CL_GLOBAL, 	/* local_mem_type */
		0, 			/* local_mem_sz */
		CL_FALSE,	/* supp_ec */
		0, 			/* prof_timer_res */
		CL_TRUE,		/* endian_little */
		CL_FALSE, 	/* avail */
		CL_TRUE,	/* compiler_avail */
		CL_EXEC_KERNEL,	/* supp_exec_cap */
		CL_QUEUE_PROFILING_ENABLE, /* cmdq_prop */
		(cl_platform_id)(-1),	/* platformid */
		0, 	/* name */
		0, 	/* vendor */
		0, 	/* drv_version */
		0, 	/* profile */
		0, 	/* version */
		"cl_khr_icd",		/* extensions */
		0,0 	/* dstrtab, dstrtab_sz */
	};

	char* dstrtab = (char*)malloc(4096);
	size_t dstrtab_sz = 1;
	dstrtab[0] = '\0';

	dtab[0].imp.name = dstrtab;
	dtab[0].imp.vendor = dstrtab;
	dtab[0].imp.drv_version = dstrtab;
	dtab[0].imp.profile = dstrtab;
	dtab[0].imp.version = dstrtab;
	dtab[0].imp.extensions = dstrtab;


	FILE* fp;
	struct stat fs;
	char buf[1024];
	size_t sz;

#if defined(__FreeBSD__)

	int val=0;
	sz=4;
	sysctlbyname("hw.ncpu",&val,&sz,0,0);
	printf("ncpu %d %d\n",val,sz);
	dtab[0].imp.max_compute_units = val;

	sz=4;
	sysctlbyname("hw.clockrate",&val,&sz,0,0);
	printf("clockrate %d %d\n",val,sz);
	dtab[0].imp.max_freq = val;

	sz=1024;
	sysctlbyname("hw.model",buf,&sz,0,0);
	printf("model %s %d\n",buf,sz);

	char* bufp = truncate_ws(buf);
	sz = 1+strnlen(bufp,__CLMAXSTR_LEN);
	strncpy(dstrtab+dstrtab_sz,bufp,sz);
	dtab[0].imp.name = dstrtab+dstrtab_sz;
	dstrtab_sz += sz;

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

			dtab[0].imp.max_compute_units = atoi(right);

		} else if (!strncasecmp(left,"cpu MHz",7)) {

			dtab[0].imp.max_freq = atoi(right);

		} else if (!strncasecmp(left,"model name",10)) {

			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			strncpy(dstrtab+dstrtab_sz,right,sz);
			dtab[0].imp.name = dstrtab+dstrtab_sz;
			dstrtab_sz += sz;

		} else if (!strncasecmp(left,"vendor_id",9)) {

			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			strncpy(dstrtab+dstrtab_sz,right,sz);
			dtab[0].imp.vendor = dstrtab+dstrtab_sz;
			dstrtab_sz += sz;

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

//		if (!strncasecmp(left,"MemFree",7)) {
//
//			dtab[0].imp.max_mem_alloc_sz = (1024/4)*atoi(right);
//		}

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

//			if (atoi(right) > 0) break;
			dtab[0].imp.max_compute_units = atoi(right) + 1;

//		} else if (!strncasecmp(left,"cpu count",8)) {
//
//			dtab[0].imp.max_compute_units = atoi(right);
//
//		} else if (!strncasecmp(left,"cpu MHz",7)) {
//
//			dtab[0].imp.max_freq = atoi(right);
//
//		} else if (!strncasecmp(left,"model name",10)) {
		} else if (!strncmp(left,"Processor",9)) {

			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			strncpy(dstrtab+dstrtab_sz,right,sz);
			dtab[0].imp.name = dstrtab+dstrtab_sz;
			dstrtab_sz += sz;

//		} else if (!strncasecmp(left,"vendor_id",9)) {
		} else if (!strncasecmp(left,"CPU implementer",15)) {

			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			strncpy(dstrtab+dstrtab_sz,right,sz);
			dtab[0].imp.vendor = dstrtab+dstrtab_sz;
			dstrtab_sz += sz;

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

//		if (!strncasecmp(left,"MemFree",7)) {
//
//			dtab[0].imp.max_mem_alloc_sz = (1024/4)*atoi(right);
//		}

	}

	fclose(fp);

#endif

dlh_compiler = dlopen("libcoprthrcc.so",RTLD_LAZY);
if (!dlh_compiler) 
	printcl( CL_WARNING "no compiler,failed to load libcoprthrcc.so");

#if defined(__x86_64__)	
//	dtab[0].imp.comp = (void*)compile_x86_64;
	dtab[0].imp.comp = dlsym(dlh_compiler,"compile_x86_64");
	dtab[0].imp.ilcomp = 0;
	dtab[0].imp.link = 0;
	dtab[0].imp.bind_ksyms = bind_ksyms_default;
	dtab[0].imp.v_cmdcall = cmdcall_x86_64_sl;
#elif defined(__arm__)
//   dtab[0].imp.comp = (void*)compile_arm;
//   dtab[0].imp.comp = dlsym(dlh_compiler,"compile_arm");;
   dtab[0].imp.comp = dlsym(dlh_compiler,"compile_arm32");;
   dtab[0].imp.ilcomp = 0;
   dtab[0].imp.link = 0;
	dtab[0].imp.bind_ksyms = bind_ksyms_default;
   dtab[0].imp.v_cmdcall = cmdcall_x86_64_sl; /* XXX fix naming -DAR */
#else
#error unsupported architecture
#endif

if (!dtab[0].imp.comp)
	printcl( CL_WARNING "no compiler, dlsym failure");


#ifdef ENABLE_OFFLINE_DEVICE
	dtab[1].imp.comp = dlsym(dlh_compiler,"compile_android_arm32");
	if (!dtab[1].imp.comp)
		printcl( CL_WARNING "no offline compiler, dlsym failure");
	else
		printcl( CL_DEBUG "found offline compiler");
#endif


	int i;

	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);

	if (getenv("COPRTHR_MAX_NUM_ENGINES"))
		ncore = min(ncore,atoi(getenv("COPRTHR_MAX_NUM_ENGINES")));

//ncore = 1;

#ifdef ENABLE_NCPU
//	char buf[256];

	printcl( CL_DEBUG "checking for force_ncpu");

	if (!getenv_token("COPRTHR_OCL","force_ncpu",buf,256)) ncpu = atoi(buf);

	if (ncore%ncpu) {
		printcl( CL_WARNING "force_ncpu ignored, must be multiple of ncore");
		ncpu = 1;
	}

	if (ncpu > 1) {

		printcl( CL_WARNING "force_ncpu %d",ncpu);

		printcl( CL_DEBUG "force_ncpu = %d",ncpu);

		*p_dtab = (struct _cl_device_id*)
			realloc(*p_dtab,(ncpu-1)*sizeof(struct _cl_device_id));

		for(devnum=1;devnum<ncpu;devnum++) 
			memcpy((*p_dtab)+devnum,(*p_dtab),sizeof(struct _cl_device_id));

		int cpd = ncore/ncpu;
		for(devnum=0;devnum<ncpu;devnum++) {
			CPU_ZERO(&dtab[devnum].imp.cpumask);
			for(i=devnum*cpd;i<(devnum+1)*cpd;i++) 
				CPU_SET(i,&dtab[devnum].imp.cpumask);
			dtab[devnum].imp.cpu.veid_base = devnum*cpd;
			dtab[devnum].imp.cpu.nve = cpd;
			
			printcl( CL_DEBUG "devnum base nve %d %d %d",
				devnum,dtab[devnum].imp.cpu.veid_base,dtab[devnum].imp.cpu.nve);
		}

		*p_ndevices = ncpu;

	} else {
		CPU_ZERO(&dtab[0].imp.cpumask);
		for(i=0;i<ncore;i++) CPU_SET(i,&dtab[0].imp.cpumask);
		dtab[0].imp.cpu.veid_base = 0;
		dtab[0].imp.cpu.nve = ncore;
	}
#else
	CPU_ZERO(&dtab[0].imp.cpumask);
	for(i=0;i<ncore;i++) CPU_SET(i,&dtab[0].imp.cpumask);
	dtab[0].imp.cpu.veid_base = 0;
	dtab[0].imp.cpu.nve = ncore;
#endif


}


void __do_release_devices(
	struct _cl_device_id* dtab,
	struct _strtab_entry* dstrtab

)
{

	if (dtab) free(dtab);
	if (dstrtab->buf) free(dstrtab->buf);
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
			dtab[devnum].imp.devtype,devtype);
//		if (dtab[devnum].imp.devtype == devtype) n++;
		if (dtab[devnum].imp.devtype & devtype) n++;
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

//printf("devtype %d\n",devtype);

	for(devnum=0;devnum<ndevices;devnum++) 
//		if (n<ndev && dtab[devnum].imp.devtype == devtype) 
		if (n<ndev && dtab[devnum].imp.devtype & devtype) 
			devices[n++] = &__resolve_platformid(platformid,dtab[devnum]);

}



