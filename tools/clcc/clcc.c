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
//#include "util.h"
#include "printcl.h"
#include "../../src/libclelf/clelf.h"

/***
 *** C compiler front-end
 *** .cl passed as clcc1(-fopencl)->clld
 *** .c 	passed as clcc1(-fstdcl)->clld
 *** .cu passed as clcc1(-fcuda)->clld
 *** -c is presently implied for clcc
 ***/ 

#define TFNAME_TEMPLATE "/tmp/clccXXXXXX"
#define DEFAULT_OFNAME "out_clcc.o"

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


char* platform_name_string[] = { 
		"unknown", 
		"amdapp", 
		"nvidia", 
		"coprthr", 
		"intel" 
};

void usage() 
{
	printf("usage: clcc [options] file...\n");
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
	printf("COPRTHR clcc\n"); 
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

#define append_str(str1,str2,sep,n) __append_str(&str1,str2,sep,n)

void __append_str( char** pstr1, char* str2, char* sep, size_t n )
{
	DEBUG2("append_str: before: '%s' '%s'",*pstr1,str2);

	if (!*pstr1 || !str2) return;

	size_t len = strlen(str2);

	if (sep) {
		*pstr1 = (char*)realloc(*pstr1,strlen(*pstr1)+len+2);
		strcat(*pstr1,sep);
	} else {
		*pstr1 = (char*)realloc(*pstr1,strlen(*pstr1)+len+1);
	}
	strncat(*pstr1,str2, ((n==0)?len:n) );

	DEBUG2("append_str: after: '%s' '%s'",*pstr1,str2);
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

	char* env_tmpdir = getenv("TMPDIR");

	char* path_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	path_str[0] = '.';
	size_t path_str_len = 2;

	char* def_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	def_str[0] = '\0';

	char* fopt_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	fopt_str[0] = '\0';

	char* cc1_opt_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	cc1_opt_str[0] = '\0';

	char* linker_opt_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	linker_opt_str[0] = '\0';


	/* XXX we will always check if 128 is enough, it should be -DAR */
	unsigned int flist_alloc = 128;
	char** flist = (char**)calloc(flist_alloc,sizeof(char*));
	unsigned int flist_n = 0;

	char* fullpath = (char*)malloc(DEFAULT_STR_SIZE);

	char** argv1 = (char**)calloc(sizeof(char*),argc);
	int argc1 = 0;	

	char cmd[1024];

	int lang = 0;
	int en_opencl = 1;
	int en_stdcl = 0;
	int en_cuda = 0;
	int en_openmp = 0;
	int en_src = 1;
	int en_bin = 1;

	int en_src_only = 0;
	int en_bin_only = 0;

	int quiet = 1;

//	char default_ofname[] = "out_clld.o";
//	char* ofname = default_ofname;
	char* ofname = 0;


	/* XXX todo - add ability to provide lang specific options -DAR */

	FILE* fp;

//   char wdtemp[] = "/tmp/xclXXXXXX";
	char* wdtemp;

	if (env_tmpdir)
		asprintf(&wdtemp,"%s/xclXXXXXX",env_tmpdir);
	else
		asprintf(&wdtemp,"/tmp/xclXXXXXX");

   char* wd = mkdtemp(wdtemp);


	n = 1;
	while (n < argc) {

//printf("compare |%s|\n",argv[n]);

		/* -fopencl */
		if (str_match_exact(argv[n],"-fopencl")) {
		
			append_str(cc1_opt_str,"-fopencl"," ",0);
			en_opencl = 1;


		/* -fstdcl */
		} else if (str_match_exact(argv[n],"-fstdcl")) {
		
			append_str(cc1_opt_str,"-fstdcl"," ",0);
			en_stdcl = 1;


		/* -fcuda */
		} else if (str_match_exact(argv[n],"-fcuda")) {
		
			append_str(cc1_opt_str,"-fcuda"," ",0);
			en_cuda = 1;


		/* -fopenmp */
		} else if (str_match_exact(argv[n],"-fopenmp")) {
		
			append_str(cc1_opt_str,"-fopenmp"," ",0);
			en_openmp = 1;


		/* -s (source only)*/
    } else if (!strcmp(argv[n],"-s")) {

			append_str(cc1_opt_str,"-s"," ",0);
			append_str(linker_opt_str,"-s"," ",0);
			en_src_only = 1;


		/* -b (binary only)*/
    } else if (!strcmp(argv[n],"-b")) {

			append_str(cc1_opt_str,"-b"," ",0);
			append_str(linker_opt_str,"-b"," ",0);
			en_bin_only = 1;


		/* -mavail */
    } else if (!strcmp(argv[n],"-mavail")) {

			append_str(cc1_opt_str,"-mavail"," ",0);
			append_str(linker_opt_str,"-mavail"," ",0);


		/* -mall */
    } else if (!strcmp(argv[n],"-mall")) {

			append_str(cc1_opt_str,"-mall"," ",0);

			append_str(linker_opt_str,"-mall"," ",0);


		/* -mplatform-exclude */
		} else if (str_match_exact(argv[n],"-mplatform-exclude")) {

			append_str(cc1_opt_str,"-mplatform-exclude"," ",0);
			append_str(cc1_opt_str,argv[++n]," ",0);

			append_str(linker_opt_str,"-mplatform-exclude"," ",0);
			append_str(linker_opt_str,argv[++n]," ",0);

		} else if (str_match_seteq(argv[n],"-mplatform-exclude")) {

			append_str(cc1_opt_str,argv[n]," ",0);

			append_str(linker_opt_str,argv[n]," ",0);


		/* -mplatform */
		} else if (str_match_exact(argv[n],"-mplatform")) {

			append_str(cc1_opt_str,"-mplatform"," ",0);
			append_str(cc1_opt_str,argv[++n]," ",0);

			append_str(linker_opt_str,"-mplatform"," ",0);
			append_str(linker_opt_str,argv[++n]," ",0);

		} else if (str_match_seteq(argv[n],"-mplatform")) {

			append_str(cc1_opt_str,argv[n]," ",0);

			append_str(linker_opt_str,argv[n]," ",0);


		/* -mdevice-exclude */
		} else if (str_match_exact(argv[n],"-mdevice-exclude")) {

			append_str(cc1_opt_str,"-mdevice-exclude"," ",0);
			append_str(cc1_opt_str,argv[++n]," ",0);

			append_str(linker_opt_str,"-mdevice-exclude"," ",0);
			append_str(linker_opt_str,argv[++n]," ",0);

		} else if (str_match_seteq(argv[n],"-mdevice-exclude")) {

			append_str(cc1_opt_str,argv[n]," ",0);

			append_str(linker_opt_str,argv[n]," ",0);


		/* -mdevice */
		} else if (str_match_exact(argv[n],"-mdevice")) {

			append_str(cc1_opt_str,"-mdevice"," ",0);
			append_str(cc1_opt_str,argv[++n]," ",0);

			append_str(linker_opt_str,"-mdevice"," ",0);
			append_str(linker_opt_str,argv[++n]," ",0);

		} else if (str_match_seteq(argv[n],"-mdevice")) {

			append_str(cc1_opt_str,argv[n]," ",0);

			append_str(linker_opt_str,argv[n]," ",0);


		/* -x language */
		}  else if (str_match_exact(argv[n],"-x")) {

			append_str(linker_opt_str,"-x"," ",0);
			append_str(linker_opt_str,argv[++n]," ",0);

			if (!strcmp(argv[n],"opencl")) lang = LANG_OPENCL;
			else if (!strcmp(argv[n],"stdcl")) lang = LANG_STDCL;
			else if (!strcmp(argv[n],"cuda")) lang = LANG_CUDA;
			else {
				ERROR2("language '%s' not recognized",argv[n]);
				usage();
				exit(-1);
			}


		} else if (str_match_fused(argv[n],"-I")) {

			append_str(cc1_opt_str,argv[n]," ",0);

		} else if (str_match_exact(argv[n],"-I")) {

			append_str(cc1_opt_str,argv[n]," ",0);
			append_str(cc1_opt_str,argv[++n]," ",0);


		} else if (str_match_fused(argv[n],"-D")) {

			append_str(cc1_opt_str,argv[n]," ",0);

		} else if (str_match_exact(argv[n],"-D")) {

			append_str(cc1_opt_str,argv[n]," ",0);
			append_str(cc1_opt_str,argv[++n]," ",0);


		} else if (!strcmp(argv[n],"-c")) {


		} else if (!strcmp(argv[n],"-o")) {

			ofname = argv[++n];

		} else if (!strcmp(argv[n],"-v")) {

			quiet = 0;

		} else if (!strcmp(argv[n],"-h")||!strcmp(argv[n],"--help")) {

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
				fprintf(stderr,"clcc: '%s' no such file\n",fname);
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


	if (en_src_only && en_bin_only) {
		ERROR2("cannot specify both options -s and -b");
		exit(-1);
	}


//	if (ofname && flist_n > 1) {
//		ERROR2("cannot specify -o with multiple files");
//		exit(-1);
//	} 

	char** tflist = (char**)calloc(flist_n,sizeof(char*));

	char* tfnames_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	tfnames_str[0] = '\0';

	/***
	 *** begin loop over files
	 ***/

	int ifile;
	for(ifile=0;ifile<flist_n;ifile++) {

		char* fname = flist[ifile];
		size_t fname_len = strlen(fname);

		char* fname_ext = strrchr(fname,'.');

/*
		if (lang == 0 && fname_ext) {

			if (!strcmp(fname_ext,".cl")) lang = LANG_OPENCL;
			else if (!strcmp(fname_ext,".c")) lang = LANG_STDCL;
			else if (!strcmp(fname_ext,".cu")) lang = LANG_CUDA;
			else {
				ERROR2("file extension '%s' not recognized",fname_ext);
				usage();
				exit(-1);
			}
			
		} else if (!lang) {

			ERROR2("no language specified");
			usage();
			exit(-1);

		}
*/

//		char cmd[1024];
//		char tfname[] = "/tmp/clccXXXXXX";
//		char tfname[] = TFNAME_TEMPLATE;
		char* tfname;
		if (env_tmpdir)
			asprintf(&tfname,"%s/clccXXXXXX",env_tmpdir);
		else
			asprintf(&tfname,"/tmp/clccXXXXXX");

		int fd = mkstemp(tfname);
		close(fd);

//		tflist[ifile] = (char*)calloc(sizeof(TFNAME_TEMPLATE)+1,1);
//		strcpy(tflist[ifile],tfname);
		tflist[ifile] = strdup(tfname);
		append_str(tfnames_str,tfname," ",0);

DEBUG2("add tfame '%s'",tfname);
DEBUG2("tfames_str '%s'",tfnames_str);

		snprintf(cmd,1024,"clcc1 -o %s %s %s",tfname,cc1_opt_str,fname);
		DEBUG2("%s",cmd);
		system(cmd);

	}

	char default_ofname[] = DEFAULT_OFNAME;
	char* single_ofname = 0;

	if (!ofname) {

		if (flist_n == 1) {
		
			char* fname = flist[0];
			size_t fname_len = strlen(fname);

			single_ofname = (char*)malloc(fname_len+3);
			strncpy(single_ofname,fname,fname_len+1);
			
			char* ext = strrchr(single_ofname,'.');
			if (!ext) {
				ext = single_ofname + fname_len;
				*ext = '.';
			}
			*(ext+1) = 'o';
			*(ext+2) = '\0';

			ofname = single_ofname;

		} else {

			ofname = default_ofname;
	
		}

	}
	
//		if (!ofname) {
//			ofname = (char*)calloc(1,fname_len+1);
//			if (fname_ext) strncpy(ofname,fname,(size_t)(fname_ext-fname));
//			else strncpy(ofname,fname,fname_len);
//			strncat(ofname,".o",fname_len);
//		}

//	snprintf(cmd,1024,"clld -o %s %s %s",ofname,linker_opt_str,tfname);
	snprintf(cmd,1024,"clld -o %s %s %s",ofname,linker_opt_str,tfnames_str);
	DEBUG2("%s",cmd);
	system(cmd);

//		if (flist_n > 1) { free(ofname); ofname=0; }/* XXX a slight hack -DAR */

	for(ifile=0;ifile<flist_n;ifile++) if (tflist[ifile]) {
		DEBUG2("removing temp file '%s'",tflist[ifile]);
		unlink(tflist[ifile]);
	}

	if (single_ofname) free(single_ofname);

	return(0);

}

