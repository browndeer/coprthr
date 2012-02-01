/* clld.c
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


//#define CLCC_TEST

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

//#include "_version.h"
#include "CL/cl.h"
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


void usage() 
{
	printf("usage: clld [options] file...\n");
	printf("options:\n");
	printf("\t-o <file>\n");
   printf("\t-v\n");
   printf("\t--help, -h\n");
   printf("\t--version, -v\n");
   printf("\t-mplatform=<platform-list>\n");
   printf("\t-mplatform-exclude=<platform-list>\n");
   printf("\t-mdevice=<device-list>\n");
   printf("\t-mdevice-exclude=<device-list>\n");
   printf("\t-mall\n");
   printf("\t-mavail\n");
}

void version()
{
	printf("BDT clld\n"); 
	printf(
		"Copyright (c) 2008-2011 Brown Deer Technology, LLC."
		" All Rights Reserved.\n"
	);
	printf("This program is free software distributed under GPLv3.\n");
}

void add_path( char* path_str, size_t* path_str_len, char* path )
{
	size_t len = strnlen(path,DEFAULT_STR_SIZE);

	if (*path_str_len + len + 2 > DEFAULT_STR_SIZE) {
		fprintf(stderr,"clld: error: path buffer overflow\n");
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

/*
void append_str( char* str1, char* str2 )
{
	if (strnlen(str1,DEFAULT_STR_SIZE) 
		+ strnlen(str2,DEFAULT_STR_SIZE) + 1 > DEFAULT_STR_SIZE
	) {
		fprintf(stderr,"clld: error: str buffer overflow\n");
		exit(-1);
	}

	strncat(str1," ",DEFAULT_STR_SIZE);
	strncat(str1,str2,DEFAULT_STR_SIZE);
}
*/


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


	char* path_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	path_str[0] = '.';
	size_t path_str_len = 2;

	char* def_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	def_str[0] = '\0';

	char* fopt_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	fopt_str[0] = '\0';

	struct clelf_data_struct data;
   clelf_init_data(&data);


	/* XXX we will always check if 128 is enough, it should be -DAR */
	unsigned int flist_alloc = 128;
	char** flist = (char**)calloc(flist_alloc,sizeof(char*));
	unsigned int flist_n = 0;

	char* fullpath = (char*)malloc(DEFAULT_STR_SIZE);

	char** argv1 = (char**)calloc(sizeof(char*),argc);
	int argc1 = 0;	

	int lang = LANG_OPENCL;
	int en_openmp = 0;
	int en_src = 1;
	int en_bin = 1;

  int en_src_only = 0;
  int en_bin_only = 0;

  int use_offline_devices = 0;

	int quiet = 1;

	char* platform_select = 0;
	char* platform_exclude = 0;
	char* device_select = 0;
	char* device_exclude = 0;

	char default_ofname[] = "out_clld.o";
	char* ofname = default_ofname;


	/* XXX todo - add ability to provide lang specific options -DAR */

	FILE* fp;

	n = 1;
	while (n < argc) {

    /* -s (source only)*/
    if (!strcmp(argv[n],"-s")) {

      en_src_only = 1;


    /* -b (binary only)*/
    } else if (!strcmp(argv[n],"-b")) {

      en_bin_only = 1;


    /* -mavail */
    } else if (!strcmp(argv[n],"-mavail")) {

			use_offline_devices = 0;


    /* -mall */
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

		} else if (str_match_seteq(argv[n],"-mdevice")) {

			list_add_str(&device_select,str_get_seteq(argv[n],"-mdevice"));
			
		} else if (str_match_exact(argv[n],"-mdevice-exclude")) {

			list_add_str(&device_exclude,argv[++n]);

		} else if (str_match_seteq(argv[n],"-mdevice-exclude")) {

			list_add_str(&device_exclude,
				str_get_seteq(argv[n],"-mdevice-exclude"));

		} else if (!strcmp(argv[n],"-mcpu")) {
			
		} else if (!strcmp(argv[n],"-mgpu")) {
			
		} else if (!strcmp(argv[n],"-mrpu")) {

		} else if (str_match_seteq(argv[n],"-s")
			||str_match_seteq(argv[n],"--no-src")) {
			
			en_src = 0;
			
		} else if (!strcmp(argv[n],"--no-bin")) {
			
			en_bin = 0;
			
		} else if (!strcmp(argv[n],"-o")) {

			ofname = argv[++n];

		} else if (!strcmp(argv[n],"-v")) {

			quiet = 0;

		} else if (str_match_exact(argv[n],"--help")) {

			usage(); 
			exit(0);

		} else if (!strcmp(argv[n],"--version")) {

			version(); 
			exit(0);


//		} else if (argv[n][0] == '-') {
//
//			append_str(fopt_str,argv[++i]);

		} else { /* nothing left, assume the arg is a filename */

			char* fname = argv[n];

			struct stat st;
			if ( stat(fname,&st) == -1 || !S_ISREG(st.st_mode)) {
				fprintf(stderr,"clld: '%s' no such file\n",fname);
				exit(-1);
			}

			char tmp[4];
			int fd = open(fname,O_RDONLY);
			if (fd == -1) {
				fprintf(stderr,"clld: '%s' open failed\n",fname);
				exit(-1);
			}
			read(fd,tmp,4);
			close(fd);

			if (tmp[0] != (char)0x7f || strncmp(tmp+1,"ELF",3)) {
				fprintf(stderr,"clld: '%s' is not an ELF object file\n",fname);
				exit(-1);
			}

			/* file is safe, push it to a list of files to link */

			/* not sure who will try to link more than 128, but we will check. */
			if (flist_n == flist_alloc) {
				flist_alloc += 128;
				flist = realloc(flist,flist_alloc);
				for(i=flist_n;i<flist_alloc;i++) flist[i] = 0;
			}
			flist[flist_n++] = fname;

			DEBUG2("pushed '%s' to the list",fname);

		}

		++n;

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

	for(i=0;i<pselc;i++) DEBUG2("platform select |%s|",pselv[i]);
	for(i=0;i<pexclc;i++) DEBUG2("platform exclude |%s|",pexclv[i]);
	for(i=0;i<dselc;i++) DEBUG2("device select |%s|",dselv[i]);
	for(i=0;i<dexclc;i++) DEBUG2("device exclude |%s|",dexclv[i]);

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

//    } else if (!strncasecmp(pselv[i],"Intel",7)) {
//
//       pselcode[i] = CLELF_PLATFORM_CODE_INTEL;

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

//    } else if (!strncasecmp(pexclv[i],"Intel",7)) {
//
//       pexclcode[i] = CLELF_PLATFORM_CODE_INTEL;

      }

	}


	/***
	 *** check the machine to determine available platforms and devices
	 ***/

   int err;

   cl_uint nplatforms;
   clGetPlatformIDs(0,0,&nplatforms);
   cl_platform_id* platforms
      = (cl_platform_id*)malloc(nplatforms*sizeof(cl_platform_id));
   clGetPlatformIDs(nplatforms,platforms,0);

   cl_context* contexts
      = (cl_context*)malloc(nplatforms*sizeof(cl_context));

   for(i=0; i<nplatforms; i++) {

      char info[1024];
      clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,1024,info,0);

//      int platform_code = 0;
//
//      if (!strncasecmp(info,"AMD",3)) {
//
//         platform_code = CLELF_PLATFORM_CODE_AMDAPP;
//
//      } else if (!strncasecmp(info,"Nvidia",6)) {
//
//         platform_code = CLELF_PLATFORM_CODE_NVIDIA;
//
//      } else if (!strncasecmp(info,"coprthr",7)) {
//
//         platform_code = CLELF_PLATFORM_CODE_COPRTHR;
//
//    } else if (!strncasecmp(info,"Intel",7)) {
//
//       platform_code = CLELF_PLATFORM_CODE_INTEL;
//
//      } else {
//
//         continue;
//
//      }
		int platform_code = clelf_platform_code(info);

		DEBUG2("available platform |%s|",platform_name_string[platform_code]);


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

      cl_context context 
			= clCreateContextFromType(cprops,CL_DEVICE_TYPE_ALL,0,0,&err);

		size_t dev_sz;
		err = clGetContextInfo(context,CL_CONTEXT_DEVICES,0,0,&dev_sz);
		cl_uint ndev = dev_sz / sizeof(cl_device_id);
		cl_device_id* devices = (cl_device_id*)malloc(ndev*sizeof(cl_device_id));
		err = clGetContextInfo(context,CL_CONTEXT_DEVICES,dev_sz,devices,0);

		DEBUG2("number of devices avail %d",ndev);

		for(j=0;j<ndev;j++) {

			cl_device_type devtype;
			err = clGetDeviceInfo(devices[j],CL_DEVICE_TYPE,
				sizeof(cl_device_type),&devtype,0);
			char devname[1024];
			err = clGetDeviceInfo(devices[j],CL_DEVICE_NAME,
            1024,devname,0);
			
			DEBUG2("available device |%s|",devname);

		}

		free(devices);

		clReleaseContext(context);

	}



	/***
	 *** begin loop over files
	 ***/
	
	int ifile;
	for(ifile=0;ifile<flist_n;ifile++) {

		char* fname = flist[ifile];

		DEBUG2("open file %d '%s'",ifile,fname);

		int fd = open(fname,O_RDONLY);
	
		struct stat st;
		stat(fname,&st);

		size_t file_sz = st.st_size;
		char* file_ptr = (char*)mmap(0,file_sz,PROT_READ,MAP_PRIVATE,fd,0);
		close(fd);
	

		struct clelf_sect_struct sect;
		bzero(&sect,sizeof(struct clelf_sect_struct));

		clelf_load_sections(file_ptr,&sect);

		int skip_file = 0;
		int bad_file = 0;

		/* now we check the various things that can be wrong */

		if (sect.has_any_clelf_section) {

			if (sect.has_any_clelf_section==5 && !sect.has_clprgtab) {

				WARN2(
					"'%s' missing .clprgtab section, possibly legacy format,"
					" file will be ignored",fname);
				skip_file = 1;

			} else if (sect.has_any_clelf_section < 6) {

				ERROR2("'%s' missing one or more clelf sections, exiting",fname);
				bad_file = 1;

			}

			if (!sect.has_clstrtab) {

				ERROR2("'%s' missing .clstrtab section, bad format, exiting",fname);
				bad_file = 1;

			}

			if (!sect.has_text) {

				WARN2(
					"'%s' no content in .cltextsrc or .cltestb sections,"
					" file will be ignored",fname);
				skip_file = 1;

			}

			int bad_hash = clelf_check_hash( file_ptr, &sect);

			if (bad_hash == 1) {

				ERROR2("'%s' missing CLELF hash symbols",fname);

				bad_file = 1;

			} else if (bad_hash == 2) {

				ERROR2("'%s' CLELF hash symbol mismatch",fname);

				bad_file = 1;

			}

		} else {

			skip_file = 1;

		}

		DEBUG2("skip_file bad_file %d %d",skip_file,bad_file);

		if (skip_file) continue;
		else if (bad_file) exit(-2);


		/* if we make it here, we have a good file to link */


		/***
		 *** add clprgtab,clkrntab,clprgbin,cltextbin entries - adjusting offsets
		 ***/

   	while (data.clprgtab_n + sect.clprgtab_n >= data.clprgtab_nalloc) {
     		data.clprgtab_nalloc += DEFAULT_CLPRGTAB_NALLOC;
     		data.clprgtab = (struct clprgtab_entry*) 
         	realloc(data.clprgtab,__clprgtab_entry_sz*data.clprgtab_nalloc);
   	}

   	while (data.clkrntab_n + sect.clkrntab_n >= data.clkrntab_nalloc) {
     		data.clkrntab_nalloc += DEFAULT_CLKRNTAB_NALLOC;
     		data.clkrntab = (struct clkrntab_entry*) 
         	realloc(data.clkrntab,__clkrntab_entry_sz*data.clkrntab_nalloc);
   	}

   	while (data.clprgsrc_n + sect.clprgsrc_n >= data.clprgsrc_nalloc) {
  	    	data.clprgsrc_nalloc += DEFAULT_CLPRGSRC_NALLOC;
 	   	data.clprgsrc = (struct clprgsrc_entry*)
 	      	realloc(data.clprgsrc,__clprgsrc_entry_sz*data.clprgsrc_nalloc);
 	  	}

   	while (data.clprgbin_n + sect.clprgbin_n >= data.clprgbin_nalloc) {
  	   	data.clprgbin_nalloc += DEFAULT_CLPRGBIN_NALLOC;
 	     	data.clprgbin = (struct clprgbin_entry*)
 	     		realloc(data.clprgbin,__clprgbin_entry_sz*data.clprgbin_nalloc);
 	  	}

/*
		size_t offset = clstrtab_strp-clstrtab_str;
   	while (offset + sect.clprgbin_n >= clstrtab_str_alloc) {
  	   	clstrtab_str_alloc += DEFAULT_STR_SIZE;
 	     	clstrtab_str = (char*)realloc(clstrtab_str,clstrtab_str_alloc);
			clstrtab_strp = clstrtab_str + offset;
 	  	}
*/

		for(i=0; i<sect.clprgtab_n; i++) {

   		data.clprgtab[data.clprgtab_n].e_name = (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
			char* name = sect.clstrtab + sect.clprgtab[i].e_name;
			add_strtab(data.clstrtab_str,data.clstrtab_strp,data.clstrtab_str_alloc,name);
   		data.clprgtab[data.clprgtab_n].e_info = sect.clprgtab[i].e_info;
   		data.clprgtab[data.clprgtab_n].e_prgsrc = sect.clprgtab[i].e_prgsrc + data.clprgsrc_n;
   		data.clprgtab[data.clprgtab_n].e_nprgsrc = sect.clprgtab[i].e_nprgsrc;
   		data.clprgtab[data.clprgtab_n].e_prgbin = data.clprgbin_n;
			/* defer clprgtab[clprgtab_n].e_nprgbin */
   		data.clprgtab[data.clprgtab_n].e_krn = data.clkrntab_n;
   		data.clprgtab[data.clprgtab_n].e_nkrn = sect.clprgtab[i].e_nkrn;

			int nprgbin = 0;

			if (en_bin) for(j=0; j<sect.clprgtab[i].e_nprgbin; j++) {

				int k = sect.clprgtab[i].e_prgbin + j;
		
				char* name = sect.clstrtab + sect.clprgbin[k].e_name;
				int platform_code = sect.clprgbin[k].e_platform;
				char* device_name = sect.clstrtab + sect.clprgbin[k].e_device;

				int pselect = 1;
				int pexclude = 0;
				int dselect = 1;
				int dexclude = 0;

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

				if (dselv) {
					dselect = 0;
					for(l=0;l<dselc;l++) 
						if (!strcasecmp(dselv[l],device_name)) dselect = 1;
				}

				if (dexclv) {
					for(l=0;l<dexclc;l++) 
						if (!strcasecmp(dexclv[l],device_name)) dexclude = 1;
				}

				int select = (pselect==1 && dselect==1)? 1 : 0;
				int exclude = (pexclude==1 || dexclude==1)? 1 : 0;

				DEBUG2("se (%d %d) (%d %d) (%d %d)",
					pselect,pexclude,dselect,dexclude,select,exclude);


				size_t alias_offset = 0;

				unsigned int n;

				for(n=data.clprgbin_n - nprgbin; n<data.clprgbin_n; n++) {

					char* name = data.clstrtab_str + data.clprgbin[n].e_device;
					int code = data.clprgbin[n].e_platform;

					if (!strcasecmp(device_name,name) && platform_code == code) {
						exclude = 1;
						DEBUG2("exclude bin for identical device |%s|%s|",
							device_name,name);
						break;
					}

					size_t offset = data.clprgbin[n].e_offset;
					size_t size = data.clprgbin[n].e_size;
					char* p = data.cltextbin_buf + offset;

					if (sect.clprgbin[k].e_size != size) continue;

					if (!memcmp(sect.cltextbin + sect.clprgbin[k].e_offset,p,size)) {
						alias_offset = offset;
						DEBUG2("alias bin for device |%s|%s|",
							device_name,name);
						break;
					}
						
				}

				if (select && !exclude) {

					if (!quiet) printf("clld: '%s' bin [%s:%s]\n",name,
						platform_name_string[platform_code],device_name);
	
   				data.clprgbin[data.clprgbin_n].e_name 
						= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
					add_strtab(data.clstrtab_str,data.clstrtab_strp,
						data.clstrtab_str_alloc,name);

   				data.clprgbin[data.clprgbin_n].e_info = sect.clprgbin[k].e_info;

   				data.clprgbin[data.clprgbin_n].e_platform = platform_code;

   				data.clprgbin[data.clprgbin_n].e_device 
						= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
					add_strtab(data.clstrtab_str,data.clstrtab_strp,
                  data.clstrtab_str_alloc,device_name);

   				data.clprgbin[data.clprgbin_n].e_shndx = -1;

					if (alias_offset) {

	   				data.clprgbin[data.clprgbin_n].e_offset = alias_offset;

						data.clprgbin[data.clprgbin_n].e_size
							= sect.clprgbin[k].e_size;

					} else {

	   				data.clprgbin[data.clprgbin_n].e_offset 
							= (intptr_t)(data.cltextbin_bufp-data.cltextbin_buf);

  	 					data.clprgbin[data.clprgbin_n].e_size 
							= sect.clprgbin[k].e_size;

//   					++data.clprgbin_n;

						size_t offset 
							= (intptr_t)(data.cltextbin_bufp-data.cltextbin_buf);

						size_t s = offset + sect.clprgbin[k].e_size;

						while (s > data.cltextbin_buf_alloc) {

     						data.cltextbin_buf_alloc += DEFAULT_BUF_ALLOC;

     						data.cltextbin_buf = realloc(data.cltextbin_buf,
								data.cltextbin_buf_alloc);

     						data.cltextbin_bufp = data.cltextbin_buf + offset;

   					}

						memcpy(data.cltextbin_bufp,
							sect.cltextbin + sect.clprgbin[k].e_offset,
							sect.clprgbin[k].e_size);

         			data.cltextbin_bufp += sect.clprgbin[k].e_size;

					}

   				++data.clprgbin_n;
					++nprgbin;

				}

			}

   		data.clprgtab[data.clprgtab_n].e_nprgbin = nprgbin;


			for(j=0; j<sect.clprgtab[i].e_nkrn; j++) {

				int k = sect.clprgtab[i].e_krn + j;

   			data.clkrntab[data.clkrntab_n].e_name 
					= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
				char* kname = sect.clstrtab + sect.clkrntab[k].e_name;
				add_strtab(data.clstrtab_str,data.clstrtab_strp,
					data.clstrtab_str_alloc,kname);
   			data.clkrntab[data.clkrntab_n].e_info = sect.clkrntab[k].e_info;
   			data.clkrntab[data.clkrntab_n].e_prg = data.clprgtab_n;
   			++data.clkrntab_n;

				if (!quiet) printf("clld: '%s' ksym %s\n",name,kname);
			}

   		++data.clprgtab_n;

		}


   	/***
   	 *** add clprgsrc entries and copy cltextsrc adjusting offsets as ncessary
   	 ***/

		if (en_src) for(i=0; i<sect.clprgsrc_n; i++) {

   		data.clprgsrc[data.clprgsrc_n].e_name 
				= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
			char* name = sect.clstrtab + sect.clprgsrc[i].e_name;
			add_strtab(data.clstrtab_str,data.clstrtab_strp,
				data.clstrtab_str_alloc,name);
   		data.clprgsrc[data.clprgsrc_n].e_info = sect.clprgsrc[i].e_info;
   		data.clprgsrc[data.clprgsrc_n].e_platform = sect.clprgsrc[i].e_platform;
   		data.clprgsrc[data.clprgsrc_n].e_device = sect.clprgsrc[i].e_device;
   		data.clprgsrc[data.clprgsrc_n].e_shndx = -1;
			data.clprgsrc[data.clprgsrc_n].e_offset 
				= (intptr_t)(data.cltextsrc_bufp-data.cltextsrc_buf);
   		data.clprgsrc[data.clprgsrc_n].e_size = sect.clprgsrc[i].e_size;
   		++data.clprgsrc_n;

			if (!quiet) printf("clld: '%s' src [<generic>]\n",name);

			size_t offset = (intptr_t)(data.cltextsrc_bufp-data.cltextsrc_buf);
			while (offset + sect.clprgsrc[i].e_size > data.cltextsrc_buf_alloc) {
  	   		data.cltextsrc_buf_alloc += DEFAULT_BUF_ALLOC;
  	   		data.cltextsrc_buf = realloc(data.cltextsrc_buf,data.cltextsrc_buf_alloc);
  	   		data.cltextsrc_bufp = data.cltextsrc_buf + offset;
  		 	}

   		memcpy(data.cltextsrc_bufp, sect.cltextsrc + sect.clprgsrc[i].e_offset, 
				sect.clprgsrc[i].e_size);

	  		data.cltextsrc_bufp += sect.clprgsrc[i].e_size;

		}

	}


	/***
	 *** calculate md5 has for cltextsrc and cltextbin
	 ***/

	unsigned long cltextsrchash[2];
	unsigned long cltextbinhash[2];


	size_t len = (intptr_t)data.cltextsrc_bufp-(intptr_t)data.cltextsrc_buf;
	if (len>0) {
		MD5((const unsigned char*)data.cltextsrc_buf, len, 
			(unsigned char*)cltextsrchash);
		DEBUG2("%lx %lx",cltextsrchash[0],cltextsrchash[1]);
	}

	len = (intptr_t)data.cltextbin_bufp-(intptr_t)data.cltextbin_buf;
	if (len>0) { 
		MD5((const unsigned char*)data.cltextbin_buf, len, 
			(unsigned char*)cltextbinhash);
		DEBUG2("%lx %lx",cltextbinhash[0],cltextbinhash[1]);
	}


	char tfname[] = "/tmp/clldXXXXXX";
	int fd = mkstemp(tfname);

	if (fd < 0) {
		fprintf(stderr,"clld: mkstemp failed");
		exit(-1);
	}

	
	clelf_write_file(fd,&data);

	close(fd);

	char cmd[1024];

	if (en_src && en_bin) {
		snprintf( cmd, 1024, "ld -r -o %s"
			" %s"
			" --defsym _CLTEXTSHASH0=0x%lx"
			" --defsym _CLTEXTSHASH1=0x%lx"
			" --defsym _CLTEXTBHASH0=0x%lx"
			" --defsym _CLTEXTBHASH1=0x%lx",
			ofname, tfname,
			cltextsrchash[0], cltextsrchash[1], cltextbinhash[0], cltextbinhash[1] );
	} else if (en_src) {
		snprintf( cmd, 1024, "ld -r -o %s"
			" %s"
			" --defsym _CLTEXTSHASH0=0x%lx"
			" --defsym _CLTEXTSHASH1=0x%lx",
			ofname, tfname,
			cltextsrchash[0], cltextsrchash[1] );
	} else if (en_bin) {
		snprintf( cmd, 1024, "ld -r -o %s"
			" %s"
			" --defsym _CLTEXTBHASH0=0x%lx"
			" --defsym _CLTEXTBHASH1=0x%lx",
			ofname, tfname,
			cltextbinhash[0], cltextbinhash[1] );
	} else {
		snprintf( cmd, 1024, "ld -r -o %s"
			" %s",
			ofname, tfname);
	}

	DEBUG2("%s",cmd);
	system(cmd);

#ifndef CLCC_TEST
	unlink(tfname);
#endif
	
	return(0);

}

