/* device.c
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

#define _GNU_SOURCE
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

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
#if defined(__x86_64__)
#include "cmdcall_x86_64.h"
#include "cmdcall_x86_64_sl.h"
#include "cmdcall_x86_64_ser.h"
#elif defined(__arm__)
#include "cmdcall_arm.h"
#else
#error unsupported architecture
#endif
//#include "cmdcall_atigpu.h"
#include "compiler.h"
#include "program.h"

//#ifdef ENABLE_ATIGPU
//#include "cal.h"
//#include "calcl.h"
//#include "compiler_atigpu.h"
//#endif


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

	struct _cl_device_id* dtab = *p_dtab = (struct _cl_device_id*)
		malloc(*p_ndevices*sizeof(struct _cl_device_id));

	__init_device_id(dtab);

	dtab[0].imp = (struct _imp_device){
		CL_DEVICE_TYPE_CPU,
		0,				/* vendorid */
		1,				/* max_compute_units */
		3,1,1,1,0,	/* max_wi_dim,max_wi_sz[] */
		1,				/* max_wg_sz */
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


#if defined(__x86_64__)	
	dtab[0].imp.comp = (void*)compile_x86_64;
	dtab[0].imp.ilcomp = 0;
	dtab[0].imp.link = 0;
	dtab[0].imp.bind_ksyms = bind_ksyms_default;
//	dtab[0].imp.v_cmdcall = cmdcall_x86_64;
	dtab[0].imp.v_cmdcall = cmdcall_x86_64_sl;
//	dtab[0].imp.v_cmdcall = cmdcall_x86_64_ser;
#elif defined(__arm__)
   dtab[0].imp.comp = (void*)compile_arm;
   dtab[0].imp.ilcomp = 0;
   dtab[0].imp.link = 0;
	dtab[0].imp.bind_ksyms = bind_ksyms_default;
   dtab[0].imp.v_cmdcall = cmdcall_arm;
#else
#error unsupported architecture
#endif


//	int i;
//	CPU_ZERO(&dtab[0].imp.cpumask);
//	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);
//	for(i=0;i<ncore;i++) CPU_SET(i,&dtab[0].imp.cpumask);

//	dtab[0].imp.cpu.veid_first = 0;
//	dtab[0].imp.cpu.veid_last = ncore-1;

	int i;

	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);

	if (getenv("COPRTHR_VCORE_NE"))
		ncore = min(ncore,atoi(getenv("COPRTHR_VCORE_NE")));

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


//	printcl( CL_DEBUG "calling vcproc_startup");
	printcl( XCL_WARNING "vcproc_startup is disabled");
//	vcproc_startup(0);


//#ifdef ENABLE_ATIGPU
//	__do_discover_devices_atigpu( p_ndevices, p_dtab, p_dstrtab );
//#endif
		
}


#if(0)

/* XXX keep ATI stuff around for reference, for a while - DAR */

static int __cal_init = 0;

void __attribute((__constructor__)) _init_cal_ctor()
{
	__cal_init = (calInit()==CAL_RESULT_OK)? 1 : 0;
	if (__cal_init) { printcl( CL_DEBUG "CAL init ok"); }
	else { printcl( CL_WARNING "CAL init failed"); }
}

void __attribute__((__destructor__)) _fini_cal_dtor()
{
	if (!__cal_init) return;

	__cal_init = 0;

	if (calShutdown() != CAL_RESULT_OK) 
		printcl( CL_WARNING "calShutdown failed");
}

void __do_discover_devices_atigpu(
	unsigned int* p_ndevices, 
	struct _cl_device_id** p_dtab, 
	struct _strtab_entry* p_dstrtab 
)
{

	if (!__cal_init) return;

/* XXX need to connect p_dstrtab with dstrtab in the CPU stuff above */

	p_dstrtab->alloc_sz = 1024;
	p_dstrtab->buf = (char*)malloc(p_dstrtab->alloc_sz);
	p_dstrtab->buf[0] = '\0';
	p_dstrtab->sz = 1;

	unsigned int ncpu = sysconf(_SC_NPROCESSORS_ONLN);

	/*
	 * check for ATI GPUs
	 */

	CALuint ngpu;
	calDeviceGetCount(&ngpu);

	unsigned int devnum = *p_ndevices;
	unsigned int ndev = *p_ndevices += (unsigned int)ngpu;

	printcl( CL_DEBUG "found %d ATI GPUs",ngpu);

	if (*p_dtab) {
		*p_dtab = (struct _cl_device_id*)
			realloc(*p_dtab,ndev*sizeof(struct _cl_device_id));
	} else {	
		*p_dtab = (struct _cl_device_id*)
			malloc(ndev*sizeof(struct _cl_device_id));
	}

	struct _cl_device_id* dtab = *p_dtab;

	// calGetVersion

	int n;
	for(n=0;n<ngpu;n++,devnum++) {

		CALdevice dev = 0;
		calDeviceOpen(&dev,n);

		CALdeviceinfo info;
		calDeviceGetInfo(&info,n);

		printcl( CL_DEBUG "[%d] ATI GPU target %d",
			devnum,info.target);
		printcl( CL_DEBUG 
			"[%d] ATI GPU max res width1d, width2d, height2d %d %d %d",devnum,
			info.maxResource1DWidth,
			info.maxResource2DWidth,
			info.maxResource2DHeight);

		CALdeviceattribs attr;
		attr.struct_size = sizeof(CALdeviceattribs);
		calDeviceGetAttribs(&attr,n);

		printcl( CL_DEBUG "[%d] ATI GPU target %d",devnum,attr.target);
		printcl( CL_DEBUG "[%d] ATI GPU local mem %d",devnum,attr.localRAM);
		printcl( CL_DEBUG "[%d] ATI GPU uncached remote mem %d",
			devnum,attr.uncachedRemoteRAM);
		printcl( CL_DEBUG "[%d] ATI GPU cached remote mem %d",
			devnum,attr.cachedRemoteRAM);
		printcl( CL_DEBUG "[%d] ATI GPU engine clk %d",
			devnum,attr.engineClock);
		printcl( CL_DEBUG "[%d] ATI GPU mem clk %d",
			devnum,attr.memoryClock);
		printcl( CL_DEBUG "[%d] ATI GPU wavefront size %d",
			devnum,attr.wavefrontSize);
		printcl( CL_DEBUG "[%d] ATI GPU nsimd %d",devnum,attr.numberOfSIMD);
		printcl( CL_DEBUG "[%d] ATI GPU double %d",
			devnum,attr.doublePrecision);
		printcl( CL_DEBUG "[%d] ATI GPU local data share %d",
			devnum,attr.localDataShare);
		printcl( CL_DEBUG "[%d] ATI GPU global data share %d",
			devnum,attr.globalDataShare);
		printcl( CL_DEBUG "[%d] ATI GPU global GPR %d",
			devnum,attr.globalGPR);
		printcl( CL_DEBUG "[%d] ATI GPU computeShader %d",
			devnum,attr.computeShader);
		printcl( CL_DEBUG "[%d] ATI GPU memExport %d",
			devnum,attr.memExport);
		printcl( CL_DEBUG "[%d] ATI GPU pitch align %d",
			devnum,attr.pitch_alignment);
		printcl( CL_DEBUG "[%d] ATI GPU nuav %d",
			devnum,attr.numberOfUAVs);
		printcl( CL_DEBUG "[%d] ATI GPU bUAVMemExport %d",
			devnum,attr.bUAVMemExport);
		printcl( CL_DEBUG "[%d] ATI GPU b3dProgramGrid %d",
			devnum,attr.b3dProgramGrid);
		printcl( CL_DEBUG "[%d] ATI GPU nshaderengines %d",
			devnum,attr.numberOfShaderEngines);
		printcl( CL_DEBUG "[%d] ATI GPU targetrev %d",
			devnum,attr.targetRevision);

		dtab[devnum].imp = (struct _imp_device){
			CL_DEVICE_TYPE_GPU,
			0,				/* vendorid */
			1,				/* max_compute_units */
			3,1,1,1,0,	/* max_wi_dim,max_wi_sz[] */
			1,				/* max_wg_sz */
			8,4,2,1,2,1,	/* pref_char/short/int/long/float/double/n */
			attr.engineClock,				/* max_freq */
			64,			/* bits */
			128*1024*1024,		/* max_mem_alloc_sz */
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
			CL_FALSE,	/* compiler_avail */
			CL_EXEC_KERNEL,	/* supp_exec_cap */
			CL_QUEUE_PROFILING_ENABLE, /* cmdq_prop */
			(cl_platform_id)(-1),	/* platformid */
			0, 	/* name */
			0, 	/* vendor */
			0, 	/* drv_version */
			0, 	/* profile */
			0, 	/* version */
			0,		/* extensions */
			0,0 	/* dstrtab, dstrtab_sz */
		};

		char* dstrtab = (char*)malloc(4096);
		size_t dstrtab_sz = 1;
		dstrtab[devnum] = '\0';

		dtab[devnum].imp.name = dstrtab;
		dtab[devnum].imp.vendor = dstrtab;
		dtab[devnum].imp.drv_version = dstrtab;
		dtab[devnum].imp.profile = dstrtab;
		dtab[devnum].imp.version = dstrtab;
		dtab[devnum].imp.extensions = dstrtab;

		dtab[devnum].imp.compiler_avail = CL_TRUE;
		dtab[devnum].imp.comp = (void*)compile_atigpu;
		dtab[devnum].imp.ilcomp = (void*)ilcompile_atigpu;
		dtab[devnum].imp.link = 0;
		dtab[devnum].imp.v_cmdcall = cmdcall_atigpu;

		CPU_ZERO(&dtab[devnum].imp.cpumask);
		CPU_SET(n%ncpu,&dtab[devnum].imp.cpumask);


		/* XXX copy these into the dtab, alternative would be to initialize
		 * XXX the dtab struct prior to the CAL open, get info calls -DAR */

		memcpy(&dtab[devnum].imp.atigpu.caldev,&dev,sizeof(CALdevice));
		memcpy(&dtab[devnum].imp.atigpu.calinfo,&info,sizeof(CALdeviceinfo));


		printcl( CL_DEBUG "finished devnum %d",devnum);

	}


	printcl( CL_DEBUG "check again target %d",dtab[1].imp.atigpu.calinfo.target);

}

#endif


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



