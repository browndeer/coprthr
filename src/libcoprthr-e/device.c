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
#include <string.h>

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include <CL/cl.h>

#include "xcl_config.h"
#include "xcl_structs.h"
#include "printcl.h"
#include "platform.h"
#include "device.h"
#include "cmdcall.h"

#if defined(__x86_64__) || defined(__arm__)
#ifdef ENABLE_EMEK_BUILD
#include "cmdcall_e32pth.h"
#else
#include "cmdcall_e32pth_needham.h"
//#include "cmdcall_e32pth_needhampro.h"
//#include "cmdcall_e32pth_blank.h"
#endif
#else
#error unsupported architecture
#endif

#include "compiler.h"
#include "program.h"

#include "dmalloc.h"

#include <e_host.h>
#include "e_platform.h"

//#define ENABLE_UVA

/***** temporary e32 stuff -DAR */
void* loaded_srec = 0;
int e_opened = 0;

#ifdef ENABLE_EMEK_BUILD

char servIP[] = "127.0.0.1";
const unsigned short eServLoaderPort = 50999;

#else

/* XXX zynq dram alloc and epiphany -DAR */
DRAM_t e_dram;
Epiphany_t e_epiphany;

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


void __do_discover_devices(
	unsigned int* p_ndevices, 
	struct _cl_device_id** p_dtab, 
	struct _strtab_entry* p_dstrtab 
)
{
	int err;

	int devnum;

	if (*p_dtab) return;

	p_dstrtab->alloc_sz = 1024;
	p_dstrtab->buf = (char*)malloc(p_dstrtab->alloc_sz);
	p_dstrtab->buf[0] = '\0';
	p_dstrtab->sz = 1;


	unsigned int ncpu = 1;
	*p_ndevices = ncpu;

	struct _cl_device_id* dtab = *p_dtab = (struct _cl_device_id*)
		malloc(*p_ndevices*sizeof(struct _cl_device_id));

	__init_device_id(dtab);

	dtab[0].imp = (struct _imp_device){
		CL_DEVICE_TYPE_ACCELERATOR,
		0,				/* vendorid */
		1,				/* max_compute_units */
		3,				/* max_wi_dim */
		1024,1024,1024,0,	/* max_wi_sz[] */
		16,				/* max_wg_sz */
		4,2,1,8,1,8,	/* pref_char/short/int/long/float/double/n */
		0,				/* max_freq */
		32,			/* bits */
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
		CL_LOCAL, 	/* local_mem_type */
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
		0,		/* extensions */
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

#elif defined(__x86_64__) || defined(__i386__) || defined(__arm__)


/* XXX hardcoded device information for now -DAR */

//	dtab[0].imp.max_compute_units = 16;
//	dtab[0].imp.max_freq = 1000;
//
//   sz = 1+strnlen("E16G",__CLMAXSTR_LEN);
//   strncpy(dstrtab+dstrtab_sz,"E16G",sz);
//   dtab[0].imp.name = dstrtab+dstrtab_sz;
//   dstrtab_sz += sz;
//
//	sz = 1+strnlen("Adapteva, Inc.",__CLMAXSTR_LEN);
//	strncpy(dstrtab+dstrtab_sz,"Adapteva, Inc.",sz);
//	dtab[0].imp.vendor = dstrtab+dstrtab_sz;
//	dstrtab_sz += sz;


#else // XXX else what? -DAR

/*
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

			dtab[0].imp.max_compute_units = atoi(right) + 1;

		} else if (!strncmp(left,"Processor",9)) {

			right = truncate_ws(right);
			sz = 1+strnlen(right,__CLMAXSTR_LEN);
			strncpy(dstrtab+dstrtab_sz,right,sz);
			dtab[0].imp.name = dstrtab+dstrtab_sz;
			dstrtab_sz += sz;

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

	}

	fclose(fp);
*/

#endif


#if defined(__x86_64__)	|| defined(__arm__)

	dtab[0].imp.ilcomp = 0;
	dtab[0].imp.link = 0;
	dtab[0].imp.bind_ksyms = bind_ksyms_default;

#else
#error unsupported architecture
#endif


/* QQQ
	int i;

	unsigned int ncore = sysconf(_SC_NPROCESSORS_ONLN);

	if (getenv("COPRTHR_VCORE_NE"))
		ncore = min(ncore,atoi(getenv("COPRTHR_VCORE_NE")));
*/


//#ifdef ENABLE_NCPU

/*
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
*/

//#else
/* QQQ
	CPU_ZERO(&dtab[0].imp.cpumask);
	for(i=0;i<ncore;i++) CPU_SET(i,&dtab[0].imp.cpumask);
	dtab[0].imp.cpu.veid_base = 0;
	dtab[0].imp.cpu.nve = ncore;
*/
//#endif


#ifdef ENABLE_EMEK_BUILD

   if (e_open((char*)servIP, eServLoaderPort)) {
      printcl( CL_ERR "Cannot establish connection to E-SERVER!");
      exit(-1);
   }
   printcl( CL_DEBUG "connect e-server at %s:%d",servIP,eServLoaderPort);

   e_opened = 1;

	struct e_platform_info_struct einfo;
//	e_get_platform_info( &e_epiphany, &einfo );
	e_get_platform_info( 0, &einfo );

//	printcl( CL_DEBUG "epiphany platform info:");
//	printcl( CL_DEBUG "\tplatform_name '%s'",einfo.e_platform_name);
//	printcl( CL_DEBUG "\tdevice_id %d",einfo.e_device_id);
//	printcl( CL_DEBUG "\tglobal_mem_base %p",einfo.e_global_mem_base);
//	printcl( CL_DEBUG "\tglobal_mem_size %ld",einfo.e_global_mem_size);
//	printcl( CL_DEBUG "\tcore_local_mem_size %ld",einfo.e_core_local_mem_size);
//	printcl( CL_DEBUG "\tcore_base_addr %p",einfo.e_core_base_addr);
//	printcl( CL_DEBUG "\tarray_ncol %d",einfo.e_array_ncol);
//	printcl( CL_DEBUG "\tarray_nrow %d",einfo.e_array_nrow);

//	devmembase = einfo.e_global_mem_base;
//	devmemlo = devmembase + 0x4000;
//	devmemhi = devmemlo + einfo.e_global_mem_size;
//
//	dmalloc_reset();
//
//	dtab[0].imp.max_compute_units = 16;
//	dtab[0].imp.max_freq = 1000;
//
//   sz = 1+strnlen(einfo.e_platform_name,__CLMAXSTR_LEN);
//   strncpy(dstrtab+dstrtab_sz,einfo.e_platform_name,sz);
//   dtab[0].imp.name = dstrtab+dstrtab_sz;
//   dstrtab_sz += sz;
//
//	sz = 1+strnlen("Adapteva, Inc.",__CLMAXSTR_LEN);
//	strncpy(dstrtab+dstrtab_sz,"Adapteva, Inc.",sz);
//	dtab[0].imp.vendor = dstrtab+dstrtab_sz;
//	dstrtab_sz += sz;

//	dtab[0].imp.e32.core_local_mem_size = einfo.e_core_local_mem_size;
//	dtab[0].imp.e32.core_base_addr = einfo.e_core_base_addr;
//	dtab[0].imp.e32.array_ncol = einfo.e_array_ncol;
//	dtab[0].imp.e32.array_nrow = einfo.e_array_nrow;
//	dtab[0].imp.e32.ncore = (einfo.e_array_ncol)*(einfo.e_array_nrow);

//	dtab[0].imp.vendorid = einfo.e_device_id;
//	dtab[0].imp.global_mem_sz = einfo.e_global_mem_size;
//	dtab[0].imp.local_mem_sz = einfo.e_core_local_mem_size;
//	dtab[0].imp.comp = (void*)compile_e32;
//	dtab[0].imp.v_cmdcall = cmdcall_e32pth;

#else

	e_open(&e_epiphany);

   e_opened = 1;

	printcl( CL_DEBUG "e_alloc using &e_dram %p", &e_dram);

	err = e_alloc( &e_dram, 0, (8192*4096) );

	if (err) 
		printcl( CL_ERR "e_alloc returned %d", err);
	else 
		printcl( CL_DEBUG "e_alloc ok, returned %d", err);

	printcl( CL_DEBUG "dram alloc:" );
	printcl( CL_DEBUG 
		"map_size=%ld map_mask=0x%x phy_base=0x%x mapped_base=%p base=%p"
		" memfd=%d", e_dram.map_size,e_dram.map_mask,e_dram.phy_base,
		e_dram.mapped_base,e_dram.base,e_dram.memfd );

#ifdef ENABLE_UVA	
	e_dram.mapped_base = mremap(e_dram.mapped_base,(8192*4096),(8192*4096),
		MREMAP_MAYMOVE|MREMAP_FIXED,(void*)0x8e000000 );

	if (e_dram.mapped_base == (void*)(-1) ) {
		perror(0);
		exit(-1);
	} else {
		e_dram.base = e_dram.mapped_base;
	}

	printcl( CL_DEBUG "dram alloc after mremap:" );
	printcl( CL_DEBUG 
		"map_size=%ld map_mask=0x%x phy_base=0x%x mapped_base=%p base=%p"
		" memfd=%d", e_dram.map_size,e_dram.map_mask,e_dram.phy_base,
		e_dram.mapped_base,e_dram.base,e_dram.memfd );
#endif

	
	struct e_platform_info_struct einfo;
	e_get_platform_info( &e_epiphany, &einfo );

//	printcl( CL_DEBUG "epiphany platform info:");
//	printcl( CL_DEBUG "\tplatform_name '%s'",einfo.e_platform_name);
//	printcl( CL_DEBUG "\tdevice_id %d",einfo.e_device_id);
//	printcl( CL_DEBUG "\tglobal_mem_base %p",einfo.e_global_mem_base);
//	printcl( CL_DEBUG "\tglobal_mem_size %ld",einfo.e_global_mem_size);
//	printcl( CL_DEBUG "\tcore_local_mem_size %ld",einfo.e_core_local_mem_size);
//	printcl( CL_DEBUG "\tcore_base_addr %p",einfo.e_core_base_addr);
//	printcl( CL_DEBUG "\tarray_ncol %d",einfo.e_array_ncol);
//	printcl( CL_DEBUG "\tarray_nrow %d",einfo.e_array_nrow);

//	devmembase = einfo.e_global_mem_base;
//	devmemlo = devmembase + 0x4000;
//	devmemhi = devmemlo + einfo.e_global_mem_size;
//
//	dmalloc_reset();
//
//	dtab[0].imp.max_compute_units = 16;
//	dtab[0].imp.max_freq = 1000;
//
//   sz = 1+strnlen(einfo.e_platform_name,__CLMAXSTR_LEN);
//   strncpy(dstrtab+dstrtab_sz,einfo.e_platform_name,sz);
//   dtab[0].imp.name = dstrtab+dstrtab_sz;
//   dstrtab_sz += sz;
//
//	sz = 1+strnlen("Adapteva, Inc.",__CLMAXSTR_LEN);
//	strncpy(dstrtab+dstrtab_sz,"Adapteva, Inc.",sz);
//	dtab[0].imp.vendor = dstrtab+dstrtab_sz;
//	dstrtab_sz += sz;

//	dtab[0].imp.e32.core_local_mem_size = einfo.e_core_local_mem_size;
//	dtab[0].imp.e32.core_base_addr = einfo.e_core_base_addr;
//	dtab[0].imp.e32.array_ncol = einfo.e_array_ncol;
//	dtab[0].imp.e32.array_nrow = einfo.e_array_nrow;
//	dtab[0].imp.e32.ncore = (einfo.e_array_ncol)*(einfo.e_array_nrow);

//	dtab[0].imp.vendorid = einfo.e_device_id;
//	dtab[0].imp.global_mem_sz = einfo.e_global_mem_size;
//	dtab[0].imp.local_mem_sz = einfo.e_core_local_mem_size;
//	dtab[0].imp.comp = (void*)compile_e32_needham;
//	dtab[0].imp.v_cmdcall = cmdcall_e32pth_needham;

#endif

	printcl( CL_DEBUG "epiphany platform info:");
	printcl( CL_DEBUG "\tplatform_name '%s'",einfo.e_platform_name);
	printcl( CL_DEBUG "\tdevice_id %d",einfo.e_device_id);
	printcl( CL_DEBUG "\tglobal_mem_base %p",einfo.e_global_mem_base);
	printcl( CL_DEBUG "\tglobal_mem_size %ld",einfo.e_global_mem_size);
	printcl( CL_DEBUG "\tcore_local_mem_size %ld",einfo.e_core_local_mem_size);
	printcl( CL_DEBUG "\tcore_base_addr %p",einfo.e_core_base_addr);
	printcl( CL_DEBUG "\tarray_ncol %d",einfo.e_array_ncol);
	printcl( CL_DEBUG "\tarray_nrow %d",einfo.e_array_nrow);

	devmembase = einfo.e_global_mem_base;
	devmemlo = devmembase + 0x4000;
	devmemhi = devmemlo + einfo.e_global_mem_size;

	dmalloc_reset();

	dtab[0].imp.max_compute_units = 16;
	dtab[0].imp.max_freq = 1000;

   sz = 1+strnlen(einfo.e_platform_name,__CLMAXSTR_LEN);
   strncpy(dstrtab+dstrtab_sz,einfo.e_platform_name,sz);
   dtab[0].imp.name = dstrtab+dstrtab_sz;
   dstrtab_sz += sz;

	sz = 1+strnlen("Adapteva, Inc.",__CLMAXSTR_LEN);
	strncpy(dstrtab+dstrtab_sz,"Adapteva, Inc.",sz);
	dtab[0].imp.vendor = dstrtab+dstrtab_sz;
	dstrtab_sz += sz;

	dtab[0].imp.e32.core_local_mem_size = einfo.e_core_local_mem_size;
	dtab[0].imp.e32.core_base_addr = einfo.e_core_base_addr;
	dtab[0].imp.e32.array_ncol = einfo.e_array_ncol;
	dtab[0].imp.e32.array_nrow = einfo.e_array_nrow;
	dtab[0].imp.e32.ncore = (einfo.e_array_ncol)*(einfo.e_array_nrow);

	dtab[0].imp.vendorid = einfo.e_device_id;
	dtab[0].imp.global_mem_sz = einfo.e_global_mem_size;
	dtab[0].imp.local_mem_sz = einfo.e_core_local_mem_size;


#ifdef ENABLE_EMEK_BUILD
	if (!strncmp(einfo.e_platform_name,"E16G Lexington",32)) {

		dtab[0].imp.comp = (void*)compile_e32;
		dtab[0].imp.v_cmdcall = cmdcall_e32pth;

	} else {

		printfcl( CL_ERR "platform '%s' unrecognized",einfo.e_platform_name);
		return(-1);

	}
#else 
	if (!strncmp(einfo.e_platform_name,"E16G Needham",32)) {

		dtab[0].imp.comp = (void*)compile_e32_needham;
		dtab[0].imp.v_cmdcall = cmdcall_e32pth_needham;

	} else if (!strncmp(einfo.e_platform_name,"E16G Needham Pro",32)) {

		dtab[0].imp.comp = (void*)compile_e32_needhampro;
		dtab[0].imp.v_cmdcall = cmdcall_e32pth_needhampro;

	} else if (!strncmp(einfo.e_platform_name,"(blank)",32)) {

		dtab[0].imp.comp = (void*)compile_e32_blank;
		dtab[0].imp.v_cmdcall = cmdcall_e32pth_blank;

	} else {

		printfcl( CL_ERR "platform '%s' unrecognized",einfo.e_platform_name);
		return(-1);

	}
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

	if (devtype == CL_DEVICE_TYPE_DEFAULT) 
		devtype = CL_DEVICE_TYPE_ACCELERATOR; /* XXX */

	for(devnum=0;devnum<ndevices;devnum++) {
		printcl( CL_DEBUG "match devtype %d %d",
			dtab[devnum].imp.devtype,devtype);
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

	if (devtype == CL_DEVICE_TYPE_DEFAULT) 
		devtype = CL_DEVICE_TYPE_ACCELERATOR; /* XXX */

	for(devnum=0;devnum<ndevices;devnum++) 
		if (n<ndev && dtab[devnum].imp.devtype & devtype) 
			devices[n++] = &__resolve_platformid(platformid,dtab[devnum]);

}



