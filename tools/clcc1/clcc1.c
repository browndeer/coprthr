/* clcc1.c
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

#include "util.h"
#include "../../src/libclelf/clelf.h"

char* platform_name_string[] = {
      "unknown",
      "amdapp",
      "nvidia",
      "coprthr",
      "intel"
};

__inline
int str_match_exact( char* arg, char* flag )
{
   if (!strcmp(arg,flag))
      return 1;
   else
      return 0;
}

__inline
int str_match_fused( char* arg, char* flag )
{
   size_t arglen = strlen(arg);
   size_t flaglen = strlen(flag);
   if (!strncmp(arg,flag,flaglen) && arglen > flaglen && arg[flaglen] == '=' )
      return 1;
   else
      return 0;
}
__inline
int str_match_seteq( char* arg, char* flag )
{
   size_t arglen = strlen(arg);
   size_t flaglen = strlen(flag);
   if (!strncmp(arg,flag,flaglen) && arglen > flaglen && arg[flaglen] == '=' )
      return 1;
   else
      return 0;
}


void usage() 
{
	printf("usage: clcc1 [options] file\n");
	printf("options:\n");
	printf("\t-c\n");
   printf("\t-x <language>\n");
   printf("\t-o <file>\n");
   printf("\t-I <directory>\n");
   printf("\t-v\n");
   printf("\t--help, -h\n");
   printf("\t--version, -v\n");
   printf("\t-fopencl\n");
   printf("\t-fstdcl\n");
   printf("\t-fcuda\n");
   printf("\t-fopenmp\n");
   printf("\t-mplatform=<platform-list>\n");
   printf("\t-mplatform-exclude=<platform-list>\n");
   printf("\t-mdevice=<device-list>\n");
   printf("\t-mdevice-exclude=<device-list>\n");
   printf("\t-mall\n");
   printf("\t-mavail\n");
}

void version()
{
	printf("COPRTHR clcc1\n"); 
	printf(
		"Copyright (c) 2008-2011 Brown Deer Technology, LLC."
		" All Rights Reserved.\n"
	);
	printf("This program is free software distributed under GPLv3.\n");
}



void add_path( char* path_str, size_t* path_str_len, char* path )
{
	size_t len = strnlen(path,1024);

	if (*path_str_len + len + 2 > DEFAULT_STR_SIZE) {
		fprintf(stderr,"clcc1: error: path buffer overflow\n");
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


void append_str( char* str1, char* str2, char* sep, size_t n )
{
   if (!str1 || !str2) return;

   size_t len = strlen(str2);

   if (sep) {
      str1 = (char*)realloc(str1,strlen(str1)+len+2);
      strcat(str1,sep);
   } else {
      str1 = (char*)realloc(str1,strlen(str1)+len+1);
   }
   strncat(str1,str2, ((n==0)?len:n) );

}


int main(int argc, char** argv)
{
	int i,j,k;
	int n;

	char default_platform[] = "platform-unknown";
	char default_device[] = "device-unknown";

	char* platform = default_platform;
	char* device = default_device;

	char* path_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	path_str[0] = '.';
	size_t path_str_len = 2;

	char* opt_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	opt_str[0] = '\0';

	struct clelf_data_struct data;
	clelf_init_data(&data);	

	char* fullpath = (char*)malloc(DEFAULT_STR_SIZE);

   int lang = LANG_OPENCL;
   int en_openmp = 0;
   int en_source = 1;
   int en_binary = 1;

   /* XXX todo - add ability to provide lang specific options -DAR */


	char* fname = 0;
	char* ofname = 0;

	int quiet = 0;

printf("\nXXX add option to use only devices that are present\n\n");

	n = 1;
	while (n < argc) {

      if (str_match_exact(argv[n],"-fopencl")) {

         lang = LANG_OPENCL;

      } else if (str_match_exact(argv[n],"-fstdcl")) {

         lang = LANG_STDCL;

      } else if (str_match_exact(argv[n],"-fcuda")) {

         lang = LANG_CUDA;

      } else if (str_match_exact(argv[n],"-fopenmp")) {

         en_openmp = 1;

      } else if (str_match_exact(argv[n],"--no-source")) {

         en_source = 0;

      } else if (str_match_exact(argv[n],"--no-binary")) {

         en_binary = 0;

		} else if (str_match_fused(argv[n],"-D")) {

			append_str(opt_str,argv[n]," ",0);

		} else if (str_match_fused(argv[n],"-I")) {

			append_str(opt_str,argv[n]," ",0);

		} else if (str_match_exact(argv[n],"-c")) {

		} else if (str_match_exact(argv[n],"-o")) {

			ofname = argv[++n];

		} else if (str_match_exact(argv[n],"-v")) {

			quiet = 0;

		} else if (str_match_exact(argv[n],"--help")) {

			usage(); 
			exit(0);

		} else if (!strcmp(argv[n],"--version")) {

			version(); 
			exit(0);

		} else { /* nothing left, assume the arg is a filename */

			if (fname) {
				ERROR2("clcc1 called with multiple input files");
				exit(-1);
			}

         fname = argv[n];

		}

		++n;

	}


	if (!ofname) {
		size_t fname_len = strlen(fname);
		char* fname_ext = strrchr(fname,'.');
		ofname = (char*)calloc(1,fname_len+1);
		if (fname_ext) strncpy(ofname,fname,(size_t)(fname_ext-fname));
      else strncpy(ofname,fname,fname_len);
		strncat(ofname,".o",fname_len);
	}


	/***
	 *** map source file
	 ***/

   struct stat st;
   if ( stat(fname,&st) == -1 || !S_ISREG(st.st_mode)) {
   fprintf(stderr,"clcc1: '%s' no such file\n",fname);
      exit(-1);
   }

	int fd = open(fname,O_RDONLY);

	if (fd < 0) {
      fprintf(stderr,"clcc1: '%s' open failed\n",fname);
      exit(-1);
   }

   size_t file_sz = st.st_size;
   void* file_ptr = mmap(0,file_sz,PROT_READ,MAP_PRIVATE,fd,0);

   close(fd);



	/***
	 *** add clprgsrc entry
	 ***/

	if (data.clprgsrc_n >= data.clprgsrc_nalloc) {
		data.clprgsrc_nalloc += DEFAULT_CLPRGSRC_NALLOC;
		data.clprgsrc = (struct clprgsrc_entry*)
			realloc(data.clprgsrc,__clprgsrc_entry_sz*data.clprgsrc_nalloc);
	}

	data.clprgsrc[data.clprgsrc_n].e_name 
		= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
	add_strtab(data.clstrtab_str,data.clstrtab_strp,
		data.clstrtab_str_alloc, fname);
	data.clprgsrc[data.clprgsrc_n].e_info = 0;
	data.clprgsrc[data.clprgsrc_n].e_platform = 0;
	data.clprgsrc[data.clprgsrc_n].e_device = 0;
	data.clprgsrc[data.clprgsrc_n].e_shndx = -1;
	data.clprgsrc[data.clprgsrc_n].e_offset 
		= (intptr_t)(data.cltextsrc_bufp-data.cltextsrc_buf);
	data.clprgsrc[data.clprgsrc_n].e_size = st.st_size;
	++data.clprgsrc_n;

	size_t offset = (intptr_t)data.cltextsrc_bufp-(intptr_t)data.cltextsrc_buf;

	while (offset+file_sz>data.cltextsrc_buf_alloc) {
		data.cltextsrc_buf_alloc += DEFAULT_BUF_ALLOC;
		data.cltextsrc_buf = realloc(data.cltextsrc_buf,data.cltextsrc_buf_alloc);
		data.cltextsrc_bufp = data.cltextsrc_buf + offset;
	}

	memcpy(data.cltextsrc_bufp,file_ptr,file_sz);

	data.cltextsrc_bufp += file_sz;



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
//   	printf("|%s|\n",info);

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
			
//		contexts[i] = clCreateContextFromType(cprops,CL_DEVICE_TYPE_ALL,0,0,&err);	
		contexts[i] = clCreateContextFromType(cprops,CL_DEVICE_TYPE_CPU,0,0,&err);	
		programs[i] = clCreateProgramWithSource(
   		contexts[i], 1, (const char**)&file_ptr, &file_sz, &err );

		err = clBuildProgram( programs[i], 0, 0, opt_str, 0, 0 );

		cl_uint ndev;
		err = clGetProgramInfo( programs[i], CL_PROGRAM_NUM_DEVICES,
   		sizeof(cl_uint), &ndev, 0 );

printf("ndev=%d\n",ndev);

		cl_device_id* devices = (cl_device_id*)malloc(ndev*sizeof(cl_device_id));

		err = clGetProgramInfo (programs[i],CL_PROGRAM_DEVICES,
			ndev*sizeof(cl_device_id),devices,0);


		size_t* bin_sizes = (size_t*)malloc( sizeof(size_t)*ndev );

		err = clGetProgramInfo( programs[i], CL_PROGRAM_BINARY_SIZES,
   		sizeof(size_t)*ndev, bin_sizes, 0 );

printf("bin size %d\n",bin_sizes[0]);

		char** bins = (char**)malloc( sizeof(char*)*ndev );

		for( j=0; j < ndev; j++ ) {
		   if( bin_sizes[j] != 0 ) {
      		bins[j] = (char*)malloc( sizeof(char)*bin_sizes[j] );
   		} else {
      		bins[j] = 0;
   		}	
		}

		err = clGetProgramInfo( programs[i], CL_PROGRAM_BINARIES,
   		sizeof(char*)*ndev, bins, 0 );

printf("bin %p\n",bins[0]);

		for( j=0; j < ndev; j++ ) {

			if (bins[j]) {

			Elf32_Ehdr* ehdr32;
			Elf64_Ehdr* ehdr64;
printf("platform code %d\n",platform_code);
			switch(platform_code) {

				case CLELF_PLATFORM_CODE_AMDAPP:
					ehdr32 = (Elf32_Ehdr*)bins[j];
					break;

				case CLELF_PLATFORM_CODE_COPRTHR:
					ehdr64 = (Elf64_Ehdr*)bins[j];
printf("ehdr64 %p\n",ehdr64);
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

printf("device name %s\n",device_name);
clelf_device_name_alias(device_name);
printf("device name %s\n",device_name);

			cl_build_status status;
			err = clGetProgramBuildInfo(programs[i],devices[j],
				CL_PROGRAM_BUILD_STATUS,sizeof(cl_build_status),&status,0);

printf("status %d\n",status);

			if (status == CL_BUILD_SUCCESS && bins[j]) {

				++nbin_valid;
printf("nbin_valid %d\n",nbin_valid);

				/***
				 *** add clprgbin entry
				 ***/

				if (data.clprgbin_n >= data.clprgbin_nalloc) {
					data.clprgbin_nalloc += DEFAULT_CLPRGBIN_NALLOC;
					data.clprgbin = (struct clprgbin_entry*)
					realloc(data.clprgbin,__clprgbin_entry_sz*data.clprgbin_nalloc);
				}

				data.clprgbin[data.clprgbin_n].e_name 
					= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
				add_strtab(data.clstrtab_str,data.clstrtab_strp,
					data.clstrtab_str_alloc,fname);
				data.clprgbin[data.clprgbin_n].e_info = 0;
				data.clprgbin[data.clprgbin_n].e_platform = platform_code;
				data.clprgbin[data.clprgbin_n].e_device 
					= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
				add_strtab(data.clstrtab_str,data.clstrtab_strp,
					data.clstrtab_str_alloc, device_name);
				data.clprgbin[data.clprgbin_n].e_shndx = -1;
				data.clprgbin[data.clprgbin_n].e_offset 
					= (intptr_t)(data.cltextbin_bufp-data.cltextbin_buf);
				data.clprgbin[data.clprgbin_n].e_size = bin_sizes[j];
				++data.clprgbin_n;

				size_t offset = (intptr_t)(data.cltextbin_bufp-data.cltextbin_buf);
				size_t n = bin_sizes[j];

				while (offset+n > data.cltextbin_buf_alloc) {
					data.cltextbin_buf_alloc += DEFAULT_BUF_ALLOC;
					data.cltextbin_buf 
						= realloc(data.cltextbin_buf,data.cltextbin_buf_alloc);
					data.cltextbin_bufp = data.cltextbin_buf + offset;
				}

				memcpy(data.cltextbin_bufp,bins[j],bin_sizes[j]);

				data.cltextbin_bufp += bin_sizes[j];

				printf("clcc1: compile '%s' [%s:%s]\n",fname,
					platform_name_string[platform_code],device_name);

			} else if (status == CL_BUILD_NONE) {

				printf("clcc1: compile '%s' [%s:%s] FAILED\n",fname,
					platform_name_string[platform_code],device_name);

			} else if (status == CL_BUILD_ERROR || bins[j]==0) {	

				printf("clcc1: compile '%s' [%s:%s] FAILED\n",info,fname,
					platform_name_string[platform_code],device_name);

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

printf("nkrn %d\n",nkrn);

		for(j=0;j<nkrn;j++) {

			char name[1024];
			err = clGetKernelInfo(kernels[j],CL_KERNEL_FUNCTION_NAME,1024,name,0);

			data.clkrntab[data.clkrntab_n].e_name 
				= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
			add_strtab(data.clstrtab_str,data.clstrtab_strp,
				data.clstrtab_str_alloc,name);

			data.clkrntab[data.clkrntab_n].e_info = 0;
			data.clkrntab[data.clkrntab_n].e_prg = 0;
			++data.clkrntab_n;

			clReleaseKernel(kernels[j]);
		}

	}



	/***
	 *** add clprgtab entry
	 ***/

	if (data.clprgtab_n >= data.clprgtab_nalloc) {
		data.clprgtab_nalloc += DEFAULT_CLPRGTAB_NALLOC;
		data.clprgtab = (struct clprgtab_entry*)
			realloc(data.clprgtab,__clprgtab_entry_sz*data.clprgtab_nalloc);
	}

	/* XXX right now we assume krntab/prgsrc/prgbin start is 0	here -DAR */

	data.clprgtab[data.clprgtab_n].e_name 
		= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
	add_strtab(data.clstrtab_str,data.clstrtab_strp,
		data.clstrtab_str_alloc,fname);
	data.clprgtab[data.clprgtab_n].e_info = 0;
	data.clprgtab[data.clprgtab_n].e_prgsrc = 0;
	data.clprgtab[data.clprgtab_n].e_nprgsrc = data.clprgsrc_n;
	data.clprgtab[data.clprgtab_n].e_prgbin = 0;
	data.clprgtab[data.clprgtab_n].e_nprgbin = data.clprgbin_n;
	data.clprgtab[data.clprgtab_n].e_krn = 0;
	data.clprgtab[data.clprgtab_n].e_nkrn = data.clkrntab_n;
	++data.clprgtab_n;

	unsigned long cltextsrchash[2];
	unsigned long cltextbinhash[2];

	size_t len = (intptr_t)(data.cltextsrc_bufp-data.cltextsrc_buf);
	MD5((const unsigned char*)data.cltextsrc_buf,len,
		(unsigned char*)cltextsrchash);

	len = (intptr_t)(data.cltextbin_bufp-data.cltextbin_buf);
	MD5((const unsigned char*)data.cltextbin_buf,len,
		(unsigned char*)cltextbinhash);



	/***
	 *** create special elf file
	 ***/

	char tfname[] = "/tmp/clccXXXXXX";
//	char tfname[] = "clccXXXXXX";
	fd = mkstemp(tfname);

	if (fd < 0) {
		fprintf(stderr,"clcc1: mkstemp failed");
		exit(-1);
	}

	clelf_write_file(fd,&data);

	close(fd);

	char cmd[1024];

//	snprintf( cmd, 1024, "ld -r -o out_clcc.o"
	snprintf( cmd, 1024, "ld -r -o %s"
		" %s"
		" --defsym _CLTEXTSHASH0=0x%lx"
		" --defsym _CLTEXTSHASH1=0x%lx"
		" --defsym _CLTEXTBHASH0=0x%lx"
		" --defsym _CLTEXTBHASH1=0x%lx",
		ofname,
		tfname,cltextsrchash[0], cltextsrchash[1], 
		cltextbinhash[0], cltextbinhash[1] );
//	printf("|%s|\n",cmd);
	system(cmd);

#ifndef CLCC_TEST
	unlink(tfname);
#endif
	
	return(0);

}


