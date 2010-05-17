/* libcltrace0_calrt.c
 *
 * Copyright (c) 2009 Brown Deer Technology, LLC.  All Rights Reserved.
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

#include "cal.h"
//#include "calcl.h"

#define CALLID_calInit              1
#define CALLID_calGetVersion              2
#define CALLID_calShutdown              3
#define CALLID_calDeviceGetCount              4
#define CALLID_calDeviceGetInfo              5
#define CALLID_calDeviceGetAttribs              6
#define CALLID_calDeviceGetStatus              7
#define CALLID_calDeviceOpen              8
#define CALLID_calDeviceClose              9
#define CALLID_calResAllocLocal2D          10
#define CALLID_calResAllocRemote2D              11
#define CALLID_calResAllocLocal1D              12
#define CALLID_calResAllocRemote1D              13
#define CALLID_calResFree              14
#define CALLID_calResMap              15
#define CALLID_calResUnmap              16
#define CALLID_calCtxCreate              17
#define CALLID_calCtxDestroy              18
#define CALLID_calCtxGetMem              19
#define CALLID_calCtxReleaseMem              20
#define CALLID_calCtxSetMem              21
#define CALLID_calCtxRunProgram              22
#define CALLID_calCtxIsEventDone              23
#define CALLID_calCtxFlush              24
#define CALLID_calMemCopy              25
#define CALLID_calImageRead              26
#define CALLID_calImageFree              27
#define CALLID_calModuleLoad              28
#define CALLID_calModuleUnload              29
#define CALLID_calModuleGetEntry              30
#define CALLID_calModuleGetName              31
#define CALLID_calGetErrorString              32
#define CALLID_calCtxRunProgramGrid              33
#define CALLID_calModuleGetFuncInfo              34
#define CALLID_calCtxRunProgramGridArray              35
//#define CALLID_calclGetVersion              36
//#define CALLID_calclCompile              37
//#define CALLID_calclLink              38
//#define CALLID_calclFreeObject              39
//#define CALLID_calclFreeImage              40
//#define CALLID_calclDisassembleImage              41
//#define CALLID_calclAssembleObject              42
//#define CALLID_calclDisassembleObject              43
//#define CALLID_calclImageGetSize              44
//#define CALLID_calclImageWrite              45
//#define CALLID_calclGetErrorString             46

struct calltab_entry {
   int id, nc, nerr;
   unsigned long long tusec;
   char cname[64];
};

#define 	__entry(cname) { CALLID_##cname, 0,0,0, #cname }

static struct calltab_entry calltab[] = {
   { 0,0,0,0,0 }, /* dummy entry */
	__entry(calInit),
	__entry(calGetVersion),
	__entry(calShutdown),
	__entry(calDeviceGetCount),
	__entry(calDeviceGetInfo),
	__entry(calDeviceGetAttribs),
	__entry(calDeviceGetStatus),
	__entry(calDeviceOpen),
	__entry(calDeviceClose),
	__entry(calResAllocLocal2D),
	__entry(calResAllocRemote2D),
	__entry(calResAllocLocal1D),
	__entry(calResAllocRemote1D),
	__entry(calResFree),
	__entry(calResMap),
	__entry(calResUnmap),
	__entry(calCtxCreate),
	__entry(calCtxDestroy),
	__entry(calCtxGetMem),
	__entry(calCtxReleaseMem),
	__entry(calCtxSetMem),
	__entry(calCtxRunProgram),
	__entry(calCtxIsEventDone),
	__entry(calCtxFlush),
	__entry(calMemCopy),
	__entry(calImageRead),
	__entry(calImageFree),
	__entry(calModuleLoad),
	__entry(calModuleUnload),
	__entry(calModuleGetEntry),
	__entry(calModuleGetName),
	__entry(calGetErrorString),
	__entry(calCtxRunProgramGrid),
	__entry(calModuleGetFuncInfo),
	__entry(calCtxRunProgramGridArray),
//	__entry(calclGetVersion),
//	__entry(calclCompile),
//	__entry(calclLink),
//	__entry(calclFreeObject),
//	__entry(calclFreeImage),
//	__entry(calclDisassembleImage),
//	__entry(calclAssembleObject),
//	__entry(calclDisassembleObject),
//	__entry(calclImageGetSize),
//	__entry(calclImageWrite),
//	__entry(calclGetErrorString),
   { 0,0,0,0,0 }  /* dummy entry */
};

#define NCALLS (sizeof(calltab)/sizeof(struct calltab_entry))

static char* cltrace_report = 0;
static char* cltrace_timestamp = 0;
static char* cltrace_timer = 0;

static int init_skip = 0;
static int fini_defer = 0;

void __attribute__((__constructor__(101))) _libcltrace0_calrt_init(void)
{
//   if (init_skip==1) return;

   cltrace_report = getenv("CLTRACE_REPORT");
   cltrace_timestamp = getenv("CLTRACE_TIMESTAMP");
   cltrace_timer = getenv("CLTRACE_TIMER");

//   init_skip = 1;
}


void __attribute__((__destructor__(101))) _libcltrace0_calrt_fini(void)
{
//   if (fini_defer==1) { fini_defer=2; return; }

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

      printf("%% time     seconds  usecs/call     calls    errors calrt_call\n");
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


/* intercept cal.h */

 _d(0,libaticalrt.so,CALresult,calInit)


 _d_ppp(0,libaticalrt.so,  CALresult, calGetVersion,
		CALuint*, CALuint*, CALuint*)

 _d(0,libaticalrt.so,  CALresult, calShutdown)


 _d_p(0,libaticalrt.so,  CALresult,calDeviceGetCount,
	CALuint*)

 _d_pd(0,libaticalrt.so,  CALresult,calDeviceGetInfo,
	CALdeviceinfo*, CALuint)

 _d_pd(0,libaticalrt.so,  CALresult,calDeviceGetAttribs,
	CALdeviceattribs*, CALuint)


 _d_pp(0,libaticalrt.so,  CALresult,calDeviceGetStatus,
	CALdevicestatus*, CALdevice)

 _d_pd(0,libaticalrt.so,  CALresult, calDeviceOpen,
	CALdevice*, CALuint)

 _d_p(0,libaticalrt.so,  CALresult, calDeviceClose,
	CALdevice)

 _d_ppddpd(0,libaticalrt.so,  CALresult,calResAllocLocal2D,
	CALresource*, CALdevice, CALuint, CALuint, 
	CALformat, CALuint)

 _d_ppdddpd(0,libaticalrt.so,  CALresult,calResAllocRemote2D,
	CALresource*, CALdevice*, CALuint, CALuint, 
	CALuint, CALformat, CALuint)

 _d_ppdpd(0,libaticalrt.so,  CALresult,calResAllocLocal1D,
	CALresource*, CALdevice, CALuint, CALformat, 
	CALuint)

 _d_ppddpd(0,libaticalrt.so,  CALresult,calResAllocRemote1D,
	CALresource*, CALdevice*, CALuint, CALuint, 
	CALformat, CALuint)

 _d_p(0,libaticalrt.so,  CALresult,calResFree,
	CALresource)

 _d_pppd(0,libaticalrt.so,  CALresult,calResMap,
	CALvoid**, CALuint*, CALresource, CALuint)

 _d_p(0,libaticalrt.so,  CALresult,calResUnmap,
	CALresource)


 _d_pp(0,libaticalrt.so,  CALresult,calCtxCreate,
	CALcontext*, CALdevice)

 _d_p(0,libaticalrt.so,  CALresult,calCtxDestroy,
	CALcontext)

 _d_ppp(0,libaticalrt.so,  CALresult,calCtxGetMem,
	CALmem*, CALcontext, CALresource)

 _d_pp(0,libaticalrt.so,  CALresult,calCtxReleaseMem,
	CALcontext, CALmem)

 _d_ppp(0,libaticalrt.so,  CALresult,calCtxSetMem,
	CALcontext, CALname, CALmem)

 _d_pppp(0,libaticalrt.so,  CALresult,calCtxRunProgram,
	CALevent*, CALcontext, CALfunc, const CALdomain*)

 _d_pp(0,libaticalrt.so,  CALresult,calCtxIsEventDone,
	CALcontext, CALevent)

 _d_p(0,libaticalrt.so,  CALresult,calCtxFlush,
	CALcontext)

 _d_ppppd(0,libaticalrt.so,  CALresult,calMemCopy,
	CALevent*, CALcontext, CALmem, CALmem, CALuint)

 _d_ppd(0,libaticalrt.so,  CALresult,calImageRead,
	CALimage*, const CALvoid*, CALuint)

 _d_p(0,libaticalrt.so,  CALresult,calImageFree,
	CALimage)

 _d_ppp(0,libaticalrt.so,  CALresult,calModuleLoad,
	CALmodule*, CALcontext, CALimage)

 _d_pp(0,libaticalrt.so,  CALresult,calModuleUnload,
	CALcontext, CALmodule)

// _d_ppps(0,libaticalrt.so,  CALresult,calModuleGetEntry,
 _d_pppp(0,libaticalrt.so,  CALresult,calModuleGetEntry,
	CALfunc*, CALcontext, CALmodule, const CALchar*)

// _d_ppps(0,libaticalrt.so,  CALresult,calModuleGetName,
 _d_pppp(0,libaticalrt.so,  CALresult,calModuleGetName,
	CALname*, CALcontext, CALmodule, const CALchar*)

 _p(0,libaticalrt.so,  const CALchar*,calGetErrorString)

 _d_ppp(0,libaticalrt.so,  CALresult,calCtxRunProgramGrid,
	CALevent*, CALcontext, CALprogramGrid*)

 _d_pppp(0,libaticalrt.so,  CALresult,calModuleGetFuncInfo,
	CALfuncInfo*, CALcontext, CALmodule, CALfunc)

 _d_ppp(0,libaticalrt.so,  CALresult,calCtxRunProgramGridArray,
	CALevent*, CALcontext, CALprogramGridArray*)



/* calcl.h */

/*
_d_ppp(0,libaticalcl.so, CALresult,calclGetVersion,
	CALuint*, CALuint*, CALuint*)

//_d_ppsp(0,libaticalrt.so, CALresult,calclCompile,
_d_pppp(0,libaticalcl.so, CALresult,calclCompile,
	CALobject*, CALlanguage, const CALchar*, 
	CALtarget)

_d_ppd(0,libaticalcl.so, CALresult,calclLink,
	CALimage*, CALobject*, CALuint)

_d_p(0,libaticalcl.so, CALresult,calclFreeObject,
	CALobject)

_d_p(0,libaticalcl.so, CALresult,calclFreeImage,
	CALimage)

//_v_pp(0,libaticalcl.so,void,calclDisassembleImage,
//	const CALimage, CALLogFunction)

//_d_ppsp(0,libaticalcl.so, CALresult,calclAssembleObject,
_d_pppp(0,libaticalcl.so, CALresult,calclAssembleObject,
	CALobject*, CALCLprogramType, const CALchar*, 
	CALtarget)

//_v_pp(0,libaticalcl.so,void,calclDisassembleObject,
//	const CALobject*, CALLogFunction)

_d_pp(0,libaticalcl.so, CALresult,calclImageGetSize,
	CALuint*, CALimage)

_d_pdp(0,libaticalcl.so, CALresult,calclImageWrite,
	CALvoid*, CALuint, CALimage)

_p(0,libaticalcl.so, const CALchar*, calclGetErrorString)
*/


