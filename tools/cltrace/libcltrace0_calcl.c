/* libcltrace0_calcl.c
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
#include "calcl.h"

#define CALLID_calclGetVersion              1
#define CALLID_calclCompile              2
#define CALLID_calclLink              3
#define CALLID_calclFreeObject              4
#define CALLID_calclFreeImage              5
#define CALLID_calclDisassembleImage              6
#define CALLID_calclAssembleObject              7
#define CALLID_calclDisassembleObject              8
#define CALLID_calclImageGetSize              9
#define CALLID_calclImageWrite              10
#define CALLID_calclGetErrorString             11

struct calltab_entry {
   int id, nc, nerr;
   unsigned long long tusec;
   char cname[64];
};

#define 	__entry(cname) { CALLID_##cname, 0,0,0, #cname }

static struct calltab_entry calltab[] = {
   { 0,0,0,0,0 }, /* dummy entry */
	__entry(calclGetVersion),
	__entry(calclCompile),
	__entry(calclLink),
	__entry(calclFreeObject),
	__entry(calclFreeImage),
	__entry(calclDisassembleImage),
	__entry(calclAssembleObject),
	__entry(calclDisassembleObject),
	__entry(calclImageGetSize),
	__entry(calclImageWrite),
	__entry(calclGetErrorString),
   { 0,0,0,0,0 }  /* dummy entry */
};

#define NCALLS (sizeof(calltab)/sizeof(struct calltab_entry))

static char* cltrace_report = 0;
static char* cltrace_timestamp = 0;
static char* cltrace_timer = 0;

static int init_skip = 0;
static int fini_defer = 0;

void __attribute__((__constructor__(101))) _libcltrace0_calcl_init(void)
{
//   if (init_skip==1) return;

   cltrace_report = getenv("CLTRACE_REPORT");
   cltrace_timestamp = getenv("CLTRACE_TIMESTAMP");
   cltrace_timer = getenv("CLTRACE_TIMER");

//   init_skip = 1;
}


void __attribute__((__destructor__(101))) _libcltrace0_calcl_fini(void)
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

      printf("%% time     seconds  usecs/call     calls    errors calcl_call\n");
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


/* calcl.h */

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


