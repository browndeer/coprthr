/* libcltrace2.c
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


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define __USE_GNU
#include <dlfcn.h>

#include <CL/cl.h>
#include "stdcl.h"

#define CALLID_clcontext_create			1
#define CALLID_clcontext_destroy			2
#define CALLID_clgetdevinfo				3
#define CALLID_clfreport_devinfo			4	
#define CALLID_clstat						5
#define CALLID_clgetndev					6	
#define CALLID_clload						7
#define CALLID_clopen						8
#define CALLID_clclose						9
#define CALLID_clsym							10	
#define CALLID_clerror						11	
#define CALLID_clmalloc						12	
#define CALLID_clfree						13	
#define CALLID_clmsync						14		
#define CALLID_clfork						15	
#define CALLID_clwait						16	
#define CALLID_clwaitev						17	

struct calltab_entry { 
	int id, nc, nerr; 
	unsigned long long tusec; 
	char cname[64];
}; 

#define __entry(cname) { CALLID_##cname, 0,0,0, #cname }

static struct calltab_entry calltab[] = {
	{ 0,0,0,0,0 }, /* dummy entry */
	__entry(clcontext_create),
	__entry(clcontext_destroy),
	__entry(clgetdevinfo),
	__entry(clfreport_devinfo),
	__entry(clstat),
	__entry(clgetndev),
	__entry(clload),
	__entry(clopen),
	__entry(clclose),
	__entry(clsym),
	__entry(clerror),
	__entry(clmalloc),
	__entry(clfree),
	__entry(clmsync),
	__entry(clfork),
	__entry(clwait),
	__entry(clwaitev),
	{ 0,0,0,0,0 }  /* dummy entry */
};

#define NCALLS (sizeof(calltab)/sizeof(struct calltab_entry))

static char* cltrace_report = 0;
static char* cltrace_timestamp = 0;
static char* cltrace_timer = 0;

static int init_skip = 0;
static int fini_defer = 0;

void __attribute__((__constructor__(101))) _libcltrace2_init(void)
{
	if (init_skip==1) return;

	cltrace_report = getenv("CLTRACE_REPORT");
	cltrace_timestamp = getenv("CLTRACE_TIMESTAMP");
	cltrace_timer = getenv("CLTRACE_TIMER");

	init_skip = 1;
}

void __attribute__((__destructor__(101))) _libcltrace2_fini(void)
{
	if (fini_defer==1) { fini_defer=2; return; }

	int i,j;
	int sort[NCALLS];
	unsigned long long sum = 0;

	for(i=0;i<NCALLS;i++) { sort[i]=i; sum += calltab[i].tusec; }

	for(j=0;j<NCALLS;j++) for(i=0;i<NCALLS-1;i++) {
		int k1 = sort[i]; 
		int k2 = sort[i+1];
		if (calltab[k1].tusec < calltab[k2].tusec) { sort[i]=k2; sort[i+1]=k1; }
	}

	float s = 100.0f/(float)sum;

	int count = 0;
	for(i=0;i<NCALLS;i++) count += calltab[sort[i]].nc;

	if (cltrace_report && count) {

		fflush(stdout);
		printf("\n");
		
		printf("%% time     seconds  usecs/call     calls    errors stdcl_call\n");
		printf("------ ----------- ----------- --------- --------- ----------------------------\n");

		for(i=0;i<NCALLS;i++) {
			struct calltab_entry e = calltab[sort[i]];
			unsigned long long t = e.tusec;
			if (e.nc>0) {
				printf("%6.2f  %10.6f %11d %9d",
					(float)t*s,1.0e-6*(float)t,t/e.nc,e.nc);
				if (e.nerr>0) printf(" %9d",e.nerr); else printf("          ");
				printf(" %s\n",e.cname);
			}
		
		}	

		printf("------ ----------- ----------- --------- --------- ----------------------------\n");

	}

}

#include "_interceptor.h"

static void* h = 0;

	__entry(clmalloc),
	__entry(clfree),
	__entry(clmsync),
	__entry(clfork),
	__entry(clwait),
	__entry(clwaitev),

_p_pddd( 2,libstdcl.so, CONTEXT*, 
	clcontext_create, 
	cl_platform_id , int , size_t , int )

_d_p( 2,libstdcl.so, int, 
	clcontext_destroy, 
	CONTEXT* )

_d_pp( 2,libstdcl.so, int,
	 clgetdevinfo, CONTEXT*, struct cldev_info* )

//_v_pdp( 2, libstdcl.so, void,
//	clfreport_devinfo,
//	FILE*, size_t, struct cldev_info* )

_d_pp( 2, libstdcl.so, int,
	 clstat,
	 CONTEXT* cp, struct clstat_info* )

_d_p( 2, libstdcl.so, cl_uint,
	clgetndev, CONTEXT* )

_p_ppdd( 2, libstdcl.so, void*,
	clload, CONTEXT* , void* , size_t , int )

_p_ppd( 2, libstdcl.so, void*,
	clopen, CONTEXT* , const char* , int )

_d_pp( 2, libstdcl.so, int,
	clclose, CONTEXT* , void* )

_p_pppd( 2, libstdcl.so, cl_kernel,
	clsym( CONTEXT* , void* , const char* , int )

//_p_v( 2, libstdcl.so, char* 
//	clerror, void )


_p_pdd( 2, libstdcl.so, void*,
	clmalloc, CONTEXT* , size_t , int  )

_v_p( 2, libstdcl.so, void,
	clfree, void* )

_p_pdpd( 2, libstdcl.so, cl_event,
	clmsync, CONTEXT* , unsigned int, void*, int )

_p_pdppd( 2, libstdcl.so, cl_event ,
	clfork, CONTEXT* , cl_uint , cl_kernel , struct clndrange_struct*, int )

_p_pdd( 2, libstdcl.so, cl_event,
	clwait, CONTEXT* , unsigned int , int )

_p_pdpd( 2, libstdclo.so, cl_event,
	 clwaitev, CONTEXT*, unsigned int , const cl_event , int )


/* XXX routines below used to make sure all libstdcl us of of opencl calls
 * were properly intercepted.  how to treat this issue here? 
 */


void _libstdcl_init(void) { 
	typedef void (*pfunc_t)(void); 
	if (!h) h = dlopen("libstdcl.so", RTLD_LAZY); 
	if (!h) { fprintf(stderr,"libcltrace1: open libstdcl failed\n"); exit(-1); } 
	pfunc_t pf = (pfunc_t)dlsym(h,"_libstdcl_init"); 
	if (!pf) { 
		fprintf(stderr,"libcltrace1: get symbol " "_libstdcl_init" " failed\n"); 
		exit(-1); 
	} 

		fini_defer = 1;
		_libcltrace1_init();
		pf(); 
	fflush(stdout); 
	return; 
}


void _libstdcl_fini(void) { 
	typedef void (*pfunc_t)(void); 
	if (!h) h = dlopen("libstdcl.so", RTLD_LAZY); 
	if (!h) { fprintf(stderr,"libcltrace1: open libstdcl failed\n"); exit(-1); } 
	pfunc_t pf = (pfunc_t)dlsym(h,"_libstdcl_fini"); 
	if (!pf) { 
		fprintf(stderr,"libcltrace1: get symbol " "_libstdcl_fini" " failed\n"); 
		exit(-1); 
	} 

		pf(); 

		if (fini_defer==2) { fini_defer=0; _libcltrace1_fini(); }
		else fini_defer = 0;

	fflush(stdout); 
	return; 
}


