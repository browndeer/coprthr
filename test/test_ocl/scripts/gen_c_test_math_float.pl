#!/usr/bin/perl
# 
# Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
#
# This software was developed by Brown Deer Technology, LLC.
# For more information contact info@browndeertechnology.com
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# DAR #

@builtins_unary_0 = qw(
	acos acospi asin asinh asinpi atan atanh atanpi 
	cbrt ceil cos cosh cospi
	erfc erf exp exp2 exp10 expm1
	fabs 
	lgamma log log2 log10 log1p logb 
	rint round rsqrt
	sin sinh sinpi sqrt
);

@builtins_unary_1 = qw(
	acosh
	floor
);

@builtins_binary23 = qw(
	atan2 atan2pi
);

@builtins_binary_43 = qw(
	copysign
	fdim fmax fmin fmod
	hypot
	nextafter
	pow powr
	remainder
);

#	hypot
#	nextafter
#	pow powr
#	remainder
#);

$size = 128;
$bsize = 4;
$clfile = 'test_math_float.cl';
$testprefix = 'test_math_';

printf "\n";
printf "#include <stdio.h>\n";
printf "#include <stdlib.h>\n";
printf "#include <math.h>\n";
printf "#include <fcntl.h>\n";
printf "#include <sys/stat.h>\n";
printf "#include <sys/mman.h>\n";
printf "\n";
printf "#include \"CL/cl.h\"\n";
printf "\n";
printf "#define SIZE $size\n";
printf "#define BLOCKSIZE $bsize\n";
printf "\n";
printf "#define __mapfile(file,filesz,pfile) do { \\\n";
printf "int fd = open(file,O_RDONLY); \\\n";
printf "struct stat fst; fstat(fd,&fst); \\\n";
printf "if (fst.st_size == 0 || !S_ISREG(fst.st_mode)) { \\\n";
printf "fprintf(stderr,\"error: open failed on '%s'\\n\",file); \\\n";
printf "pfile=0; \\\n";
printf "} else { \\\n";
printf "filesz = fst.st_size; \\\n";
printf "pfile = mmap(0,filesz,PROT_READ,MAP_PRIVATE,fd,0); \\\n";
printf "} \\\n";
printf "close(fd); \\\n";
printf "} while(0);\n";
printf "\n";
printf "#define __exit(err,line) \\\n";
printf "do { fprintf(\"error code %d\\n\",err); exit(line); } while(0)\n";
printf "\n";


printf "#define acos(x) (acosf(x))\n";
printf "#define acosh(x) (acoshf(x))\n";
printf "#define acospi(x) (M_1_PI*acos(x))\n";
printf "#define asin(x) (asinf(x))\n";
printf "#define asinh(x) (asinhf(x))\n";
printf "#define asinpi(x) (M_1_PI*asin(x))\n";
printf "#define atan(x) (atanf(x))\n";
printf "#define atanh(x) (atanhf(x))\n";
printf "#define atanpi(x) (M_1_PI*atan(x))\n";

printf "#define cbrt(x) (cbrtf(x))\n";
printf "#define ceil(x) (ceilf(x))\n";
printf "#define cos(x) (cosf(x))\n";
printf "#define cosh(x) (coshf(x))\n";
printf "#define cospi(x) (M_1_PI*cosf(x))\n";

printf "#define erfc(x) (erfcf(x))\n";
printf "#define erf(x) (erff(x))\n";
printf "#define exp(x) (expf(x))\n";
printf "#define exp2(x) (exp2f(x))\n";
printf "#define exp10(x) (exp10f(x))\n";
printf "#define expm1(x) (expm1f(x))\n";

printf "#define fabs(x) (fabsf(x))\n";
printf "#define floor(x) (floorf(x))\n";

printf "#define lgamma(x) (lgammaf(x))\n";
printf "#define log(x) (logf(x))\n";
printf "#define log2(x) (log2f(x))\n";
printf "#define log10(x) (log10f(x))\n";
printf "#define log1p(x) (log1pf(x))\n";
printf "#define logb(x) (logbf(x))\n";
printf "#define rint(x) (rintf(x))\n";
printf "#define round(x) (roundf(x))\n";
printf "#define rsqrt(x) (1.0f/sqrtf(x))\n";

printf "#define sin(x) (sinf(x))\n";
printf "#define sinh(x) (sinhf(x))\n";
printf "#define sinpi(x) (M_1_PI*sin(x))\n";
printf "#define sqrt(x) (sqrtf(x))\n";

printf "#define atan2(x,y) (atan2f(x,y))\n";
printf "#define atan2pi(x,y) (M_1_PI*atan2f(x,y))\n";
printf "#define copysign(x,y) (copysignf(x,y))\n";
printf "#define fdim(x,y) (fdimf(x,y))\n";
printf "#define fmax(x,y) (fmaxf(x,y))\n";
printf "#define fmin(x,y) (fminf(x,y))\n";
printf "#define fmod(x,y) (fmodf(x,y))\n";
printf "#define hypot(x,y) (hypotf(x,y))\n";
printf "#define nextafter(x,y) (nextafterf(x,y))\n";
printf "#define pow(x,y) (powf(x,y))\n";
printf "#define powr(x,y) (powf(x,y))\n";
printf "#define remainder(x,y) (remainderf(x,y))\n";


printf "float exp2f(float);\n";
printf "float exp10f(float);\n";
printf "float log2f(float);\n";
printf "float roundf(float);\n";
printf "float fdimf(float, float);\n";
printf "float fmaxf(float, float);\n";
printf "float fminf(float, float);\n";

printf "int main( int argc, char** argv )\n";
printf "{\n";
printf "int i,j;\n";
printf "cl_uint err; \n";
printf "\n";

printf "size_t size = SIZE;\n";
printf "size_t blocksize = BLOCKSIZE;\n";
printf "\n";
printf "i=1;\n";
printf "char* arg;\n";
printf "while(i<argc) {\n";
printf "arg=argv[i++];\n";
printf "if (!strncmp(arg,\"--size\",6)) size = atoi(argv[i++]);\n";
printf "else if (!strncmp(arg,\"--blocksize\",11)) blocksize = atoi(argv[i++]);\n";
printf "else {\n";
printf "fprintf(stderr,\"unrecognized option '%s'\\n\",arg);\n";
printf "exit(-1);\n";
printf "}\n";
printf "}\n";
printf "\n";

printf "cl_uint nplatforms;\n";
printf "cl_platform_id* platforms;\n";
printf "\n";
printf "if (clGetPlatformIDs(0,0,&nplatforms)) exit(__LINE__);\n";
printf "\n";
printf "if (nplatforms == 0) exit(__LINE__);\n";
printf "\n";
printf "platforms = (cl_platform_id*)malloc(nplatforms*sizeof(cl_platform_id));\n";
printf "\n";
printf "if (!platforms) exit(__LINE__);\n";
printf "\n";
printf "if (clGetPlatformIDs(nplatforms,platforms,0)) exit(__LINE__);\n";
printf "\n";
printf "char buf[256];\n";
printf "\n";
printf "cl_uint ndev = 0;\n";
printf "if (clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_CPU,0,0,&ndev)) \n";
printf "exit(__LINE__);\n";
printf "\n";
printf "cl_device_id* devices = (cl_device_id*)malloc(ndev*sizeof(cl_device_id));\n";
printf "\n";
printf "if (!devices) exit(__LINE__);\n";
printf "\n";
printf "if (clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_CPU,ndev,devices,0)) \n";
printf "exit(__LINE__);\n";
printf "\n";
printf "cl_context_properties ctxprop[3] = {\n";
printf "(cl_context_properties)CL_CONTEXT_PLATFORM,\n";
printf "(cl_context_properties)platforms[0],\n";
printf "(cl_context_properties)0\n";
printf "};\n";
printf "\n";
printf "cl_context ctx = clCreateContext(ctxprop,ndev,devices,0,0,&err);\n";
printf "if (err) exit(__LINE__);\n";
printf "\n";
printf "cl_command_queue cmdq = clCreateCommandQueue(ctx,devices[0],0,&err);\n";
printf "if (err) exit(__LINE__);\n";


for($c=0;$c<10-2;++$c) {
printf "float* aa$c = (float*)malloc(size*sizeof(int));\n";
printf "if (!aa$c) exit(__LINE__);\n";
printf "float* bb$c = (float*)malloc(size*sizeof(int));\n";
printf "if (!bb$c) exit(__LINE__);\n";
}


printf "for(i=0;i<size;i++) { \n";
printf "aa0[i] = 1.0f/((float)(i+1)*(1.1));\n";
printf "aa1[i] = ((float)(i+1)*(1.2));\n";
printf "aa2[i] = 1.0f/((float)(i+1)*(1.3));\n";
printf "aa3[i] = 1.0f/((float)(i+1)*(1.4));\n";
printf "aa4[i] = 1.0f/((float)((i%7)-3.5+1)*(1.4));\n";
for($c=5;$c<10-2;++$c) {
printf "aa".$c."[i] = 1.0f/((float)(i+1)*(1.1+0.".$c.")); bb".$c."[i] = 0; \n";
}
for($c=0;$c<10-2;++$c) {
printf "bb".$c."[i] = 0; \n";
}
printf "}\n";


for($c=0;$c<10-2;++$c) {
printf "cl_mem bufa$c = clCreateBuffer(ctx,CL_MEM_USE_HOST_PTR,size*sizeof(float),aa$c,&err);\n";
printf "if (err) exit(__LINE__);\n";
printf "cl_mem bufb$c = clCreateBuffer(ctx,CL_MEM_USE_HOST_PTR,size*sizeof(float),bb$c,&err);\n";
printf "if (err) exit(__LINE__);\n";
}

printf "size_t file_sz;\n";
printf "void* pfile;\n";
printf "__mapfile(\"$clfile\",file_sz,pfile);\n";

printf "cl_program prg=clCreateProgramWithSource(\n";
printf "ctx,1,(const char**)&pfile,&file_sz,&err);\n";
printf "if (err) exit(__LINE__);\n";

printf "if (clBuildProgram(prg,ndev,devices,0,0,0)) exit(__LINE__);\n";

printf "size_t gws1[] = { size };\n";
printf "size_t lws1[] = { blocksize };\n";
printf "cl_event ev[10];\n";
printf "cl_kernel krn;\n";
printf "float sum,sum_correct;\n";
printf "float tol = pow(10.0,-8+log10((float)size));\n";




foreach $f (@builtins_unary_0) {

printf "krn = clCreateKernel(prg,\"$testprefix".$f."_kern\",&err);\n";
printf "if (err) exit(__LINE__);\n";

printf "if (clSetKernelArg(krn,0,sizeof(cl_mem),&bufa0)) exit(__LINE__);\n";
printf "if (clSetKernelArg(krn,1,sizeof(cl_mem),&bufb0)) exit(__LINE__);\n";

printf "if (clEnqueueNDRangeKernel(cmdq,krn,1,0,gws1,lws1,0,0,&ev[0])) \n";
printf "exit(__LINE__);\n";

printf "if (clEnqueueReadBuffer(cmdq,bufb0,CL_TRUE,0,size*sizeof(float),bb0,0,0,&ev[1])) \n";
printf "exit(__LINE__);\n";

printf "if (clWaitForEvents(2,ev)) exit(__LINE__);\n";

printf "sum_correct = 0;\n";
printf "for(i=0;i<size;i++) sum_correct += $f(aa0[i]);\n";

printf "sum = 0;\n";
printf "for(i=0;i<size;i++) sum += bb0[i];\n";
printf "printf(\"(%%e) sum ".$f." %%e\",sum_correct,sum);\n";
printf "printf(\" relerr %%e (%%e)\\n\",fabs((sum-sum_correct)/sum_correct),tol);\n";
printf "if (fabs((sum-sum_correct)/sum_correct) > tol) exit(__LINE__);\n";

printf "if (clReleaseEvent(ev[0])) exit(__LINE__);\n";
printf "if (clReleaseEvent(ev[1])) exit(__LINE__);\n";

printf "if (clReleaseKernel(krn)) exit(__LINE__);\n";

}



foreach $f (@builtins_unary_1) {

printf "krn = clCreateKernel(prg,\"$testprefix".$f."_kern\",&err);\n";
printf "if (err) exit(__LINE__);\n";

printf "if (clSetKernelArg(krn,0,sizeof(cl_mem),&bufa1)) exit(__LINE__);\n";
printf "if (clSetKernelArg(krn,1,sizeof(cl_mem),&bufb1)) exit(__LINE__);\n";

printf "if (clEnqueueNDRangeKernel(cmdq,krn,1,0,gws1,lws1,0,0,&ev[0])) \n";
printf "exit(__LINE__);\n";

printf "if (clEnqueueReadBuffer(cmdq,bufb1,CL_TRUE,0,size*sizeof(float),bb1,0,0,&ev[1])) \n";
printf "exit(__LINE__);\n";

printf "if (clWaitForEvents(2,ev)) exit(__LINE__);\n";

printf "sum_correct = 0;\n";
printf "for(i=0;i<size;i++) sum_correct += $f(aa1[i]);\n";

printf "sum = 0;\n";
printf "for(i=0;i<size;i++) sum += bb1[i];\n";
printf "printf(\"(%%e) sum ".$f." %%e\",sum_correct,sum);\n";
printf "printf(\" relerr %%e (%%e)\\n\",fabs((sum-sum_correct)/sum_correct),tol);\n";
printf "if (fabs((sum-sum_correct)/sum_correct) > tol) exit(__LINE__);\n";

printf "if (clReleaseEvent(ev[0])) exit(__LINE__);\n";
printf "if (clReleaseEvent(ev[1])) exit(__LINE__);\n";

printf "if (clReleaseKernel(krn)) exit(__LINE__);\n";

}




foreach $f (@builtins_binary_23) {

printf "krn = clCreateKernel(prg,\"$testprefix".$f."_kern\",&err);\n";
printf "if (err) exit(__LINE__);\n";

printf "if (clSetKernelArg(krn,0,sizeof(cl_mem),&bufa2)) exit(__LINE__);\n";
printf "if (clSetKernelArg(krn,1,sizeof(cl_mem),&bufa3)) exit(__LINE__);\n";
printf "if (clSetKernelArg(krn,2,sizeof(cl_mem),&bufb2)) exit(__LINE__);\n";

printf "if (clEnqueueNDRangeKernel(cmdq,krn,1,0,gws1,lws1,0,0,&ev[0])) \n";
printf "exit(__LINE__);\n";

printf "if (clEnqueueReadBuffer(cmdq,bufb1,CL_TRUE,0,size*sizeof(float),bb1,0,0,&ev[1])) \n";
printf "exit(__LINE__);\n";

printf "if (clWaitForEvents(2,ev)) exit(__LINE__);\n";

printf "sum_correct = 0;\n";
printf "for(i=0;i<size;i++) sum_correct += $f(aa2[i],aa3[i]);\n";

printf "sum = 0;\n";
printf "for(i=0;i<size;i++) sum += bb2[i];\n";
printf "printf(\"(%%e) sum ".$f." %%e\",sum_correct,sum);\n";
printf "printf(\" relerr %%e (%%e)\\n\",fabs((sum-sum_correct)/sum_correct),tol);\n";
printf "if (fabs((sum-sum_correct)/sum_correct) > tol) exit(__LINE__);\n";

printf "if (clReleaseEvent(ev[0])) exit(__LINE__);\n";
printf "if (clReleaseEvent(ev[1])) exit(__LINE__);\n";

printf "if (clReleaseKernel(krn)) exit(__LINE__);\n";

}


foreach $f (@builtins_binary_43) {

printf "krn = clCreateKernel(prg,\"$testprefix".$f."_kern\",&err);\n";
printf "if (err) exit(__LINE__);\n";

printf "if (clSetKernelArg(krn,0,sizeof(cl_mem),&bufa4)) exit(__LINE__);\n";
printf "if (clSetKernelArg(krn,1,sizeof(cl_mem),&bufa3)) exit(__LINE__);\n";
printf "if (clSetKernelArg(krn,2,sizeof(cl_mem),&bufb2)) exit(__LINE__);\n";

printf "if (clEnqueueNDRangeKernel(cmdq,krn,1,0,gws1,lws1,0,0,&ev[0])) \n";
printf "exit(__LINE__);\n";

printf "if (clEnqueueReadBuffer(cmdq,bufb1,CL_TRUE,0,size*sizeof(float),bb1,0,0,&ev[1])) \n";
printf "exit(__LINE__);\n";

printf "if (clWaitForEvents(2,ev)) exit(__LINE__);\n";

printf "sum_correct = 0;\n";
printf "for(i=0;i<size;i++) sum_correct += $f(aa4[i],aa3[i]);\n";

printf "sum = 0;\n";
printf "for(i=0;i<size;i++) sum += bb2[i];\n";
printf "printf(\"(%%e) sum ".$f." %%e\",sum_correct,sum);\n";
printf "printf(\" relerr %%e (%%e)\\n\",fabs((sum-sum_correct)/sum_correct),tol);\n";
printf "if (fabs((sum-sum_correct)/sum_correct) > tol) exit(__LINE__);\n";

printf "if (clReleaseEvent(ev[0])) exit(__LINE__);\n";
printf "if (clReleaseEvent(ev[1])) exit(__LINE__);\n";

printf "if (clReleaseKernel(krn)) exit(__LINE__);\n";

}




printf "if (clReleaseProgram(prg)) exit(__LINE__);\n";

for($c=0;$c<10-2;++$c) {
printf "if (clReleaseMemObject(bufa$c)) exit(__LINE__);\n";
printf "if (clReleaseMemObject(bufb$c)) exit(__LINE__);\n";
}

printf "if (clReleaseCommandQueue(cmdq)) exit(__LINE__);\n";
printf "if (clReleaseContext(ctx)) exit(__LINE__);\n";
printf "printf(\"done!\\n\");\n";
printf "return(0);\n";
printf "}\n";

