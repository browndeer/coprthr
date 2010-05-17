#!/usr/bin/perl

## gen_interceptor.pl
##
## Copyright (c) 2009 Brown Deer Technology, LLC.  All Rights Reserved.
##
## This software was developed by Brown Deer Technology, LLC.
## For more information contact info@browndeertechnology.com
##
## This program is free software: you can redistribute it and/or modify it
## under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
## as published by the Free Software Foundation.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

## DAR ##

@RCODE = ("p", "d");
@RTYPE = ("void*", "cl_int");

@CODE = ("p", "d", "e");
@PCODE = ("p", "d", "p");

### 0 operand
for($r = 0; $r <= $#RCODE; ++$r) {
$rc = $RCODE[$r];
printf "#define _".$rc."(level,lib,rt,fname) \\\n";
printf "rt fname( void ) \\\n";
printf "{ typedef rt (*pfunc_t)(void); \\\n";
printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
printf "rt retval = pf(); \\\n";
printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";
printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
printf "printf(\": \"#fname\"() = %%d\",retval); \\\n";
printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
}

### 1 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$pc0 = $PCODE[$a0];
printf "#define _".$rc."_".$c0."(level,lib,rt,fname,t0) \\\n";
printf "rt fname( t0 a0 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c0 =~ /e/) { printf "cl_int err; rt retval = pf(&err); if (a0) *a0=err; \\\n"; }
else { printf "rt retval = pf(a0); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";


printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c0 =~ /e/) { printf "printf(\": \"#fname\"(%%d) = %%d\",err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0) = %%d\",a0,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c0 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} }

### 2 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE; ++$a1) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
printf "#define _".$rc."_".$c0.$c1."(level,lib,rt,fname,t0,t1) \\\n";
printf "rt fname( t0 a0, t1 a1 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c1 =~ /e/) { printf "cl_int err; rt retval = pf(a0,&err); if (a1) *a1=err; \\\n"; }
else { printf "rt retval = pf(a0,a1); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c1 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%d) = %%d\",a0,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1) = %%d\",a0,a1,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c1 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } }

### 3 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE; ++$a2) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
printf "#define _".$rc."_".$c0.$c1.$c2."(level,lib,rt,fname,t0,t1,t2) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c2 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,&err); if (a2) *a2=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2); \\\n"; }
printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c2 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%d) = %%d\",a0,a1,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2) = %%d\",a0,a1,a2,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c2 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } }

### 4 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE-1; ++$a2) {
for($a3 = 0; $a3 <= $#CODE; ++$a3) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$c3 = $CODE[$a3];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
$pc3 = $PCODE[$a3];
printf "#define _".$rc."_".$c0.$c1.$c2.$c3."(level,lib,rt,fname,t0,t1,t2,t3) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2, t3 a3 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2,t3); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";

if ($c3 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,a2,&err); if (a3) *a3=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2,a3); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";

if ($c3 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%d) = %%d\",a0,a1,a2,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3) = %%d\",a0,a1,a2,a3,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";

if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c3 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }

printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } } }



### 5 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE-1; ++$a2) {
for($a3 = 0; $a3 <= $#CODE-1; ++$a3) {
for($a4 = 0; $a4 <= $#CODE; ++$a4) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$c3 = $CODE[$a3];
$c4 = $CODE[$a4];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
$pc3 = $PCODE[$a3];
$pc4 = $PCODE[$a4];
printf "#define _".$rc."_".$c0.$c1.$c2.$c3.$c4."(level,lib,rt,fname,t0,t1,t2,t3,t4) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2, t3 a3, t4 a4 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2,t3,t4); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c4 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,a2,a3,&err); if (a4) *a4=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2,a3,a4); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c4 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%d) = %%d\",a0,a1,a2,a3,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4) = %%d\",a0,a1,a2,a3,a4,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c4 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } } } }

### 6 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE-1; ++$a2) {
for($a3 = 0; $a3 <= $#CODE-1; ++$a3) {
for($a4 = 0; $a4 <= $#CODE-1; ++$a4) {
for($a5 = 0; $a5 <= $#CODE; ++$a5) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$c3 = $CODE[$a3];
$c4 = $CODE[$a4];
$c5 = $CODE[$a5];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
$pc3 = $PCODE[$a3];
$pc4 = $PCODE[$a4];
$pc5 = $PCODE[$a5];
printf "#define _".$rc."_".$c0.$c1.$c2.$c3.$c4.$c5."(level,lib,rt,fname,t0,t1,t2,t3,t4,t5) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2,t3,t4,t5); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c5 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,a2,a3,a4,&err); if (a5) *a5=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2,a3,a4,a5); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c5 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%d) = %%d\",a0,a1,a2,a3,a4,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5) = %%d\",a0,a1,a2,a3,a4,a5,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c5 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } } } } }

### 7 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE-1; ++$a2) {
for($a3 = 0; $a3 <= $#CODE-1; ++$a3) {
for($a4 = 0; $a4 <= $#CODE-1; ++$a4) {
for($a5 = 0; $a5 <= $#CODE-1; ++$a5) {
for($a6 = 0; $a6 <= $#CODE; ++$a6) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$c3 = $CODE[$a3];
$c4 = $CODE[$a4];
$c5 = $CODE[$a5];
$c6 = $CODE[$a6];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
$pc3 = $PCODE[$a3];
$pc4 = $PCODE[$a4];
$pc5 = $PCODE[$a5];
$pc6 = $PCODE[$a6];
printf "#define _".$rc."_".$c0.$c1.$c2.$c3.$c4.$c5.$c6."(level,lib,rt,fname,t0,t1,t2,t3,t4,t5,t6) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2,t3,t4,t5,t6); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c6 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,a2,a3,a4,a5,&err); if (a6) *a6=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2,a3,a4,a5,a6); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c6 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%d) = %%d\",a0,a1,a2,a3,a4,a5,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6) = %%d\",a0,a1,a2,a3,a4,a5,a6,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c6 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } } } } } }

### 8 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE-1; ++$a2) {
for($a3 = 0; $a3 <= $#CODE-1; ++$a3) {
for($a4 = 0; $a4 <= $#CODE-1; ++$a4) {
for($a5 = 0; $a5 <= $#CODE-1; ++$a5) {
for($a6 = 0; $a6 <= $#CODE-1; ++$a6) {
for($a7 = 0; $a7 <= $#CODE; ++$a7) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$c3 = $CODE[$a3];
$c4 = $CODE[$a4];
$c5 = $CODE[$a5];
$c6 = $CODE[$a6];
$c7 = $CODE[$a7];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
$pc3 = $PCODE[$a3];
$pc4 = $PCODE[$a4];
$pc5 = $PCODE[$a5];
$pc6 = $PCODE[$a6];
$pc7 = $PCODE[$a7];
printf "#define _".$rc."_".$c0.$c1.$c2.$c3.$c4.$c5.$c6.$c7."(level,lib,rt,fname,t0,t1,t2,t3,t4,t5,t6,t7) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2,t3,t4,t5,t6,t7); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c7 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,a2,a3,a4,a5,a6,&err); if (a7) *a7=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2,a3,a4,a5,a6,a7); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c7 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%d) = %%d\",a0,a1,a2,a3,a4,a5,a6,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%$pc7) = %%d\",a0,a1,a2,a3,a4,a5,a6,a7,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c7 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } } } } } } }

### 9 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE-1; ++$a2) {
for($a3 = 0; $a3 <= $#CODE-1; ++$a3) {
for($a4 = 0; $a4 <= $#CODE-1; ++$a4) {
for($a5 = 0; $a5 <= $#CODE-1; ++$a5) {
for($a6 = 0; $a6 <= $#CODE-1; ++$a6) {
for($a7 = 0; $a7 <= $#CODE-1; ++$a7) {
for($a8 = 0; $a8 <= $#CODE; ++$a8) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$c3 = $CODE[$a3];
$c4 = $CODE[$a4];
$c5 = $CODE[$a5];
$c6 = $CODE[$a6];
$c7 = $CODE[$a7];
$c8 = $CODE[$a8];
$pc8 = $PCODE[$a8];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
$pc3 = $PCODE[$a3];
$pc4 = $PCODE[$a4];
$pc5 = $PCODE[$a5];
$pc6 = $PCODE[$a6];
$pc7 = $PCODE[$a7];
$pc8 = $PCODE[$a8];
printf "#define _".$rc."_".$c0.$c1.$c2.$c3.$c4.$c5.$c6.$c7.$c8."(level,lib,rt,fname,t0,t1,t2,t3,t4,t5,t6,t7,t8) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7, t8 a8 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2,t3,t4,t5,t6,t7,t8); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c8 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,a2,a3,a4,a5,a6,a7,&err); if (a8) *a8=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2,a3,a4,a5,a6,a7,a8); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c8 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%$pc7,%%d) = %%d\",a0,a1,a2,a3,a4,a5,a6,a7,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%$pc7,%%$pc8) = %%d\",a0,a1,a2,a3,a4,a5,a6,a7,a8,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c8 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } } } } } } } }

### 10 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE-1; ++$a2) {
for($a3 = 0; $a3 <= $#CODE-1; ++$a3) {
for($a4 = 0; $a4 <= $#CODE-1; ++$a4) {
for($a5 = 0; $a5 <= $#CODE-1; ++$a5) {
for($a6 = 0; $a6 <= $#CODE-1; ++$a6) {
for($a7 = 0; $a7 <= $#CODE-1; ++$a7) {
for($a8 = 0; $a8 <= $#CODE-1; ++$a8) {
for($a9 = 0; $a9 <= $#CODE; ++$a9) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$c3 = $CODE[$a3];
$c4 = $CODE[$a4];
$c5 = $CODE[$a5];
$c6 = $CODE[$a6];
$c7 = $CODE[$a7];
$c8 = $CODE[$a8];
$c9 = $CODE[$a9];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
$pc3 = $PCODE[$a3];
$pc4 = $PCODE[$a4];
$pc5 = $PCODE[$a5];
$pc6 = $PCODE[$a6];
$pc7 = $PCODE[$a7];
$pc8 = $PCODE[$a8];
$pc9 = $PCODE[$a9];
printf "#define _".$rc."_".$c0.$c1.$c2.$c3.$c4.$c5.$c6.$c7.$c8.$c9."(level,lib,rt,fname,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7, t8 a8, t9 a9 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2,t3,t4,t5,t6,t7,t8,t9); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c9 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,a2,a3,a4,a5,a6,a7,a8,&err); if (a9) *a9=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c9 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%$pc7,%%$pc8,%%d) = %%d\",a0,a1,a2,a3,a4,a5,a6,a7,a8,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%$pc7,%%$pc8,%%$pc9) = %%d\",a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c9 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } } } } } } } } }

### 11 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE-1; ++$a2) {
for($a3 = 0; $a3 <= $#CODE-1; ++$a3) {
for($a4 = 0; $a4 <= $#CODE-1; ++$a4) {
for($a5 = 0; $a5 <= $#CODE-1; ++$a5) {
for($a6 = 0; $a6 <= $#CODE-1; ++$a6) {
for($a7 = 0; $a7 <= $#CODE-1; ++$a7) {
for($a8 = 0; $a8 <= $#CODE-1; ++$a8) {
for($a9 = 0; $a9 <= $#CODE; ++$a9) {
for($a10 = 0; $a10 < 2; ++$a10) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$c3 = $CODE[$a3];
$c4 = $CODE[$a4];
$c5 = $CODE[$a5];
$c6 = $CODE[$a6];
$c7 = $CODE[$a7];
$c8 = $CODE[$a8];
$c9 = $CODE[$a9];
$c10 = $CODE[$a10];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
$pc3 = $PCODE[$a3];
$pc4 = $PCODE[$a4];
$pc5 = $PCODE[$a5];
$pc6 = $PCODE[$a6];
$pc7 = $PCODE[$a7];
$pc8 = $PCODE[$a8];
$pc9 = $PCODE[$a9];
$pc10 = $PCODE[$a10];
printf "#define _".$rc."_".$c0.$c1.$c2.$c3.$c4.$c5.$c6.$c7.$c8.$c9.$c10."(level,lib,rt,fname,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7, t8 a8, t9 a9, t10 a10 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c10 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,&err); if (a10) *a10=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c10 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%$pc7,%%$pc8,%%$pc9,%%d) = %%d\",a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%$pc7,%%$pc8,%%$pc9,%%$pc10) = %%d\",a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c10 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } } } } } } } } } }

### 12 operand
for($r = 0; $r <= $#RCODE; ++$r) {
for($a0 = 0; $a0 <= $#CODE-1; ++$a0) {
for($a1 = 0; $a1 <= $#CODE-1; ++$a1) {
for($a2 = 0; $a2 <= $#CODE-1; ++$a2) {
for($a3 = 0; $a3 <= $#CODE-1; ++$a3) {
for($a4 = 0; $a4 <= $#CODE-1; ++$a4) {
for($a5 = 0; $a5 <= $#CODE-1; ++$a5) {
for($a6 = 0; $a6 <= $#CODE-1; ++$a6) {
for($a7 = 0; $a7 <= $#CODE-1; ++$a7) {
for($a8 = 0; $a8 <= $#CODE-1; ++$a8) {
for($a9 = 0; $a9 <= $#CODE-1; ++$a9) {
for($a10 = 0; $a10 <= $#CODE-1; ++$a10) {
for($a11 = 0; $a11 <= $#CODE; ++$a11) {
$rc = $RCODE[$r];
$c0 = $CODE[$a0];
$c1 = $CODE[$a1];
$c2 = $CODE[$a2];
$c3 = $CODE[$a3];
$c4 = $CODE[$a4];
$c5 = $CODE[$a5];
$c6 = $CODE[$a6];
$c7 = $CODE[$a7];
$c8 = $CODE[$a8];
$c9 = $CODE[$a9];
$c10 = $CODE[$a10];
$c11 = $CODE[$a11];
$pc0 = $PCODE[$a0];
$pc1 = $PCODE[$a1];
$pc2 = $PCODE[$a2];
$pc3 = $PCODE[$a3];
$pc4 = $PCODE[$a4];
$pc5 = $PCODE[$a5];
$pc6 = $PCODE[$a6];
$pc7 = $PCODE[$a7];
$pc8 = $PCODE[$a8];
$pc9 = $PCODE[$a9];
$pc10 = $PCODE[$a10];
$pc11 = $PCODE[$a11];
printf "#define _".$rc."_".$c0.$c1.$c2.$c3.$c4.$c5.$c6.$c7.$c8.$c9.$c10.$c11."(level,lib,rt,fname,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11) \\\n";
printf "rt fname( t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7, t8 a8, t9 a9, t10 a10, t11 a11 ) \\\n";
printf "{ typedef rt (*pfunc_t)(t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11); \\\n";

printf "if (!h) h =  dlopen( #lib , RTLD_LAZY); \\\n";
printf "if (!h) { fprintf(stderr,\"libcltrace\" #level \": open \" #lib \" failed\\n\"); exit(-1); } \\\n";
printf "pfunc_t pf = (pfunc_t)dlsym(h,#fname); \\\n";
printf "if (!pf) { fprintf(stderr,\"libcltrace\" #level \": get symbol \" #fname \" failed\\n\"); exit(-1); } \\\n";
printf "struct timeval time0,time1; gettimeofday(&time0,0); \\\n";
if ($c11 =~ /e/) { printf "cl_int err; rt retval = pf(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,&err); if (a11) *a11=err; \\\n"; }
else { printf "rt retval = pf(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11); \\\n"; }

printf "gettimeofday(&time1,0); \\\n";
printf "if(!cltrace_report){ \\\n";
printf "printf(\"cltrace\" #level ); \\\n";

printf "if (cltrace_timestamp) printf(\"(%%d:%%d)\",time0.tv_sec,time0.tv_usec); \\\n";
if ($c11 =~ /e/) { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%$pc7,%%$pc8,%%$pc9,%%$pc10,%%d) = %%d\",a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,err,retval); \\\n"; }
else { printf "printf(\": \"#fname\"(%%$pc0,%%$pc1,%%$pc2,%%$pc3,%%$pc4,%%$pc5,%%$pc6,%%$pc7,%%$pc8,%%$pc9,%%$pc10,%%$pc11) = %%d\",a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,retval); \\\n"; }

printf "if (cltrace_timer) printf(\" <%%f>\",1.0*(time1.tv_sec-time0.tv_sec) + 1.0e-6*(time1.tv_usec-time0.tv_usec)); \\\n";
printf "printf(\"\\n\"); fflush(stdout); \\\n";
printf "} \\\n";
printf "++calltab[CALLID_##fname].nc; \\\n";
if ($rc =~ /d/) { printf "if (retval) ++calltab[CALLID_##fname].nerr; \\\n"; }
elsif ($c11 =~ /e/) { printf "if (err) ++calltab[CALLID_##fname].nerr; \\\n"; }
printf "calltab[CALLID_##fname].tusec += 1000000*(time1.tv_sec-time0.tv_sec)+(time1.tv_usec-time0.tv_usec); \\\n";
printf "return(retval); \\\n";
printf "}\n\n";
} } } } } } } } } } } } }

