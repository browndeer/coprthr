/* clcc.c
 *
 * Copyright (c) 2008-2011 Brown Deer Technology, LLC.  All Rights Reserved.
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <libelf.h>
#include <errno.h>
#include <openssl/md5.h>

#include <CL/cl.h>
#define CL_CONTEXT_OFFLINE_DEVICES_AMD 0x403F

#include "clelf.h"

char shstrtab[] = { 
		"\0"
		".clprgtab\0" /* section 1, shstrtab offset 1 */
		".clkrntab\0" /* section 2, shstrtab offset 11 */
		".clprgs\0" /* section 3, shstrtab offset 21 */
		".cltexts\0" /* section 4, shstrtab offset 29 */
		".clprgb\0" /* section 5, shstrtab offset 38 */
		".cltextb\0" /* section 6, shstrtab offset 46 */
		".clstrtab\0" /* section 7, shstrtab offset 55 */
		".shstrtab\0" /* section 8, shstrtab offset 65 */
};

int shstrtab_offset[] = { 0,1,11,21,29,38,46,55,65 };

void usage() 
{
	printf("usage: WRONG-FIX clcc [options] file...\n");
	printf("options:\n");
	printf("\t--cl-binary \n");
	printf("\t--cl-device DEVICE\n");
	printf("\t--cl-path PATH\n");
	printf("\t--cl-platform PLATFORM\n");
	printf("\t--cl-source\n");
	printf("\t--help, -h\n");
	printf("\t--version, -v\n");
}

void version()
{
	printf("BDT clcc\n"); 
	printf(
		"Copyright (c) 2008-2011 Brown Deer Technology, LLC."
		" All Rights Reserved.\n"
	);
	printf("This program is free software distributed under LGPLv3.\n");
}

void add_path( char* path_str, size_t* path_str_len, char* path )
{
	size_t len = strnlen(path,1024);

	if (*path_str_len + len + 2 > DEFAULT_STR_SIZE) {
		fprintf(stderr,"clcc: error: path buffer overflow\n");
		exit(-1);
	}

	strncpy(path_str+*path_str_len,path,len);
	*path_str_len += len+1;
}

int resolve_fullpath( 
		char* fullpath, const char* filename, 
		char* path_str, size_t path_str_len
)
{
	char* p = path_str;

	struct stat st;

	while (p+1 < path_str+path_str_len) {

		snprintf(fullpath,DEFAULT_STR_SIZE,"%s/%s",p,filename);

		if (!stat(fullpath,&st) && S_ISREG(st.st_mode)) return(0);

		p += strnlen(p,DEFAULT_STR_SIZE-((intptr_t)p+(intptr_t)path_str)) + 1;
	}

	*fullpath = '\0';

	return(-1);
}


#define add_strtab(str,strp,alloc_sz,str2) do { \
   size_t offset = (intptr_t)(strp-str); \
   size_t len2 = strnlen(str2,DEFAULT_STR_SIZE); \
   while ( offset + len2 + 1 > alloc_sz) { \
      alloc_sz += DEFAULT_STR_SIZE; \
      str = (char*)realloc(str,alloc_sz); \
      strp = str + offset; \
   } \
   strncpy(strp,str2,len2+1); \
	strp += len2 + 1; \
   } while(0)


int main(int argc, char** argv)
{
	int i,j,k;
	int n;

	char default_platform[] = "platform-unknown";
	char default_device[] = "device-unknown";

	char* platform = default_platform;
	char* device = default_device;

//	int sw_src_bin = 0;

	char* path_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	path_str[0] = '.';
	size_t path_str_len = 2;

	unsigned int clprgtab_nalloc = DEFAULT_CLPRGTAB_NALLOC;
	struct clprgtab_entry* clprgtab = (struct clprgtab_entry*)
		malloc(__clprgtab_entry_sz*clprgtab_nalloc);
	unsigned int clprgtab_n = 0;

	unsigned int clkrntab_nalloc = DEFAULT_CLKRNTAB_NALLOC;
	struct clkrntab_entry* clkrntab = (struct clkrntab_entry*)
		malloc(__clkrntab_entry_sz*clkrntab_nalloc);
	unsigned int clkrntab_n = 0;

	unsigned int clprgs_nalloc = DEFAULT_CLPRGS_NALLOC;
	struct clprgs_entry* clprgs = (struct clprgs_entry*)
		malloc(__clprgs_entry_sz*clprgs_nalloc);
	unsigned int clprgs_n = 0;

	unsigned int clprgb_nalloc = DEFAULT_CLPRGB_NALLOC;
	struct clprgb_entry* clprgb = (struct clprgb_entry*)
		malloc(__clprgb_entry_sz*clprgb_nalloc);
	unsigned int clprgb_n = 0;

	size_t cltexts_buf_alloc = DEFAULT_BUF_ALLOC;
	char* cltexts_buf = (char*)calloc(1,cltexts_buf_alloc);
	char* cltexts_bufp = cltexts_buf;

	size_t cltextb_buf_alloc = DEFAULT_BUF_ALLOC;
	char* cltextb_buf = (char*)calloc(1,cltextb_buf_alloc);
	char* cltextb_bufp = cltextb_buf;

	size_t clstrtab_str_alloc = DEFAULT_STR_SIZE;
	char* clstrtab_str = (char*)calloc(1,clstrtab_str_alloc);
	char* clstrtab_strp = clstrtab_str;
	

	char* fullpath = (char*)malloc(DEFAULT_STR_SIZE);

   int lang = LANG_OPENCL;
   int en_openmp = 0;
   int en_source = 1;
   int en_binary = 1;

   /* XXX todo - add ability to provide lang specific options -DAR */


   char wdtemp[] = "/tmp/xclXXXXXX";
   char* wd = mkdtemp(wdtemp);

	char* fname = 0;

printf("\nXXX add option to use only devices that are present\n\n");

	i = 1;
	while (i < argc) {

      if (!strcmp(argv[i],"-fopencl")) {

         lang = LANG_OPENCL;

      } else if (!strcmp(argv[i],"-fstdcl")) {

         lang = LANG_STDCL;

      } else if (!strcmp(argv[i],"-fcuda")) {

         lang = LANG_CUDA;

      } else if (!strcmp(argv[i],"-fopenmp")) {

         en_openmp = 1;

      } else if (!strcmp(argv[i],"--no-source")) {

         en_source = 0;

      } else if (!strcmp(argv[i],"--no-binary")) {

         en_binary = 0;

		} else if (!strcmp(argv[i],"--cl-path")) {

			add_path(path_str,&path_str_len,argv[++i]);

		} else if (!strcmp(argv[i],"-h")||!strcmp(argv[i],"--help")) {

			usage(); 
			exit(0);

		} else if (!strcmp(argv[i],"-v")||!strcmp(argv[i],"--version")) {

			version(); 
			exit(0);

		} else { /* nothing left, assume the arg is a filename */

         fname = argv[i];

		}

		++i;

	}


	/***
	 *** map source file
	 ***/

   struct stat st;
   if ( stat(fname,&st) == -1 || !S_ISREG(st.st_mode)) {
   fprintf(stderr,"clcc: '%s' no such file\n",fname);
      exit(-1);
   }

	int fd = open(fname,O_RDONLY);

	if (fd < 0) {
      fprintf(stderr,"clcc: '%s' open failed\n",fname);
      exit(-1);
   }

   size_t file_sz = st.st_size;
   void* file_ptr = mmap(0,file_sz,PROT_READ,MAP_PRIVATE,fd,0);

   close(fd);



	/***
	 *** add clprgs entry
	 ***/

	if (clprgs_n >= clprgs_nalloc) {
		clprgs_nalloc += DEFAULT_CLPRGS_NALLOC;
		clprgs = (struct clprgs_entry*)
			realloc(clprgs,__clprgs_entry_sz*clprgs_nalloc);
	}

	clprgs[clprgs_n].e_name = (intptr_t)(clstrtab_strp-clstrtab_str);
//	strncpy(clstrtab_strp,fname,DEFAULT_STR_SIZE);
//	clstrtab_strp += strnlen(fname,DEFAULT_STR_SIZE) + 1;
	add_strtab(clstrtab_str,clstrtab_strp,clstrtab_str_alloc,fname);
	clprgs[clprgs_n].e_info = 0;
	clprgs[clprgs_n].e_platform = 0;
	clprgs[clprgs_n].e_device = 0;
	clprgs[clprgs_n].e_shndx = -1;
	clprgs[clprgs_n].e_offset = (intptr_t)(cltexts_bufp-cltexts_buf);
	clprgs[clprgs_n].e_size = st.st_size;
	++clprgs_n;

//	strncpy(clstrtab_strp,fullpath,DEFAULT_STR_SIZE);
//	clstrtab_strp += strnlen(fullpath,DEFAULT_STR_SIZE) + 1;

	size_t offset = (intptr_t)cltexts_bufp-(intptr_t)cltexts_buf;

	while (offset+file_sz>cltexts_buf_alloc) {
		cltexts_buf_alloc += DEFAULT_BUF_ALLOC;
		cltexts_buf = realloc(cltexts_buf,cltexts_buf_alloc);
		cltexts_bufp = cltexts_buf + offset;
	}

	memcpy(cltexts_bufp,file_ptr,file_sz);

	cltexts_bufp += file_sz;



	/***
	 *** compile program for each opencl platform
	 ***/

	int err;

	cl_uint nplatforms;
	clGetPlatformIDs(0,0,&nplatforms);
	cl_platform_id* platforms
		= (cl_platform_id*)malloc(nplatforms*sizeof(cl_platform_id));
	clGetPlatformIDs(nplatforms,platforms,0);

//	printf("number of platforms %d\n",nplatforms);

	cl_context* contexts
		= (cl_context*)malloc(nplatforms*sizeof(cl_context));

	cl_program* programs
		= (cl_program*)malloc(nplatforms*sizeof(cl_program));

	int nbin_valid = 0;

	for(i=0; i<nplatforms; i++) {

		char info[1024];
      clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1024,info,0);
   	printf("|%s|\n",info);

		int platform_code = 0;

		if (!strncasecmp(info,"AMD",3)) {

			platform_code = CLELF_PLATFORM_CODE_AMDAPP;

		} else if (!strncasecmp(info,"Nvidia",6)) {

			platform_code = CLELF_PLATFORM_CODE_NVIDIA;

		} else if (!strncasecmp(info,"coprthr",7)) {

			platform_code = CLELF_PLATFORM_CODE_COPRTHR;

//		} else if (!strncasecmp(info,"Intel",7)) {
//
//			platform_code = CLELF_PLATFORM_CODE_INTEL;

		} else {

			continue;

		}

		cl_context_properties cprops[5];
		cprops[0] = CL_CONTEXT_PLATFORM;
		cprops[1] = (cl_context_properties)platforms[i];

		switch (platform_code) {

			case CLELF_PLATFORM_CODE_AMDAPP:
				cprops[2] = CL_CONTEXT_OFFLINE_DEVICES_AMD;
				cprops[3] = (cl_context_properties)1;
				cprops[4] = (cl_context_properties)0;
				break;

			default:
				cprops[2] = (cl_context_properties)0;

		}
			
		contexts[i] = clCreateContextFromType(cprops,CL_DEVICE_TYPE_ALL,0,0,&err);	
		programs[i] = clCreateProgramWithSource(
   		contexts[i], 1, (const char**)&file_ptr, &file_sz, &err );

//		printf("source |%s|\n",file_ptr);

		err = clBuildProgram( programs[i], 0, 0, 0, 0, 0 );

		cl_uint ndev;
		err = clGetProgramInfo( programs[i], CL_PROGRAM_NUM_DEVICES,
   		sizeof(cl_uint), &ndev, 0 );

		printf("number of devices %d\n",ndev);

		cl_device_id* devices = (cl_device_id*)malloc(ndev*sizeof(cl_device_id));

		err = clGetProgramInfo (programs[i],CL_PROGRAM_DEVICES,
			ndev*sizeof(cl_device_id),devices,0);


		size_t* bin_sizes = (size_t*)malloc( sizeof(size_t)*ndev );

		err = clGetProgramInfo( programs[i], CL_PROGRAM_BINARY_SIZES,
   		sizeof(size_t)*ndev, bin_sizes, 0 );

		char** bins = (char**)malloc( sizeof(char*)*ndev );

		for( j=0; j < ndev; j++ ) {
//			printf("size for device %d is %d\n",j,bin_sizes[j]);
		   if( bin_sizes[j] != 0 ) {
      		bins[j] = (char*)malloc( sizeof(char)*bin_sizes[j] );
   		} else {
      		bins[j] = 0;
   		}	
		}

		err = clGetProgramInfo( programs[i], CL_PROGRAM_BINARIES,
   		sizeof(char*)*ndev, bins, 0 );

		for( j=0; j < ndev; j++ ) {

			if (bins[j]) {

			Elf32_Ehdr* ehdr32;
			Elf64_Ehdr* ehdr64;
			switch(platform_code) {

				case CLELF_PLATFORM_CODE_AMDAPP:
					ehdr32 = (Elf32_Ehdr*)bins[j];
//					printf("device code %d\n",ehdr32->e_machine);
					break;

				case CLELF_PLATFORM_CODE_COPRTHR:
					ehdr64 = (Elf64_Ehdr*)bins[j];
//					printf("device code %d\n",ehdr64->e_machine);
					break;

				case CLELF_PLATFORM_CODE_NVIDIA:
				default:
					printf("device code %d\n",0);

			}

			}

		}


		for( j=0; j < ndev; j++ ) {

			char device_name[1024];
			err = clGetDeviceInfo(devices[j],CL_DEVICE_NAME,1024,device_name,0);
//			printf("device name |%s|\n",device_name);

			cl_build_status status;
			err = clGetProgramBuildInfo(programs[i],devices[j],
				CL_PROGRAM_BUILD_STATUS,sizeof(cl_build_status),&status,0);
//			printf("build status %d\n",status);

			if (status == CL_BUILD_SUCCESS && bins[j]) {

				++nbin_valid;

				/***
				 *** add clprgb entry
				 ***/

				if (clprgb_n >= clprgb_nalloc) {
					clprgb_nalloc += DEFAULT_CLPRGB_NALLOC;
					clprgb = (struct clprgb_entry*)
						realloc(clprgb,__clprgb_entry_sz*clprgb_nalloc);
				}

				clprgb[clprgb_n].e_name = (intptr_t)(clstrtab_strp-clstrtab_str);
				add_strtab(clstrtab_str,clstrtab_strp,clstrtab_str_alloc,fname);
				clprgb[clprgb_n].e_info = 0;
				clprgb[clprgb_n].e_platform = platform_code;
				clprgb[clprgb_n].e_device = (intptr_t)(clstrtab_strp-clstrtab_str);
//				strncpy(clstrtab_strp,device_name,DEFAULT_STR_SIZE);
//				clstrtab_strp += strnlen(device_name,DEFAULT_STR_SIZE) + 1;
				add_strtab(clstrtab_str,clstrtab_strp,clstrtab_str_alloc,
					device_name);
				clprgb[clprgb_n].e_shndx = -1;
				clprgb[clprgb_n].e_offset = (intptr_t)(cltextb_bufp-cltextb_buf);
				clprgb[clprgb_n].e_size = bin_sizes[j];
				++clprgb_n;

				size_t offset = (intptr_t)cltextb_bufp-(intptr_t)cltextb_buf;
				size_t n = bin_sizes[j];

				while (offset+n>cltextb_buf_alloc) {
					cltextb_buf_alloc += DEFAULT_BUF_ALLOC;
					cltextb_buf = realloc(cltextb_buf,cltextb_buf_alloc);
					cltextb_bufp = cltextb_buf + offset;
				}

				memcpy(cltextb_bufp,bins[j],bin_sizes[j]);

				cltextb_bufp += bin_sizes[j];

				printf("clcc: compile '%s' [%s]\n",fname,device_name);

			} else if (status == CL_BUILD_NONE) {

				printf("clcc: compile '%s' [%s] FAILED\n",fname,device_name);

			} else if (status == CL_BUILD_ERROR || bins[j]==0) {	

				printf("clcc: compile '%s' [%s] FAILED\n",info,fname,device_name);

				char* build_log;
				size_t build_log_sz;
				err = clGetProgramBuildInfo(programs[i],devices[j],
					CL_PROGRAM_BUILD_LOG,0,0,&build_log_sz);
				build_log = (char*)malloc(build_log_sz);
				err = clGetProgramBuildInfo(programs[i],devices[j],
					CL_PROGRAM_BUILD_LOG,build_log_sz,build_log,0);
				if (build_log_sz > 0) printf("%s",build_log);
				if (build_log[build_log_sz-1] != '\n') printf("\n");

				free(build_log);

			}
			
		}

		cl_uint nkrn;

		err = clCreateKernelsInProgram (programs[i],0,0,&nkrn);
		cl_kernel* kernels = (cl_kernel*)malloc(nkrn*sizeof(cl_kernel));
		err = clCreateKernelsInProgram (programs[i],nkrn,kernels,0);

		for(j=0;j<nkrn;j++) {

			char name[1024];
			err = clGetKernelInfo(kernels[j],CL_KERNEL_FUNCTION_NAME,1024,name,0);

//			printf("KKK %s\n",name);

			clkrntab[clkrntab_n].e_name = (intptr_t)(clstrtab_strp-clstrtab_str);
//			strncpy(clstrtab_strp,name,DEFAULT_STR_SIZE);
//			clstrtab_strp += strnlen(name,DEFAULT_STR_SIZE) + 1;
			add_strtab(clstrtab_str,clstrtab_strp,clstrtab_str_alloc,name);

			clkrntab[clkrntab_n].e_info = 0;
			clkrntab[clkrntab_n].e_prg = 0;
			++clkrntab_n;

			clReleaseKernel(kernels[j]);
		}

	}



	/***
	 *** add clprgtab entry
	 ***/

	if (clprgtab_n >= clprgtab_nalloc) {
		clprgtab_nalloc += DEFAULT_CLPRGTAB_NALLOC;
		clprgtab = (struct clprgtab_entry*)
			realloc(clprgtab,__clprgtab_entry_sz*clprgtab_nalloc);
	}

	/* XXX right now we assume krntab/prgs/prgb start is 0	here -DAR */

	clprgtab[clprgtab_n].e_name = (intptr_t)(clstrtab_strp-clstrtab_str);
//	strncpy(clstrtab_strp,fname,DEFAULT_STR_SIZE);
//	clstrtab_strp += strnlen(fname,DEFAULT_STR_SIZE) + 1;
	add_strtab(clstrtab_str,clstrtab_strp,clstrtab_str_alloc,fname);
	clprgtab[clprgtab_n].e_info = 0;
	clprgtab[clprgtab_n].e_prgs = 0;
	clprgtab[clprgtab_n].e_nprgs = clprgs_n;
	clprgtab[clprgtab_n].e_prgb = 0;
	clprgtab[clprgtab_n].e_nprgb = clprgb_n;
	clprgtab[clprgtab_n].e_krn = 0;
	clprgtab[clprgtab_n].e_nkrn = clkrntab_n;
	++clprgtab_n;

//	strncpy(clstrtab_strp,fullpath,DEFAULT_STR_SIZE);
//	clstrtab_strp += strnlen(fullpath,DEFAULT_STR_SIZE) + 1;

	printf("nprgs nprgb %d %d\n",clprgs_n,clprgb_n);

	unsigned long cltextshash[2];
	unsigned long cltextbhash[2];

	size_t len = (intptr_t)cltexts_bufp-(intptr_t)cltexts_buf;
	MD5((const unsigned char*)cltexts_buf, len, (unsigned char*)cltextshash);
	printf("len texts %d file_sz %d\n",len,file_sz);

	len = (intptr_t)cltextb_bufp-(intptr_t)cltextb_buf;
	MD5((const unsigned char*)cltextb_buf, len, (unsigned char*)cltextbhash);

	printf("%lx %lx \n",cltextshash[0],cltextshash[1]);
	printf("%lx %lx \n",cltextbhash[0],cltextbhash[1]);


	/***
	 *** create special elf file
	 ***/

	char tfname[] = "/tmp/clccXXXXXX";
	fd = mkstemp(tfname);

	if (fd < 0) {
		fprintf(stderr,"clcc: mkstemp failed");
		exit(-1);
	}
	
	Elf* e;
	Elf_Scn* scn;
	Elf_Data* data;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr,"clcc: library initialization failed: %s",elf_errmsg(-1));
		exit(-1);
	}

//	if ((fd = open("out_clcc.o", O_WRONLY|O_CREAT, 0777)) < 0) {
//		fprintf(stderr,"open \%s\" failed", "out_clcc.o");
//		exit(-1);
//	}

	if ((e = elf_begin(fd, ELF_C_WRITE, NULL)) == 0) {
		fprintf(stderr,"elf_begin() failed: %d: %s", elf_errno(),elf_errmsg(-1));
		exit(-1);
	}

#if defined(__x86_64__)

	Elf64_Ehdr* ehdr = 0;
	Elf64_Phdr* phdr = 0;
	Elf64_Shdr* shdr = 0;
printf("__x86_64__\n");

#elif defined(__i386__) 

	Elf32_Ehdr* ehdr = 0;
	Elf32_Phdr* phdr = 0;
	Elf32_Shdr* shdr = 0;

printf("__i386__\n");
#endif


	/*
	 * construct elf header
	 */

#if defined(__x86_64__)

	if ((ehdr = elf64_newehdr(e)) == 0) {
		fprintf(stderr,"elf64_newehdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr->e_machine = EM_X86_64; 
	ehdr->e_type = ET_NONE;
	ehdr->e_version = EV_CURRENT;
	ehdr->e_shstrndx = 8; /* set section index of .shstrtab */

#elif defined(__i386__) 

	if ((ehdr = elf32_newehdr(e)) == 0) {
		fprintf(stderr,"elf32_newehdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr->e_machine = EM_386; 
	ehdr->e_type = ET_NONE;
	ehdr->e_shstrndx = 8; /* set section index of .shstrtab */

#endif


	/* 
	 * construct section [1] .clprgtab
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = (char*)clprgtab;
	data->d_type = ELF_T_WORD;
	data->d_size = __clprgtab_entry_sz*clprgtab_n;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[1];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clprgtab_entry_sz;



	/* 
	 * construct section [2] .clkrntab
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = (char*)clkrntab;
	data->d_type = ELF_T_WORD;
	data->d_size = __clkrntab_entry_sz*clkrntab_n;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[2];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clkrntab_entry_sz;



	/* 
	 * construct section [3] .clprgs 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = (char*)clprgs;
	data->d_type = ELF_T_WORD;
	data->d_size = __clprgs_entry_sz*clprgs_n;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[3];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clprgs_entry_sz;


	/* 
	 * construct section [4] .cltexts 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = cltexts_buf;
	data->d_type = ELF_T_BYTE;
	data->d_size = (intptr_t)(cltexts_bufp - cltexts_buf);
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[4];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = 0;


	/* 
	 * construct section [5] .clprgb 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = (char*)clprgb;
	data->d_type = ELF_T_WORD;
	data->d_size = __clprgb_entry_sz*clprgb_n;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[5];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clprgb_entry_sz;


	/* 
	 * construct section [6] .cltextb 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = cltextb_buf;
	data->d_type = ELF_T_BYTE;
	data->d_size = (intptr_t)(cltextb_bufp - cltextb_buf);
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[6];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = 0;


	/* 
	 * construct section [7] .clstrtab 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = clstrtab_str;
	data->d_type = ELF_T_BYTE;
	data->d_size = (intptr_t)(clstrtab_strp - clstrtab_str);
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[7];
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;


	/* 
	 * construct section [8] .shstrtab 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr, "elf_newscn() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	data->d_align = 16;
	data->d_buf = shstrtab;
	data->d_off = 0LL;
	data->d_size = sizeof(shstrtab);
	data->d_type = ELF_T_BYTE;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[8];
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;


	printf("here\n");

	if (elf_update(e, ELF_C_NULL) < 0)  {
		fprintf(stderr, "elf_update(NULL) failed: %s.", elf_errmsg(-1));
		exit(-1);
	}


	if (elf_update(e, ELF_C_WRITE) < 0)  {
		fprintf(stderr, "elf_update() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	elf_end(e);
	close(fd);

	char cmd[1024];

	snprintf( cmd, 1024, "ld -r -o out_clcc.o"
		" %s"
		" --defsym _CLTEXTSHASH0=0x%lx"
		" --defsym _CLTEXTSHASH1=0x%lx"
		" --defsym _CLTEXTBHASH0=0x%lx"
		" --defsym _CLTEXTBHASH1=0x%lx",
		tfname,cltextshash[0], cltextshash[1], cltextbhash[0], cltextbhash[1] );
	printf("|%s|\n",cmd);
	system(cmd);

#ifndef CLCC_TEST
	unlink(tfname);
#endif
	
	return(0);

}


