/* imp_structs.h
 *
 * Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
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

#ifndef _imp_structs_h
#define _imp_structs_h

#include <sys/queue.h>
#include <pthread.h>

#ifdef ENABLE_ATIGPU
#include "cal.h"
#endif

#include "cpuset_type.h"
#include "cmdcall.h"

#ifndef min
#define min(a,b) ((a<b)?a:b)
#endif
#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif


#define CLARG_KIND_UNDEFINED	0x000
#define CLARG_KIND_VOID       0x001
#define CLARG_KIND_DATA       0x002
#define CLARG_KIND_GLOBAL     0x004
#define CLARG_KIND_LOCAL      0x008
#define CLARG_KIND_CONSTANT   0x010
#define CLARG_KIND_SAMPLER    0x020
#define CLARG_KIND_IMAGE2D    0x040
#define CLARG_KIND_IMAGE3D    0x080


struct _strtab_entry {
   size_t alloc_sz;
   size_t sz;
   char* buf;
};

struct _clsymtab_entry {
	cl_uint kind;
	cl_uint type;
};

#define CLSYM_KIND_


/* device */

struct _imp_device {

	cl_device_type devtype;
	cl_uint vendorid;
	cl_uint max_compute_units;
	cl_uint max_wi_dim;
	size_t max_wi_sz[4];
	size_t max_wg_sz;
	cl_uint pref_charn;
	cl_uint pref_shortn;
	cl_uint pref_intn;
	cl_uint pref_longn;
	cl_uint pref_floatn;
	cl_uint pref_doublen;
	cl_uint max_freq;
	cl_uint addr_bits;
	cl_ulong max_mem_alloc_sz;
	cl_bool supp_img;
	cl_uint img_max_narg_r;
	cl_uint img_max_narg_w;
	size_t img2d_max_width;
	size_t img2d_max_height;
	size_t img3d_max_width;
	size_t img3d_max_height;
	size_t img3d_max_depth;
	cl_uint max_samplers;
	size_t max_param_sz;
	cl_uint mem_align_bits;
	cl_uint datatype_align_sz;
	cl_device_fp_config single_fp_config;
	cl_device_mem_cache_type global_mem_cache_type;
	cl_uint global_mem_cacheline_sz;
	cl_ulong global_mem_cache_sz;
	cl_ulong global_mem_sz;
	cl_ulong max_const_buffer_sz;
	cl_uint max_const_narg;
	cl_device_local_mem_type local_mem_type;
	cl_ulong local_mem_sz;
	cl_bool supp_ec;
	size_t prof_timer_res;
	cl_bool endian_little;
	cl_bool avail;
	cl_bool compiler_avail;
	cl_device_exec_capabilities supp_exec_cap;
	cl_command_queue_properties cmdq_prop;
	cl_platform_id platformid;
	char* name;
	char* vendor;
	char* drv_version;
	char* profile;
	char* version;
	char* extensions;

	char* dstrtab;
	size_t dstrtab_sz;

	void* (*comp)(void*);
	void* (*ilcomp)(void*);
	void* (*link)(void*);

	cmdcall_t* v_cmdcall;

	cpu_set_t cpumask;

	union {

		struct {
			unsigned int veid_base;
			unsigned int nve;
		} cpu;

#ifdef ENABLE_ATIGPU
		struct {
			CALdevice caldev;
			CALdeviceinfo calinfo;
		} atigpu;
#endif
		
	};

};

#define __imp_init_device(imp) do { \
	bzero(&imp,sizeof(_imp_device)); \
	} while(0)

#define __imp_free_device(imp) do {} while(0)

#define __resolve_devid(d,m) ((d)->imp.m)



/* platform */

struct _imp_platform {
   char* profile;
   char* version;
   char* name;
   char* vendor;
   char* extensions;
   unsigned int ndevices;
   struct _cl_device_id* dtab;
   struct _strtab_entry* dstrtab;
};

#define __imp_init_platform(imp) do { \
	bzero(&imp,sizeof(_imp_pllatform)); \
	} while(0)

#define __imp_free_platform(imp) do {} while(0)

#define __resolve_platformid(p,m) ((p)->imp.m)



/* kernel */

struct _imp_kernel {
	void* v_kbin;
	void* v_ksym;
	void* v_kcall;
	cl_uint knum;

	cl_uint* arg_kind;
	size_t* arg_sz;
	void** arg_vec;

	size_t arg_buf_sz;
	void* arg_buf;
};

#define __imp_init_kernel(imp) do { \
	imp.v_kbin = 0; \
	imp.v_ksym = 0; \
	imp.v_kcall = 0; \
	imp.knum = -1; \
	imp.arg_kind = 0; \
	imp.arg_sz = 0; \
	imp.arg_vec = 0; \
	imp.arg_buf_sz = 0; \
	imp.arg_buf = 0; \
	} while(0)

#define __imp_free_kernel(imp) do { \
	__free(imp.arg_kind); \
	__free(imp.arg_sz); \
	__free(imp.arg_vec); \
	__free(imp.arg_buf); \
	} while(0)



/* memobj */

struct _imp_mem {
	int devnum;
	void** res;
	void** resmap;
	unsigned int host_cached;
	void* cache_ptr;
};

#define __imp_init_memobj(imp) do { \
	imp.res = 0; \
	imp.resmap = 0; \
	} while(0)

#define __imp_free_memobj(imp) do { \
	__free(imp.res); \
	__free(imp.resmap); \
	} while(0)



/* event */

struct _imp_event {
	pthread_mutex_t mtx;
	pthread_cond_t sig;
	struct cmdcall_arg* cmd_argp;
	TAILQ_ENTRY(_cl_event) cmds;
};

#if defined(__FreeBSD__)
#define PTHREAD_MUTEX_ERRORCHECK_NP PTHREAD_MUTEX_ERRORCHECK
#endif

#define __imp_init_event(imp) do { \
	pthread_mutexattr_t attr; \
	int attrtype = PTHREAD_MUTEX_ERRORCHECK_NP; \
	pthread_mutexattr_init(&attr); \
	pthread_mutexattr_settype(&attr,attrtype); \
	pthread_mutex_init(&imp.mtx,&attr); \
	pthread_cond_init(&imp.sig,0); \
	pthread_mutex_unlock(&imp.mtx); \
	imp.cmd_argp = 0; \
	} while(0)

#define __imp_free_event(imp) do { \
	pthread_cond_destroy(&imp.sig); \
	pthread_mutex_destroy(&imp.mtx); \
	__free(imp.cmd_argp); \
	} while(0)



/* command_queue */

struct _imp_command_queue {
	pthread_t td;
	pthread_mutex_t mtx;
	pthread_cond_t sig;
	unsigned int qstat;
	struct _cl_event* cmd_submitted;
	struct _cl_event* cmd_running;
	TAILQ_HEAD(tailhead_cmds_queued,_cl_event) cmds_queued;
	TAILQ_HEAD(tailhead_cmds_complete,_cl_event) cmds_complete;
};

#define __imp_init_command_queue(imp) do { \
	pthread_mutex_init(&imp.mtx,0); \
	pthread_cond_init(&imp.sig,0); \
	imp.qstat = 0; \
	imp.cmd_submitted = (struct _cl_event*)0; \
	imp.cmd_running = (struct _cl_event*)0; \
	TAILQ_INIT(&imp.cmds_queued); \
	TAILQ_INIT(&imp.cmds_complete); \
	} while(0)

#define __imp_free_command_queue(imp) do { \
	pthread_cond_destroy(&imp.sig); \
	pthread_mutex_destroy(&imp.mtx); \
	} while(0)



/* program */

struct _imp_program {

	cl_uint nclsym;
	struct clsymtab_entry* clsymtab;
	struct clargtab_entry* clargtab;
	char* clstrtab;

	cl_uint nkrn;
	char** kname;
	cl_uint* knarg;
	size_t* karg_buf_sz;
	cl_uint** karg_kind;
	size_t** karg_sz;

	void** v_kbin;
	void*** v_ksym; /* v_kbin, v_ksym might not be useful, not sure */ 
	void*** v_kcall; 

};

#define __imp_init_program(imp) do { \
	imp.nclsym = 0; \
	imp.clsymtab = 0; \
	imp.clstrtab = 0; \
	imp.nkrn = 0; \
	imp.kname = 0; \
	imp.knarg = 0; \
	imp.karg_kind = 0; \
	imp.karg_buf_sz = 0; \
	imp.karg_sz = 0; \
	imp.v_kbin = 0; \
	imp.v_ksym = 0; \
	} while(0)

/* XXX does not yet deal with actual kbin and ksym -DAR */
#define __imp_free_program(imp) do { \
	int k; \
	__free(imp.clsymtab); \
	__free(imp.clstrtab); \
	__free(imp.kname); \
	__free(imp.knarg); \
	__free(imp.karg_buf_sz); \
	for(k=0;k<imp.nkrn;k++) {__free(imp.karg_kind[k]); __free(imp.karg_sz[k]);} \
	__free(imp.karg_kind); \
	__free(imp.karg_sz); \
	} while(0)

#define __nkernels_in_program(prg) (prg->imp.nkrn)


/* context */

struct _imp_context {

#ifdef ENABLE_ATIGPU
	CALcontext* calctx;
#else
	int dummy;
#endif

};

#ifdef ENABLE_ATIGPU
#define __imp_init_context(imp) do { \
	imp.calctx = 0; \
	} while(0)
#else
#define __imp_init_context(imp) do { } while(0)
#endif

#ifdef ENABLE_ATIGPU
#define __imp_free_context(imp) do { \
	__free(imp.calctx); \
	} while(0)
#else
#define __imp_free_context(imp) do { } while(0)
#endif

struct _elf_data {
	char filename[256];
	void* dlh;
	void* map;
};

#define __init_elf_data(ed) do { \
   (ed).filename[0] = '\0'; \
   (ed).dlh = 0; \
   (ed).map = 0; \
   } while(0)



#endif

