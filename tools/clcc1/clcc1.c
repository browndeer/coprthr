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
#include <errno.h>

#include <CL/cl.h>
#define CL_CONTEXT_OFFLINE_DEVICES_AMD 0x403F
#define CL_CONTEXT_OFFLINE_DEVICES_COPRTHR 0x403F

#include "printcl.h"
#include "../../src/libclelf/clelf.h"

#include "coprthr_cc.h"
#include "coprthr.h"

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
	if (!strncmp(arg,flag,flaglen) && arglen > flaglen )
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

__inline
char* str_get_fused( char* arg, char* flag )
{ return arg + strlen(flag); }

__inline
char* str_get_seteq( char* arg, char* flag )
{ return arg + strlen(flag) + 1; }


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
		"Copyright (c) 2008-2012 Brown Deer Technology, LLC."
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


#define append_str(str1,str2,sep,n) __append_str(&str1,str2,sep,n)

void __append_str( char** pstr1, char* str2, char* sep, size_t n )
{
	if (!*pstr1 || !str2) return;

	size_t len = strlen(str2);

	if (sep) {
		*pstr1 = (char*)realloc(*pstr1,strlen(*pstr1)+len+2);
		strcat(*pstr1,sep);
	} else {
		*pstr1 = (char*)realloc(*pstr1,strlen(*pstr1)+len+1);
	}
	strncat(*pstr1,str2, ((n==0)?len:n) );

}


void list_add_str( char** pstr1, char* str2 )
{
  if (!str2) return;

  size_t len = strlen(str2);

  if (!(*pstr1)) {
    *pstr1 = (char*)malloc(len+1);
    strcpy(*pstr1,str2);
  } else {
    *pstr1 = (char*)realloc(*pstr1,strlen(*pstr1)+len+2);
    strcat(*pstr1,",");
    strcat(*pstr1,str2);
  }

}


void list_str_to_argv( char* str, int* argc, char*** argv )
{

  *argc = 0;

  if (str) {

    char* buf = (char*)malloc(strlen(str)+1);
    char* tmp_ptr;

    strcpy(buf,str);

      char* tok = strtok_r(buf, ",", &tmp_ptr);
    if (tok) ++(*argc);
      while (tok && (*argc) < 1024) {
         tok = strtok_r(0, ",", &tmp_ptr);
         if (tok) ++(*argc);
      }

    *argv = (char**)malloc((*argc)*sizeof(char*));

    int n = 0;
      tok = strtok_r(str, ",", &tmp_ptr);
      if (tok) (*argv)[n++] = tok;
      while (tok && n < 1024) {
         tok = strtok_r(0, ",", &tmp_ptr);
         if (tok) (*argv)[n++] = tok;
      }

  }

}


int main(int argc, char** argv)
{
	int i,j,k;
	int n;

	char default_platform[] = "platform-unknown";
	char default_device[] = "device-unknown";

	char* platform = default_platform;
	char* device = default_device;

	char* tmpdir;
   char* env_tmpdir = getenv("TMPDIR");
   tmpdir = (env_tmpdir)? strdup(env_tmpdir) : strdup("/tmp");

   printcl( CL_DEBUG "tmpdir=%s",tmpdir);

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
	int en_src = 1;
	int en_bin = 1;

	int en_src_only = 0;
	int en_bin_only = 0;

	int use_offline_devices = 0;

	int quiet = 1;

	int dump_bin = 0;
	int use_coprthr_cc = 0;
	char* target_str = 0;
	int en_report_targets = 0;

	char* platform_select = 0;
	char* platform_exclude = 0;
	char* device_select = 0;
	char* device_exclude = 0;


	/* XXX todo - add ability to provide lang specific options -DAR */

	char* fname = 0;
	char* ofname = 0;


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


		/* -s (source only)*/
    } else if (!strcmp(argv[n],"-s")) {

      en_src_only = 1;


    /* -b (binary only)*/
    } else if (!strcmp(argv[n],"-b")) {

      en_bin_only = 1;


		/* -mavail (use only available devices) */
		} else if (!strcmp(argv[n],"-mavail")) {

			use_offline_devices = 0;
		
	
		/* -mall (use all devices including offline) */
		} else if (!strcmp(argv[n],"-mall")) {

			use_offline_devices = 1;
			

		} else if (str_match_exact(argv[n],"-mplatform")) {

			list_add_str(&platform_select,argv[++n]);

		} else if (str_match_seteq(argv[n],"-mplatform")) {

			list_add_str(&platform_select,str_get_seteq(argv[n],"-mplatform"));

		} else if (str_match_exact(argv[n],"-mplatform-exclude")) {

			list_add_str(&platform_exclude,argv[++n]);

		} else if (str_match_seteq(argv[n],"-mplatform-exclude")) {

			list_add_str(&platform_exclude,
				str_get_seteq(argv[n],"-mplatform-exclude"));
			
		} else if (str_match_exact(argv[n],"-mdevice")) {

			list_add_str(&device_select,argv[++n]);
			use_offline_devices = 1;

		} else if (str_match_seteq(argv[n],"-mdevice")) {

			list_add_str(&device_select,str_get_seteq(argv[n],"-mdevice"));
			use_offline_devices = 1;
			
		} else if (str_match_exact(argv[n],"-mdevice-exclude")) {

			list_add_str(&device_exclude,argv[++n]);

		} else if (str_match_seteq(argv[n],"-mdevice-exclude")) {

			list_add_str(&device_exclude,
				str_get_seteq(argv[n],"-mdevice-exclude"));

		} else if (!strcmp(argv[n],"-mcpu")) {
			
		} else if (!strcmp(argv[n],"-mgpu")) {
			
		} else if (!strcmp(argv[n],"-mrpu")) {

//		} else if (str_match_seteq(argv[n],"-s")
//
//			||str_match_seteq(argv[n],"--no-src")) {
//			
//			en_src = 0;
//			
//		} else if (!strcmp(argv[n],"--no-bin")) {
//			
//			en_bin = 0;
//
//		} else if (str_match_exact(argv[n],"--no-source")) {
//
//			en_src = 0;
//
//		} else if (str_match_exact(argv[n],"--no-binary")) {
//
//			en_bin = 0;

		} else if (str_match_fused(argv[n],"-D")) {

			append_str(opt_str,argv[n]," ",0);

		} else if (str_match_exact(argv[n],"-D")) {

			append_str(opt_str,argv[n]," ",0);
			append_str(opt_str,argv[++n]," ",0);


		} else if (str_match_fused(argv[n],"-I")) {

			append_str(opt_str,"-I"," ",0);
			append_str(opt_str,realpath(argv[n]+2,0),0,0);

		} else if (str_match_exact(argv[n],"-I")) {

			append_str(opt_str,argv[n]," ",0);
			append_str(opt_str,realpath(argv[++n],0)," ",0);



		} else if (str_match_exact(argv[n],"-c")) {

		} else if (str_match_exact(argv[n],"-o")) {

			ofname = argv[++n];

		} else if (str_match_exact(argv[n],"-v")) {

			quiet = 0;

		} else if (str_match_exact(argv[n],"--dump-bin")) {

			dump_bin = 1;

		} else if (str_match_exact(argv[n],"--coprthr-cc")) {

			use_coprthr_cc = 1;

		} else if (str_match_seteq(argv[n],"-mtarget")) {

			target_str = str_get_seteq(argv[n],"-mtarget");
			
		} else if (str_match_exact(argv[n],"--targets")) {

			en_report_targets = 1;
			
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


	if (target_str && !use_coprthr_cc) {
		printcl( CL_ERR "-mtarget is only used with --coprthr_cc");
		exit(-1);
	} else if (en_report_targets && !use_coprthr_cc) {
		printcl( CL_ERR "--targets is only used with --coprthr_cc");
		exit(-1);
	} else if (en_report_targets && use_coprthr_cc) {
		char* log = 0;
		coprthr_cc(0,0,"--targets",&log);
		printf( "%s\n",log);
		exit(0);
	} else if (!target_str && use_coprthr_cc) {
		printcl( CL_ERR "--coprthr_cc requires -mtarget to select target device");
		exit(-1);
	}


	if (!ofname) {
		size_t fname_len = strlen(fname);
		char* fname_ext = strrchr(fname,'.');
		ofname = (char*)calloc(1,fname_len+1);
		if (fname_ext) strncpy(ofname,fname,(size_t)(fname_ext-fname));
		else strncpy(ofname,fname,fname_len);
		strncat(ofname,".o",fname_len);
	}


	if (en_src_only) { en_src=1; en_bin=0; }

	if (en_bin_only) { en_src=0; en_bin=1; }


  /***
   *** process platform and device select/exclude lists
   ***/

  int pselc = 0;
  char** pselv = 0;
  int pexclc = 0;
  char** pexclv = 0;

  int dselc = 0;
  char** dselv = 0;
  int dexclc = 0;
  char** dexclv = 0;

  list_str_to_argv(platform_select,&pselc,&pselv);
  list_str_to_argv(platform_exclude,&pexclc,&pexclv);

  list_str_to_argv(device_select,&dselc,&dselv);
  list_str_to_argv(device_exclude,&dexclc,&dexclv);

  for(i=0;i<pselc;i++) printcl( CL_DEBUG "platform select |%s|",pselv[i]);
  for(i=0;i<pexclc;i++) printcl( CL_DEBUG "platform exclude |%s|",pexclv[i]);
  for(i=0;i<dselc;i++) printcl( CL_DEBUG "device select |%s|",dselv[i]);
  for(i=0;i<dexclc;i++) printcl( CL_DEBUG "device exclude |%s|",dexclv[i]);

  int* pselcode = (int*)malloc(pselc*sizeof(int));
  int* pexclcode = (int*)malloc(pexclc*sizeof(int));

  for(i=0;i<pselc;i++) {

    pselcode[i] = 0;

      if (!strncasecmp(pselv[i],"AMD",3)) {

         pselcode[i] = CLELF_PLATFORM_CODE_AMDAPP;

      } else if (!strncasecmp(pselv[i],"Nvidia",6)) {

         pselcode[i] = CLELF_PLATFORM_CODE_NVIDIA;

      } else if (!strncasecmp(pselv[i],"coprthr",7)) {

         pselcode[i] = CLELF_PLATFORM_CODE_COPRTHR;

    } else if (!strncasecmp(pselv[i],"Intel",5)) {

       pselcode[i] = CLELF_PLATFORM_CODE_INTEL;

      }

  }

  for(i=0;i<pexclc;i++) {

    pexclcode[i] = 0;

      if (!strncasecmp(pexclv[i],"AMD",3)) {

         pexclcode[i] = CLELF_PLATFORM_CODE_AMDAPP;

      } else if (!strncasecmp(pexclv[i],"Nvidia",6)) {

         pexclcode[i] = CLELF_PLATFORM_CODE_NVIDIA;

      } else if (!strncasecmp(pexclv[i],"coprthr",7)) {

         pexclcode[i] = CLELF_PLATFORM_CODE_COPRTHR;

    	} else if (!strncasecmp(pexclv[i],"Intel",5)) {

       	pexclcode[i] = CLELF_PLATFORM_CODE_INTEL;

      }

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

	if (en_src) {

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

	}


	/***
	 *** compile program for each opencl platform
	 ***/

	int err;

	if (use_coprthr_cc) {

	int nbin_valid = 0;

	for(i=0; i<1; i++) {

		int platform_code = 0;

			platform_code = CLELF_PLATFORM_CODE_COPRTHR;

		size_t* bin_sizes = (size_t*)malloc( sizeof(size_t)*1 );

		printcl( CL_DEBUG "bin size %d",bin_sizes[0]);

		char** bins = (char**)malloc( sizeof(char*)*1 );

			if( bin_sizes[0] != 0 ) {
				bins[0] = (char*)malloc( sizeof(char)*bin_sizes[0] );
			} else {
				bins[0] = 0;
			}	

		printcl( CL_DEBUG "bin %p",bins[0]);

		char* opt = 0;
		char* log = 0;
		asprintf(&opt,"-mtarget=%s %s",target_str,opt_str);
		coprthr_program_t prg1 = coprthr_cc(file_ptr,file_sz,opt,&log);
		bins[0] = prg1->bin;
		bin_sizes[0] = prg1->bin_sz;
		printcl( CL_DEBUG "log=%p",log);
		printcl( CL_DEBUG "log='%s'",log);

		printcl( CL_DEBUG "bin %p",bins[0]);

		char* device_name = target_str;

		for( j=0; j < 1; j++ ) {

			cl_build_status status;
			status = (prg1)? CL_BUILD_SUCCESS : CL_BUILD_ERROR;

			printcl( CL_DEBUG "status %d",status);

			if (status == CL_BUILD_SUCCESS && bins[j]) {

				if (dump_bin) {

					printcl( CL_DEBUG 
						"dump_bin fname='%s' platform_code=%d device='%s' bin_sz=%ld",
						fname,platform_code,device_name,bin_sizes[j]);

					char* tmp_filename = (char*)malloc(256);
					snprintf(tmp_filename,256,"%sbin.%d.%s\0",
						fname,platform_code,device_name);

					int tmp_fd = open(tmp_filename,O_WRONLY|O_CREAT, 0644);
					write(tmp_fd,bins[j],bin_sizes[j]);
					close(tmp_fd);

				}


				++nbin_valid;
				printcl( CL_DEBUG "nbin_valid %d",nbin_valid);

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

						if (!quiet) printf("%s\n",log);
						printf("\n");

			} else if (status == CL_BUILD_NONE) {

				printf("clcc1: compile '%s' [%s:%s] FAILED\n",fname,
					platform_name_string[platform_code],device_name);

			} else if (status == CL_BUILD_ERROR || bins[j]==0) {	

				printf("clcc1: compile '%s' [%s:%s] FAILED\n",fname,
					platform_name_string[platform_code],device_name);

						printf("%s",log);
						printf("\n");

			}
			
		}

		cl_uint nkrn = 0;

/*
		err = clCreateKernelsInProgram (programs[i],0,0,&nkrn);
		cl_kernel* kernels = (cl_kernel*)malloc(nkrn*sizeof(cl_kernel));
		err = clCreateKernelsInProgram (programs[i],nkrn,kernels,0);
*/

		printcl( CL_DEBUG "nkrn %d",nkrn);

      while (data.clkrntab_n + nkrn >= data.clkrntab_nalloc) {
         data.clkrntab_nalloc += DEFAULT_CLKRNTAB_NALLOC;
         data.clkrntab = (struct clkrntab_entry*)
            realloc(data.clkrntab,__clkrntab_entry_sz*data.clkrntab_nalloc);
      }

/*
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
*/

	}
	
	} else if (en_bin) {

	cl_uint nplatforms;
	clGetPlatformIDs(0,0,&nplatforms);
	cl_platform_id* platforms
		= (cl_platform_id*)malloc(nplatforms*sizeof(cl_platform_id));
	clGetPlatformIDs(nplatforms,platforms,0);

	printf("number of platforms %d\n",nplatforms);

	cl_context* contexts = (cl_context*)malloc(nplatforms*sizeof(cl_context));

	cl_program* programs = (cl_program*)malloc(nplatforms*sizeof(cl_program));

	int nbin_valid = 0;

	for(i=0; i<nplatforms; i++) {

		char* opt_str2 = strdup(opt_str);

		char* info = malloc(1024);
		clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1024,info,0);

		int platform_code = 0;

		if (!strncasecmp(info,"AMD",3)) {

			platform_code = CLELF_PLATFORM_CODE_AMDAPP;
			append_str(opt_str2," -D __AMD__"," ",0);

		} else if (!strncasecmp(info,"Nvidia",6)) {

			platform_code = CLELF_PLATFORM_CODE_NVIDIA;
			append_str(opt_str2," -D __NVIDIA__"," ",0);

		} else if (!strncasecmp(info,"coprthr",7)) {

			platform_code = CLELF_PLATFORM_CODE_COPRTHR;
			append_str(opt_str2," -D __coprthr__"," ",0);

		} else if (!strncasecmp(info,"Intel",5)) {

			platform_code = CLELF_PLATFORM_CODE_INTEL;
			append_str(opt_str2," -D __INTEL__"," ",0);

		} else {

			continue;

		}

    int pselect = 1;
    int pexclude = 0;

    int l;

    if (pselv) {
       pselect = 0;
       for(l=0;l<pselc;l++)
           if (pselcode[l] == platform_code) pselect = 1;
    }

    if (pexclv) {
       for(l=0;l<pexclc;l++)
          if (pexclcode[l] == platform_code) pexclude = 1;
    }

    printcl( CL_DEBUG "se (%d %d)", pselect,pexclude);

		if (!pselect || pexclude) continue;


		cl_context_properties cprops[5];
		cprops[0] = CL_CONTEXT_PLATFORM;
		cprops[1] = (cl_context_properties)platforms[i];
		cprops[2] = (cl_context_properties)0;

		if (use_offline_devices) switch (platform_code) {

			case CLELF_PLATFORM_CODE_AMDAPP:
			case CLELF_PLATFORM_CODE_COPRTHR:
				cprops[2] = CL_CONTEXT_OFFLINE_DEVICES_AMD;
				cprops[3] = (cl_context_properties)1;
				cprops[4] = (cl_context_properties)0;
				break;

			default:
				break;

		}
			
//	contexts[i] = clCreateContextFromType(cprops,CL_DEVICE_TYPE_ALL,0,0,&err);
		cl_context tmpctx 
			= clCreateContextFromType(cprops,CL_DEVICE_TYPE_ALL,0,0,&err);
		cl_uint ndev0;
//		clGetDeviceIDs(platforms[i],CL_DEVICE_TYPE_ALL,0,0,&ndev0);
		clGetContextInfo(tmpctx,CL_CONTEXT_NUM_DEVICES,sizeof(cl_uint),&ndev0,0);
		cl_device_id* devices0 =(cl_device_id*)malloc(ndev0*sizeof(cl_device_id));
//		clGetDeviceIDs(platforms[i],CL_DEVICE_TYPE_ALL,ndev0,devices0,0);
		clGetContextInfo(tmpctx,CL_CONTEXT_DEVICES,
			ndev0*sizeof(cl_device_id),devices0,0);
//		clReleaseContext(tmpctx);

		printcl( CL_DEBUG "number of supported devices %d",ndev0);

		n = 0;
		for(j=0;j<ndev0;j++) {

			cl_device_type devtype;
			err = clGetDeviceInfo(devices0[j],CL_DEVICE_TYPE,sizeof(cl_device_type),
				&devtype,0);
			char devname[1024];
			err = clGetDeviceInfo(devices0[j],CL_DEVICE_NAME, 1024,devname,0);

      int dselect = 1;
      int dexclude = 0;

      int l;

      if (dselv) {
         dselect = 0;
         for(l=0;l<dselc;l++)
            if (!strcasecmp(dselv[l],devname)) dselect = 1;
      }

      if (dexclv) {
         for(l=0;l<dexclc;l++)
            if (!strcasecmp(dexclv[l],devname)) dexclude = 1;
      }

      printcl( CL_DEBUG "device |%s| se (%d %d)", devname,dselect,dexclude);

			if (dselect && !dexclude) devices0[n++] = devices0[j];
				
		}

		if (n==0) continue;

		contexts[i] = clCreateContext(cprops, n, devices0, 0, 0, &err );

		programs[i] = clCreateProgramWithSource(
			contexts[i], 1, (const char**)&file_ptr, &file_sz, &err );

//		err = clBuildProgram( programs[i], 0, 0, opt_str, 0, 0 );
		err = clBuildProgram( programs[i], 0, 0, opt_str2, 0, 0 );

		cl_uint ndev;
		err = clGetProgramInfo( programs[i], CL_PROGRAM_NUM_DEVICES,
			sizeof(cl_uint), &ndev, 0 );

		printcl( CL_DEBUG "ndev=%d",ndev);

		cl_device_id* devices = (cl_device_id*)malloc(ndev*sizeof(cl_device_id));

		err = clGetProgramInfo (programs[i],CL_PROGRAM_DEVICES,
			ndev*sizeof(cl_device_id),devices,0);


		size_t* bin_sizes = (size_t*)malloc( sizeof(size_t)*ndev );

		err = clGetProgramInfo( programs[i], CL_PROGRAM_BINARY_SIZES,
			sizeof(size_t)*ndev, bin_sizes, 0 );

		printcl( CL_DEBUG "bin size %d",bin_sizes[0]);

		char** bins = (char**)malloc( sizeof(char*)*ndev );

		for( j=0; j < ndev; j++ ) {
			if( bin_sizes[j] != 0 ) {
				bins[j] = (char*)malloc( sizeof(char)*bin_sizes[j] );
			} else {
				bins[j] = 0;
			}	
		}

		printcl( CL_DEBUG "bin %p",bins[0]);

		err = clGetProgramInfo( programs[i], CL_PROGRAM_BINARIES,
			sizeof(char*)*ndev, bins, 0 );

		printcl( CL_DEBUG "bin %p",bins[0]);

		for( j=0; j < ndev; j++ ) {

			if (bins[j]) {

			Elf32_Ehdr* ehdr32;
			Elf64_Ehdr* ehdr64;

			printcl( CL_DEBUG "platform code %d",platform_code);

			switch(platform_code) {

				case CLELF_PLATFORM_CODE_AMDAPP:
					ehdr32 = (Elf32_Ehdr*)bins[j];
					break;

				case CLELF_PLATFORM_CODE_COPRTHR:
					ehdr64 = (Elf64_Ehdr*)bins[j];
					printcl( CL_DEBUG "ehdr64 %p",ehdr64);
					break;

				case CLELF_PLATFORM_CODE_NVIDIA:
				default:
					printcl( CL_DEBUG "device code %d",0);
					break;

			}

			}

		}

		for( j=0; j < ndev; j++ ) {

			char device_name[1024];
			err = clGetDeviceInfo(devices[j],CL_DEVICE_NAME,1024,device_name,0);

			printcl( CL_DEBUG "device name %s",device_name);
			clelf_device_name_alias(device_name);
			printcl( CL_DEBUG "aliased device name %s",device_name);

			cl_build_status status;
			err = clGetProgramBuildInfo(programs[i],devices[j],
				CL_PROGRAM_BUILD_STATUS,sizeof(cl_build_status),&status,0);

			printcl( CL_DEBUG "status %d",status);

			if (status == CL_BUILD_SUCCESS && bins[j]) {

				if (dump_bin) {

printcl( CL_DEBUG "XXX fname='%s' platform_code=%d device='%s' bin_sz=%ld",
	fname,platform_code,device_name,bin_sizes[j]);

					char* tmp_filename = (char*)malloc(256);
					snprintf(tmp_filename,256,"%sbin.%d.%s\0",
						fname,platform_code,device_name);
					int tmp_fd = open(tmp_filename,O_WRONLY|O_CREAT,644);
					write(tmp_fd,bins[j],bin_sizes[j]);
					close(tmp_fd);
				}


				++nbin_valid;
				printcl( CL_DEBUG "nbin_valid %d",nbin_valid);

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

				char* build_log;
				size_t build_log_sz;
				err = clGetProgramBuildInfo(programs[i],devices[j],
					CL_PROGRAM_BUILD_LOG,0,0,&build_log_sz);
				if (build_log_sz) {
					build_log = (char*)malloc(build_log_sz);
					err = clGetProgramBuildInfo(programs[i],devices[j],
						CL_PROGRAM_BUILD_LOG,build_log_sz,build_log,0);
					if (build_log_sz > 1) {
						printf("%s\n",build_log);
						if (build_log[build_log_sz-2] != '\n') printf("\n");
						printf("\n");
					}
					free(build_log);
				}

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
				if (build_log_sz) {
					build_log = (char*)malloc(build_log_sz);
					err = clGetProgramBuildInfo(programs[i],devices[j],
						CL_PROGRAM_BUILD_LOG,build_log_sz,build_log,0);
					if (build_log_sz > 1) {
						printf("%s",build_log);
						if (build_log[build_log_sz-2] != '\n') printf("\n");
						printf("\n");
					}
					free(build_log);
				}

			}
			
		}

		cl_uint nkrn;

		err = clCreateKernelsInProgram (programs[i],0,0,&nkrn);
		cl_kernel* kernels = (cl_kernel*)malloc(nkrn*sizeof(cl_kernel));
		err = clCreateKernelsInProgram (programs[i],nkrn,kernels,0);

		printcl( CL_DEBUG "nkrn %d",nkrn);

      while (data.clkrntab_n + nkrn >= data.clkrntab_nalloc) {
         data.clkrntab_nalloc += DEFAULT_CLKRNTAB_NALLOC;
         data.clkrntab = (struct clkrntab_entry*)
            realloc(data.clkrntab,__clkrntab_entry_sz*data.clkrntab_nalloc);
      }

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

		free(info);
	
		free(opt_str2);
	}
	
	} // if (en_bin)


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

	unsigned int cltextsrchash[4];
	unsigned int cltextbinhash[4];

	size_t len = (intptr_t)(data.cltextsrc_bufp-data.cltextsrc_buf);
	clelf_md5((const unsigned char*)data.cltextsrc_buf,len,
		(unsigned char*)cltextsrchash);

	len = (intptr_t)(data.cltextbin_bufp-data.cltextbin_buf);
	clelf_md5((const unsigned char*)data.cltextbin_buf,len,
		(unsigned char*)cltextbinhash);


	/***
	 *** create special elf file
	 ***/

//	char tfname[] = "/tmp/clccXXXXXX";
//	char tfname[] = "clccXXXXXX";
	char* tfname;
   asprintf(&tfname,"%s/clccXXXXXX",tmpdir);
	fd = mkstemp(tfname);

	if (fd < 0) {
		fprintf(stderr,"clcc1: mkstemp failed");
		exit(-1);
	}

	clelf_write_file(fd,&data);

	close(fd);

	{
		struct stat fs;
		stat(tfname,&fs);
//		printf("clelf file size %d\n",fs.st_size);
	}

	char cmd[1024];

//#ifdef __LP64__
//	snprintf( cmd, 1024, "ld -r -o %s"
//		" %s"
//		" --defsym _CLTEXTSHASH0=0x%lx"
//		" --defsym _CLTEXTSHASH1=0x%lx"
//
//		" --defsym _CLTEXTBHASH0=0x%lx"
//		" --defsym _CLTEXTBHASH1=0x%lx",
//		ofname,
//		tfname,cltextsrchash[0], cltextsrchash[1], 
//		cltextbinhash[0], cltextbinhash[1] );
//#else
	snprintf( cmd, 1024, "ld -r -o %s"
		" %s"
		" --defsym _CLTEXTSHASH0=0x%x"
		" --defsym _CLTEXTSHASH1=0x%x"
		" --defsym _CLTEXTSHASH2=0x%x"
		" --defsym _CLTEXTSHASH3=0x%x"
		" --defsym _CLTEXTBHASH0=0x%x"
		" --defsym _CLTEXTBHASH1=0x%x"
		" --defsym _CLTEXTBHASH2=0x%x"
		" --defsym _CLTEXTBHASH3=0x%x",
		ofname, tfname,
		cltextsrchash[0], cltextsrchash[1], cltextsrchash[2], cltextsrchash[3], 
		cltextbinhash[0], cltextbinhash[1], cltextbinhash[2], cltextbinhash[3] );
//#endif

	system(cmd);

	printcl( CL_DEBUG "removing temp file '%s'",tfname);
	unlink(tfname);
	
	return(0);

}


