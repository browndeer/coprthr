/* clld.c
 *
 * Copyright (c) 2008 Brown Deer Technology, LLC.  All Rights Reserved.
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
#include <libelf.h>
#include <errno.h>

//#include "_version.h"
#include "util.h"
#include "../clcc/clelf.h"

#define DEFAULT_STR_SIZE	1024
#define DEFAULT_BUF_ALLOC	1048576
#define DEFAULT_CLPRGS_NALLOC	16
#define DEFAULT_CLPRGB_NALLOC	128

struct clelf_info_struct {

   unsigned int clprgtab_n;
   struct clprgtab_entry* clprgtab;

   unsigned int clkrntab_n;
   struct clkrntab_entry* clkrntab;

   unsigned int clprgs_n;
   struct clprgs_entry* clprgs;

   size_t cltexts_sz;
   char* cltexts;

   unsigned int clprgb_n;
   struct clprgb_entry* clprgb;

   size_t cltextb_sz;
   char* cltextb;

   size_t clstrtab_sz;
   char* clstrtab;

};


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


char* platform_name_string[] = { 
		"unknown", 
		"amdapp", 
		"nvidia", 
		"coprthr", 
		"intel" 
};

void usage() 
{
	printf("usage: WRONG-FIX clld [options] file...\n");
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

	int sw_src_bin = 0;


	char* path_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	path_str[0] = '.';
	size_t path_str_len = 2;

	char* def_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	def_str[0] = '\0';

	char* fopt_str = (char*)calloc(1,DEFAULT_STR_SIZE);
	fopt_str[0] = '\0';

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


	/* XXX we will always check if 128 is enough, it should be -DAR */
	unsigned int flist_alloc = 128;
	char** flist = (char**)calloc(flist_alloc,sizeof(char*));
	unsigned int flist_n = 0;

	char* fullpath = (char*)malloc(DEFAULT_STR_SIZE);

	char** argv1 = (char**)calloc(sizeof(char*),argc);
	int argc1 = 0;	

	int lang = LANG_OPENCL;
	int en_openmp = 0;
	int en_source = 1;
	int en_binary = 1;

	char* platform_select = 0;
	char* platform_exclude = 0;
	char* device_select = 0;
	char* device_exclude = 0;

	char default_ofname[] = "out_clld.o";
	char* ofname = default_ofname;


	/* XXX todo - add ability to provide lang specific options -DAR */

	FILE* fp;

#ifdef CLCC_TEST
   char wdtemp[] = "./";
//   char filebase[]   = "XXXXXX";
   char* wd = wdtemp;
#else
   char wdtemp[] = "/tmp/xclXXXXXX";
//   char filebase[]   = "XXXXXX";
   char* wd = mkdtemp(wdtemp);
//   char* fb = mktemp(filebase);
#endif



	n = 1;
	while (n < argc) {


		if (!strncmp(argv[n],"-mplatform",10)) {

			char* p = strchr(argv[n],'=');

			if (p) printf("RECOG =\n");
			else printf("RECOG\n");

			if (!p) p = argv[++n];
			else p += 1;

			if (strnlen(p,DEFAULT_STR_SIZE) == 0) continue;

			list_add_str(&platform_select,p);

		} else if (!strncmp(argv[n],"-mplatform-exclude",18)) {

			char* p = strchr(argv[n],'=');

			if (!p) p = argv[++n];
			else p += 1;

			if (strnlen(p,DEFAULT_STR_SIZE) == 0) continue;

			list_add_str(&platform_exclude,p);
			
		} else if (!strncmp(argv[n],"-mdevice",8)) {

			char* p = strchr(argv[n],'=');

			if (!p) p = argv[++n];
			else p += 1;

			if (strnlen(p,DEFAULT_STR_SIZE) == 0) continue;

			list_add_str(&device_select,p);
			
		} else if (!strncmp(argv[n],"-mdevice-exclude",16)) {

			char* p = strchr(argv[n],'=');

			if (!p) p = argv[++n];
			else p += 1;

			if (strnlen(p,DEFAULT_STR_SIZE) == 0) continue;

			list_add_str(&device_exclude,p);
			
		} else if (!strcmp(argv[n],"-o")) {

			ofname = argv[++n];

		} else if (!strcmp(argv[n],"-h")||!strcmp(argv[n],"--help")) {

			usage(); 
			exit(0);

		} else if (!strcmp(argv[n],"-v")||!strcmp(argv[n],"--version")) {

			version(); 
			exit(0);


		} else if (argv[n][0] == '-') {

			append_str(fopt_str,argv[++i]);

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

			printf("pushed '%s' to the list\n",fname);


		}

		++n;

	}


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


	/* begin loop over files */
	
	int ifile;
	for(ifile=0;ifile<flist_n;ifile++) {

		char* fname = flist[ifile];

		printf("open file %d '%s'\n",ifile,fname);

		int fd = open(fname,O_RDONLY);
	
		struct stat st;
		stat(fname,&st);

		size_t file_sz = st.st_size;
		char* file_ptr = (char*)mmap(0,file_sz,PROT_READ,MAP_PRIVATE,fd,0);
		close(fd);
		
#if defined(__x86_64__)
   	Elf64_Ehdr* elf = (Elf64_Ehdr*)file_ptr;
   	Elf64_Shdr* p_shdr = (Elf64_Shdr*)((intptr_t)elf + elf->e_shoff);
		Elf64_Sym* symtab = 0;
		size_t symtab_n = 0;
		char* strtab = 0;
#elif defined(__i386__)
   	Elf32_Ehdr* elf = (Elf32_Ehdr*)file_ptr;
   	Elf32_Shdr* p_shdr = (Elf32_Shdr*)((intptr_t)elf + elf->e_shoff);
		Elf32_Sym* symtab = 0;
		size_t symtab_n = 0;
		char* strtab = 0;
#endif

		printf("number of section headers %d\n",elf->e_shnum);

		char* shstr = (char*)file_ptr + p_shdr[elf->e_shstrndx].sh_offset;

		struct clelf_info_struct info;
		bzero(&info,sizeof(struct clelf_info_struct));


		int has_any_clelf_section = 0;
		int has_clprgtab = 0;
		int has_clkrntab = 0;
		int has_clstrtab = 0;
		int has_text = 0;

		for(n=1;n<=elf->e_shnum;n++) {

			printf("section offset in img %d bytes (%s) size %d\n",
  	   		p_shdr->sh_offset,
  	   		shstr+p_shdr->sh_name,p_shdr->sh_size
  	 		);

      	if (!strcmp(shstr+p_shdr->sh_name,".clprgtab")) {

         	info.clprgtab=(struct clprgtab_entry*)(file_ptr+p_shdr->sh_offset);
         	info.clprgtab_n=p_shdr->sh_size/__clprgtab_entry_sz;
				++has_any_clelf_section;
				has_clprgtab = 1;

      	} else if (!strcmp(shstr+p_shdr->sh_name,".clkrntab")) {

         	info.clkrntab=(struct clkrntab_entry*)(file_ptr+p_shdr->sh_offset);
         	info.clkrntab_n=p_shdr->sh_size/__clkrntab_entry_sz;
				++has_any_clelf_section;
				has_clkrntab = 1;

      	} else if (!strcmp(shstr+p_shdr->sh_name,".clprgs")) {

         	info.clprgs=(struct clprgs_entry*)(file_ptr+p_shdr->sh_offset);
         	info.clprgs_n=p_shdr->sh_size/__clprgs_entry_sz;
				++has_any_clelf_section;

      	} else if (!strncmp(shstr+p_shdr->sh_name,".cltexts",8)) {

         	info.cltexts = (char*)(file_ptr + p_shdr->sh_offset);
         	info.cltexts_sz = p_shdr->sh_size;
				++has_any_clelf_section;
				if (info.cltexts_sz > 0) ++has_text;

      	} else if (!strncmp(shstr+p_shdr->sh_name,".clprgb",7)) {

         	info.clprgb=(struct clprgb_entry*)(file_ptr+p_shdr->sh_offset);
         	info.clprgb_n=p_shdr->sh_size/__clprgb_entry_sz;
				++has_any_clelf_section;

      	} else if (!strncmp(shstr+p_shdr->sh_name,".cltextb",8)) {

         	info.cltextb = (char*)(file_ptr + p_shdr->sh_offset);
         	info.cltextb_sz = p_shdr->sh_size;
				++has_any_clelf_section;
				if (info.cltextb_sz > 0) ++has_text;

      	} else if (!strncmp(shstr+p_shdr->sh_name,".clstrtab",9)) {

   			info.clstrtab = (char*)(file_ptr + p_shdr->sh_offset);
         	info.clstrtab_sz = p_shdr->sh_size;
				++has_any_clelf_section;
				has_clstrtab = 1;

      	} else if (!strncmp(shstr+p_shdr->sh_name,".symtab",7)) {

#if defined(__x86_64__)
         	symtab = (Elf64_Sym*)(file_ptr+p_shdr->sh_offset);
         	symtab_n = p_shdr->sh_size/sizeof(Elf64_Sym);
#elif defined(__i386__)
         	symtab = (Elf32_Sym*)(file_ptr+p_shdr->sh_offset);
         	symtab_n = p_shdr->sh_size/sizeof(Elf32_Sym);
#endif

      	} else if (!strncmp(shstr+p_shdr->sh_name,".strtab",7)) {

   			strtab = (char*)(file_ptr + p_shdr->sh_offset);

      	}

			p_shdr += 1;

		}	


		DEBUG2("clelf info {"
			"\n\tclprgtab:\t%p(%d),%d"
			"\n\tclkrntab:\t%p(%d),%d"
			"\n\tclprgs:\t\t%p(%d),%d"
			"\n\tcltexts:\t%p(%d),%d"
			"\n\tclprgb:\t\t%p(%d),%d"
			"\n\tcltextb:\t%p(%d),%d"
			"\n\tclstrtab:\t%p(%d),%d"
			"\n}",
			info.clprgtab,
				(intptr_t)info.clprgtab-(intptr_t)file_ptr,info.clprgtab_n,
			info.clkrntab,
				(intptr_t)info.clkrntab-(intptr_t)file_ptr,info.clkrntab_n,
			info.clprgs,
				(intptr_t)info.clprgs-(intptr_t)file_ptr,info.clprgs_n,
			info.cltexts,
				(intptr_t)info.cltexts-(intptr_t)file_ptr,info.cltexts_sz,
			info.clprgb,
				(intptr_t)info.clprgb-(intptr_t)file_ptr,info.clprgb_n,
			info.cltextb,
				(intptr_t)info.cltextb-(intptr_t)file_ptr,info.cltextb_sz,
			info.clstrtab,
				(intptr_t)info.clstrtab-(intptr_t)file_ptr,info.clstrtab_sz
		);

		int skip_file = 0;
		int bad_file = 0;

		/* now we check the various things that can be wrong */

		if (has_any_clelf_section) {

			if (has_any_clelf_section==5 && !has_clprgtab) {

				WARN2(
					"'%s' missing .clprgtab section, possibly legacy format,"
					" file will be ignored",fname);
				skip_file = 1;

			} else if (has_any_clelf_section < 6) {

				ERROR2("'%s' missing one or more clelf sections, exiting",fname);
				bad_file = 1;

			}

			if (!has_clstrtab) {

				ERROR2("'%s' missing .clstrtab section, bad format, exiting",fname);
				bad_file = 1;

			}

			if (!has_text) {

				WARN2(
					"'%s' no content in .cltexts or .cltestb sections,"
					" file will be ignored",fname);
				skip_file = 1;

			}

			if (info.cltexts && info.cltexts_sz > 0) {

					unsigned long hash[2];

					const unsigned char* ptr 
						= (const unsigned char*)info.cltexts;
					size_t len = info.cltexts_sz;

					MD5(ptr, len, (unsigned char*)hash);

					unsigned long hash0 = 0, hash1 = 0;

					if (symtab && symtab_n > 0 && strtab) {

						for(j=0;j<symtab_n;j++) {

							if (!strcmp(strtab+symtab[j].st_name,"_CLTEXTSHASH0")) 
								hash0 = symtab[j].st_value;

							if (!strcmp(strtab+symtab[j].st_name,"_CLTEXTSHASH1"))
								hash1 = symtab[j].st_value;
						}

					} else {

						ERROR2("'%s' missing CLELF hash symbols, exiting",fname);
						bad_file = 1;

					}

					if (hash0==0 || hash1==0 || hash0!=hash[0] || hash1!=hash[1]) {

						ERROR2("'%s' CLELF hash symbol mismatch, exiting",fname);
						fprintf(stderr,"%lx|%lx %lx|%lx\n",
							hash0,hash[0],hash1,hash[1]);
						bad_file = 1;

					}

			}

			if (info.cltextb && info.cltextb_sz > 0) {

					unsigned long hash[2];

					const unsigned char* ptr 
						= (const unsigned char*)info.cltextb;
					size_t len = info.cltextb_sz;

					MD5(ptr, len, (unsigned char*)hash);

					unsigned long hash0 = 0, hash1 = 0;

					if (symtab && symtab_n > 0 && strtab) {

						for(j=0;j<symtab_n;j++) {

							if (!strcmp(strtab+symtab[j].st_name,"_CLTEXTBHASH0")) 
								hash0 = symtab[j].st_value;

							if (!strcmp(strtab+symtab[j].st_name,"_CLTEXTBHASH1"))
								hash1 = symtab[j].st_value;
						}

					} else {

						ERROR2("'%s' missing CLELF hash symbols, exiting",fname);
						bad_file = 1;

					}

					if (hash0==0 || hash1==0 || hash0!=hash[0] || hash1!=hash[1]) {

						ERROR2("'%s' CLELF hash symbol mismatch, exiting",fname);
						fprintf(stderr,"%lx|%lx %lx|%lx\n",
							hash0,hash[0],hash1,hash[1]);
						bad_file = 1;

					}

			}

		} else {

			skip_file = 1;

		}

		printf("skip_file bad_file %d %d\n",skip_file,bad_file);

		if (skip_file) continue;
		else if (bad_file) exit(-2);


		/* if we make it here, we have a good file to link */


		/***
		 *** add clprgtab,clkrntab,clprgb,cltextb entries - adjusting offsets
		 ***/

   	while (clprgtab_n + info.clprgtab_n >= clprgtab_nalloc) {
     		clprgtab_nalloc += DEFAULT_CLPRGTAB_NALLOC;
     		clprgtab = (struct clprgtab_entry*) 
         	realloc(clprgtab,__clprgtab_entry_sz*clprgtab_nalloc);
   	}

   	while (clkrntab_n + info.clkrntab_n >= clkrntab_nalloc) {
     		clkrntab_nalloc += DEFAULT_CLKRNTAB_NALLOC;
     		clkrntab = (struct clkrntab_entry*) 
         	realloc(clkrntab,__clkrntab_entry_sz*clkrntab_nalloc);
   	}

   	while (clprgs_n + info.clprgs_n >= clprgs_nalloc) {
  	    	clprgs_nalloc += DEFAULT_CLPRGS_NALLOC;
 	   	clprgs = (struct clprgs_entry*)
 	      	realloc(clprgs,__clprgs_entry_sz*clprgs_nalloc);
 	  	}

   	while (clprgb_n + info.clprgb_n >= clprgb_nalloc) {
  	   	clprgb_nalloc += DEFAULT_CLPRGB_NALLOC;
 	     	clprgb = (struct clprgb_entry*)
 	     		realloc(clprgb,__clprgb_entry_sz*clprgb_nalloc);
 	  	}

/*
		size_t offset = clstrtab_strp-clstrtab_str;
   	while (offset + info.clprgb_n >= clstrtab_str_alloc) {
  	   	clstrtab_str_alloc += DEFAULT_STR_SIZE;
 	     	clstrtab_str = (char*)realloc(clstrtab_str,clstrtab_str_alloc);
			clstrtab_strp = clstrtab_str + offset;
 	  	}
*/

		for(i=0; i<info.clprgtab_n; i++) {


   		clprgtab[clprgtab_n].e_name = (intptr_t)(clstrtab_strp-clstrtab_str);
			char* name = info.clstrtab + info.clprgtab[i].e_name;
			add_strtab(clstrtab_str,clstrtab_strp,clstrtab_str_alloc,name);
   		clprgtab[clprgtab_n].e_info = info.clprgtab[i].e_info;
   		clprgtab[clprgtab_n].e_prgs = info.clprgtab[i].e_prgs + clprgs_n;
   		clprgtab[clprgtab_n].e_nprgs = info.clprgtab[i].e_nprgs;
   		clprgtab[clprgtab_n].e_prgb = clprgb_n;
			/* defer clprgtab[clprgtab_n].e_nprgb */
   		clprgtab[clprgtab_n].e_krn = clkrntab_n;
   		clprgtab[clprgtab_n].e_nkrn = info.clprgtab[i].e_nkrn;

			int nprgb = 0;

			for(j=0; j<info.clprgtab[i].e_nprgb; j++) {

				int k = info.clprgtab[i].e_prgb + j;
		
				int platform_code = info.clprgb[k].e_platform;
				char* device_name = info.clstrtab + info.clprgb[k].e_device;

printf("DEVICE NAME %s\n",device_name);

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

				if (select && !exclude) {

					printf("XXX %s |%s:%s|\n",name,
						platform_name_string[platform_code],device_name);
	
   				clprgb[clprgb_n].e_name = (intptr_t)(clstrtab_strp-clstrtab_str);
					char* name = info.clstrtab + info.clprgb[k].e_name;
					add_strtab(clstrtab_str,clstrtab_strp,clstrtab_str_alloc,name);
   				clprgb[clprgb_n].e_info = info.clprgb[k].e_info;
   				clprgb[clprgb_n].e_platform = info.clprgb[k].e_platform;
   				clprgb[clprgb_n].e_device = info.clprgb[k].e_device;
   				clprgb[clprgb_n].e_shndx = -1;
   				clprgb[clprgb_n].e_offset = (intptr_t)(cltextb_bufp-cltextb_buf);
   				clprgb[clprgb_n].e_size = info.clprgb[k].e_size;
   				++clprgb_n;

					size_t offset = (intptr_t)(cltextb_bufp-cltextb_buf);

					while (offset + info.clprgb[k].e_size > cltextb_buf_alloc) {
     					cltextb_buf_alloc += DEFAULT_BUF_ALLOC;
     					cltextb_buf = realloc(cltextb_buf,cltextb_buf_alloc);
     					cltextb_bufp = cltextb_buf + offset;
   				}

					memcpy(cltextb_bufp,info.cltextb + info.clprgb[k].e_offset,
						info.clprgb[k].e_size);

         		cltextb_bufp += info.clprgb[k].e_size;


					++nprgb;

				}

			}

   		clprgtab[clprgtab_n].e_nprgb = nprgb;


			for(j=0; j<info.clprgtab[i].e_nkrn; j++) {

				int k = info.clprgtab[i].e_krn + j;

   			clkrntab[clkrntab_n].e_name =(intptr_t)(clstrtab_strp-clstrtab_str);
				char* name = info.clstrtab + info.clkrntab[k].e_name;
				add_strtab(clstrtab_str,clstrtab_strp,clstrtab_str_alloc,name);
   			clkrntab[clkrntab_n].e_info = info.clkrntab[k].e_info;
   			clkrntab[clkrntab_n].e_prg = clprgtab_n;
   			++clkrntab_n;

			}


   		++clprgtab_n;

		}


   	/***
   	 *** add clprgs entries and copy cltexts adjusting offsets as ncessary
   	 ***/

		for(i=0; i<info.clprgs_n; i++) {

   		clprgs[clprgs_n].e_name = (intptr_t)(clstrtab_strp-clstrtab_str);
			char* name = info.clstrtab + info.clprgs[i].e_name;
			add_strtab(clstrtab_str,clstrtab_strp,clstrtab_str_alloc,name);
   		clprgs[clprgs_n].e_info = info.clprgs[i].e_info;
   		clprgs[clprgs_n].e_platform = info.clprgs[i].e_platform;
   		clprgs[clprgs_n].e_device = info.clprgs[i].e_device;
   		clprgs[clprgs_n].e_shndx = -1;
			clprgs[clprgs_n].e_offset = (intptr_t)(cltexts_bufp-cltexts_buf);
   		clprgs[clprgs_n].e_size = info.clprgs[i].e_size;
   		++clprgs_n;

			size_t offset = (intptr_t)(cltexts_bufp-cltexts_buf);
			while (offset + info.clprgs[i].e_size > cltexts_buf_alloc) {
  	   		cltexts_buf_alloc += DEFAULT_BUF_ALLOC;
  	   		cltexts_buf = realloc(cltexts_buf,cltexts_buf_alloc);
  	   		cltexts_bufp = cltexts_buf + offset;
  		 	}

   		memcpy(cltexts_bufp, info.cltexts + info.clprgs[i].e_offset, 
				info.clprgs[i].e_size);

	  		cltexts_bufp += info.clprgs[i].e_size;

		}

	}


	/***
	 *** calculate md5 has for cltexts and cltextb
	 ***/

	unsigned long cltextshash[2];
	unsigned long cltextbhash[2];


	size_t len = (intptr_t)cltexts_bufp-(intptr_t)cltexts_buf;
	MD5((const unsigned char*)cltexts_buf, len, (unsigned char*)cltextshash);

	len = (intptr_t)cltextb_bufp-(intptr_t)cltextb_buf;
	MD5((const unsigned char*)cltextb_buf, len, (unsigned char*)cltextbhash);

	printf("%lx %lx \n",cltextshash[0],cltextshash[1]);
	printf("%lx %lx \n",cltextbhash[0],cltextbhash[1]);


	/***
	 *** create special elf file
	 ***/

#ifdef CLCC_TEST
	char tfname[] = "./clldXXXXXX";
#else
	char tfname[] = "/tmp/clldXXXXXX";
#endif
	int fd = mkstemp(tfname);

	if (fd < 0) {
		fprintf(stderr,"clld: mkstemp failed");
		exit(-1);
	}
	
	Elf* e;
	Elf_Scn* scn;
	Elf_Data* data;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr,"clld: library initialization failed: %s",elf_errmsg(-1));
		exit(-1);
	}

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

	snprintf( cmd, 1024, "ld -r -o %s"
		" %s"
		" --defsym _CLTEXTSHASH0=0x%lx"
		" --defsym _CLTEXTSHASH1=0x%lx"
		" --defsym _CLTEXTBHASH0=0x%lx"
		" --defsym _CLTEXTBHASH1=0x%lx",
		ofname, tfname,
		cltextshash[0], cltextshash[1], cltextbhash[0], cltextbhash[1] );

	printf("|%s|\n",cmd);
	system(cmd);

#ifndef CLCC_TEST
	unlink(tfname);
#endif
	
	return(0);

}

