
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <sys/mman.h>

#include "stdcl.h"

#define SIZE 128
#define BLOCKSIZE 4

#define __mapfile(file,filesz,pfile) do { \
int fd = open(file,O_RDONLY); \
struct stat fst; fstat(fd,&fst); \
if (fst.st_size == 0 || !S_ISREG(fst.st_mode)) { \
fprintf(stderr,"error: open failed on ''\n",file); \
pfile=0; \
} else { \
filesz = fst.st_size; \
pfile = mmap(0,filesz,PROT_READ,MAP_PRIVATE,fd,0); \
} \
close(fd); \
} while(0);

//#define ____exit(err,line) \
//do { fprintf("error code %d\n",err); __exit(line); } while(0)

#define __exit(line) \
do { printf("error code at line %d\n",line); exit(line); } while(0)

int main( int argc, char** argv )
{
int i,j;
cl_uint err; 

CONTEXT* cp = 0;
unsigned int devnum = 0;
size_t size = SIZE;
size_t blocksize = BLOCKSIZE;

i=1;
char* arg;
while(i<argc) {
arg=argv[i++];
if (!strncmp(arg,"--size",6)) size = atoi(argv[i++]);
else if (!strncmp(arg,"--blocksize",11)) blocksize = atoi(argv[i++]);
else if (!strncmp(arg,"--dev",6)) { cp = stddev; devnum = atoi(argv[i++]); }
else if (!strncmp(arg,"--cpu",6)) { cp = stdcpu; devnum = atoi(argv[i++]); }
else if (!strncmp(arg,"--gpu",6)) { cp = stdgpu; devnum = atoi(argv[i++]); }
else {
fprintf(stderr,"unrecognized option ''\n",arg);
__exit(-1);
}
}

if (!cp) __exit(__LINE__);
if (devnum >= clgetndev(cp)) __exit(__LINE__);
int* aa0 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa0) __exit(__LINE__);
int* bb0 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb0) __exit(__LINE__);
int* aa1 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa1) __exit(__LINE__);
int* bb1 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb1) __exit(__LINE__);
int* aa2 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa2) __exit(__LINE__);
int* bb2 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb2) __exit(__LINE__);
int* aa3 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa3) __exit(__LINE__);
int* bb3 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb3) __exit(__LINE__);
int* aa4 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa4) __exit(__LINE__);
int* bb4 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb4) __exit(__LINE__);
int* aa5 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa5) __exit(__LINE__);
int* bb5 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb5) __exit(__LINE__);
int* aa6 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa6) __exit(__LINE__);
int* bb6 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb6) __exit(__LINE__);
int* aa7 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa7) __exit(__LINE__);
int* bb7 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb7) __exit(__LINE__);
for(i=0;i<size;i++) { 
aa0[i] = i+13*0; bb0[i] = 0; 
aa1[i] = i+13*1; bb1[i] = 0; 
aa2[i] = i+13*2; bb2[i] = 0; 
aa3[i] = i+13*3; bb3[i] = 0; 
aa4[i] = i+13*4; bb4[i] = 0; 
aa5[i] = i+13*5; bb5[i] = 0; 
aa6[i] = i+13*6; bb6[i] = 0; 
aa7[i] = i+13*7; bb7[i] = 0; 
}
void* clh = clopen(cp,"test_arg_int.cl",CLLD_NOW);
cl_kernel krn;
clndrange_t ndr = clndrange_init1d(0,size,blocksize);
cl_event ev[10];
int sum,sum_correct;
float tol = powf(10.0,-8+log10((float)size));
krn = clsym(cp,clh,"test_arg_1_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 1 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_1_2_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
clarg_set_global(cp,krn,2,bb1);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<2;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
printf("(%d) sum 1 2 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_2_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<2;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 2 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_1_3_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
clarg_set_global(cp,krn,2,bb1);
clarg_set_global(cp,krn,3,bb2);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<3;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
printf("(%d) sum 1 3 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_2_2_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,bb0);
clarg_set_global(cp,krn,3,bb1);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<2;++i)
for(j=0;j<2;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
printf("(%d) sum 2 2 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_3_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<3;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 3 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_1_4_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
clarg_set_global(cp,krn,2,bb1);
clarg_set_global(cp,krn,3,bb2);
clarg_set_global(cp,krn,4,bb3);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<4;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
printf("(%d) sum 1 4 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_2_3_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,bb0);
clarg_set_global(cp,krn,3,bb1);
clarg_set_global(cp,krn,4,bb2);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<2;++i)
for(j=0;j<3;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
printf("(%d) sum 2 3 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_3_2_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,bb0);
clarg_set_global(cp,krn,4,bb1);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<3;++i)
for(j=0;j<2;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
printf("(%d) sum 3 2 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_4_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<4;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 4 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_1_5_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
clarg_set_global(cp,krn,2,bb1);
clarg_set_global(cp,krn,3,bb2);
clarg_set_global(cp,krn,4,bb3);
clarg_set_global(cp,krn,5,bb4);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<5;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
printf("(%d) sum 1 5 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_2_4_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,bb0);
clarg_set_global(cp,krn,3,bb1);
clarg_set_global(cp,krn,4,bb2);
clarg_set_global(cp,krn,5,bb3);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<2;++i)
for(j=0;j<4;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
printf("(%d) sum 2 4 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_3_3_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,bb0);
clarg_set_global(cp,krn,4,bb1);
clarg_set_global(cp,krn,5,bb2);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<3;++i)
for(j=0;j<3;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
printf("(%d) sum 3 3 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_4_2_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,bb0);
clarg_set_global(cp,krn,5,bb1);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<4;++i)
for(j=0;j<2;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
printf("(%d) sum 4 2 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_5_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<5;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 5 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_1_6_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
clarg_set_global(cp,krn,2,bb1);
clarg_set_global(cp,krn,3,bb2);
clarg_set_global(cp,krn,4,bb3);
clarg_set_global(cp,krn,5,bb4);
clarg_set_global(cp,krn,6,bb5);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb5,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<6;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
for(i=0;i<size;i++) sum += bb5[i];
printf("(%d) sum 1 6 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_2_5_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,bb0);
clarg_set_global(cp,krn,3,bb1);
clarg_set_global(cp,krn,4,bb2);
clarg_set_global(cp,krn,5,bb3);
clarg_set_global(cp,krn,6,bb4);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<2;++i)
for(j=0;j<5;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
printf("(%d) sum 2 5 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_3_4_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,bb0);
clarg_set_global(cp,krn,4,bb1);
clarg_set_global(cp,krn,5,bb2);
clarg_set_global(cp,krn,6,bb3);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<3;++i)
for(j=0;j<4;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
printf("(%d) sum 3 4 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_4_3_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,bb0);
clarg_set_global(cp,krn,5,bb1);
clarg_set_global(cp,krn,6,bb2);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<4;++i)
for(j=0;j<3;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
printf("(%d) sum 4 3 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_5_2_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,bb0);
clarg_set_global(cp,krn,6,bb1);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<5;++i)
for(j=0;j<2;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
printf("(%d) sum 5 2 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_6_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa5,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,aa5);
clarg_set_global(cp,krn,6,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<6;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 6 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_1_7_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
clarg_set_global(cp,krn,2,bb1);
clarg_set_global(cp,krn,3,bb2);
clarg_set_global(cp,krn,4,bb3);
clarg_set_global(cp,krn,5,bb4);
clarg_set_global(cp,krn,6,bb5);
clarg_set_global(cp,krn,7,bb6);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb5,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb6,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<7;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
for(i=0;i<size;i++) sum += bb5[i];
for(i=0;i<size;i++) sum += bb6[i];
printf("(%d) sum 1 7 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_2_6_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,bb0);
clarg_set_global(cp,krn,3,bb1);
clarg_set_global(cp,krn,4,bb2);
clarg_set_global(cp,krn,5,bb3);
clarg_set_global(cp,krn,6,bb4);
clarg_set_global(cp,krn,7,bb5);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb5,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<2;++i)
for(j=0;j<6;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
for(i=0;i<size;i++) sum += bb5[i];
printf("(%d) sum 2 6 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_3_5_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,bb0);
clarg_set_global(cp,krn,4,bb1);
clarg_set_global(cp,krn,5,bb2);
clarg_set_global(cp,krn,6,bb3);
clarg_set_global(cp,krn,7,bb4);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<3;++i)
for(j=0;j<5;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
printf("(%d) sum 3 5 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_4_4_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,bb0);
clarg_set_global(cp,krn,5,bb1);
clarg_set_global(cp,krn,6,bb2);
clarg_set_global(cp,krn,7,bb3);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<4;++i)
for(j=0;j<4;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
printf("(%d) sum 4 4 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_5_3_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,bb0);
clarg_set_global(cp,krn,6,bb1);
clarg_set_global(cp,krn,7,bb2);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<5;++i)
for(j=0;j<3;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
printf("(%d) sum 5 3 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_6_2_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa5,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,aa5);
clarg_set_global(cp,krn,6,bb0);
clarg_set_global(cp,krn,7,bb1);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<6;++i)
for(j=0;j<2;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
printf("(%d) sum 6 2 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_7_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa5,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa6,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,aa5);
clarg_set_global(cp,krn,6,aa6);
clarg_set_global(cp,krn,7,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<7;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 7 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_1_8_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
clarg_set_global(cp,krn,2,bb1);
clarg_set_global(cp,krn,3,bb2);
clarg_set_global(cp,krn,4,bb3);
clarg_set_global(cp,krn,5,bb4);
clarg_set_global(cp,krn,6,bb5);
clarg_set_global(cp,krn,7,bb6);
clarg_set_global(cp,krn,8,bb7);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb5,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb6,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb7,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<8;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
for(i=0;i<size;i++) sum += bb5[i];
for(i=0;i<size;i++) sum += bb6[i];
for(i=0;i<size;i++) sum += bb7[i];
printf("(%d) sum 1 8 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_2_7_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,bb0);
clarg_set_global(cp,krn,3,bb1);
clarg_set_global(cp,krn,4,bb2);
clarg_set_global(cp,krn,5,bb3);
clarg_set_global(cp,krn,6,bb4);
clarg_set_global(cp,krn,7,bb5);
clarg_set_global(cp,krn,8,bb6);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb5,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb6,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<2;++i)
for(j=0;j<7;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
for(i=0;i<size;i++) sum += bb5[i];
for(i=0;i<size;i++) sum += bb6[i];
printf("(%d) sum 2 7 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_3_6_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,bb0);
clarg_set_global(cp,krn,4,bb1);
clarg_set_global(cp,krn,5,bb2);
clarg_set_global(cp,krn,6,bb3);
clarg_set_global(cp,krn,7,bb4);
clarg_set_global(cp,krn,8,bb5);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb5,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<3;++i)
for(j=0;j<6;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
for(i=0;i<size;i++) sum += bb5[i];
printf("(%d) sum 3 6 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_4_5_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,bb0);
clarg_set_global(cp,krn,5,bb1);
clarg_set_global(cp,krn,6,bb2);
clarg_set_global(cp,krn,7,bb3);
clarg_set_global(cp,krn,8,bb4);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb4,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<4;++i)
for(j=0;j<5;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
for(i=0;i<size;i++) sum += bb4[i];
printf("(%d) sum 4 5 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_5_4_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,bb0);
clarg_set_global(cp,krn,6,bb1);
clarg_set_global(cp,krn,7,bb2);
clarg_set_global(cp,krn,8,bb3);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb3,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<5;++i)
for(j=0;j<4;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
for(i=0;i<size;i++) sum += bb3[i];
printf("(%d) sum 5 4 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_6_3_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa5,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,aa5);
clarg_set_global(cp,krn,6,bb0);
clarg_set_global(cp,krn,7,bb1);
clarg_set_global(cp,krn,8,bb2);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb2,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<6;++i)
for(j=0;j<3;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
for(i=0;i<size;i++) sum += bb2[i];
printf("(%d) sum 6 3 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_7_2_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa5,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa6,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,aa5);
clarg_set_global(cp,krn,6,aa6);
clarg_set_global(cp,krn,7,bb0);
clarg_set_global(cp,krn,8,bb1);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<7;++i)
for(j=0;j<2;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
printf("(%d) sum 7 2 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_8_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa2,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa3,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa4,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa5,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa6,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa7,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,aa2);
clarg_set_global(cp,krn,3,aa3);
clarg_set_global(cp,krn,4,aa4);
clarg_set_global(cp,krn,5,aa5);
clarg_set_global(cp,krn,6,aa6);
clarg_set_global(cp,krn,7,aa7);
clarg_set_global(cp,krn,8,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<8;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 8 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
printf("done!\n");
clclose(cp,clh);
return(0);
}
