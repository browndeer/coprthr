/* clnm.c
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

//#include "_version.h"
//#include "CL/cl.h"
//#include "util.h"
#include "printcl.h"
#include "../../src/libclelf/clelf.h"


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
	printf("BDT clnm\n"); 
	printf(
		"Copyright (c) 2008-2011 Brown Deer Technology, LLC."
		" All Rights Reserved.\n"
	);
	printf("This program is free software distributed under GPLv3.\n");
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

	struct clelf_data_struct data;
   clelf_init_data(&data);


	/* XXX we will always check if 128 is enough, it should be -DAR */
	unsigned int flist_alloc = 128;
	char** flist = (char**)calloc(flist_alloc,sizeof(char*));
	unsigned int flist_n = 0;

	char* fullpath = (char*)malloc(DEFAULT_STR_SIZE);

	char** argv1 = (char**)calloc(sizeof(char*),argc);
	int argc1 = 0;	

	int en_openmp = 0;
	int en_src = 1;
	int en_bin = 1;



	n = 1;
	while (n < argc) {

		if (!strcmp(argv[n],"-h")||!strcmp(argv[n],"--help")) {

			usage(); 
			exit(0);

		} else if (!strcmp(argv[n],"-V") || !strcmp(argv[n],"--version")) {

			version(); 
			exit(0);

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


		printcl( CL_DEBUG "data.clprgtab_n sect.clprgtab_n %d %d",
			data.clprgtab_n,sect.clprgtab_n);

		printcl( CL_DEBUG "data.clkrntab_n sect.clkrntab_n %d %d",
			data.clkrntab_n,sect.clkrntab_n);

		printcl( CL_DEBUG "data.clprgsrc_n sect.clprgsrc_n %d %d",
			data.clprgsrc_n,sect.clprgsrc_n);

		printcl( CL_DEBUG "data.clprgbin_n sect.clprgbin_n %d %d",
			data.clprgbin_n,sect.clprgbin_n);


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

		for(i=0; i<sect.clprgtab_n; i++) {

   		data.clprgtab[data.clprgtab_n].e_name 
				= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
			char* name = sect.clstrtab + sect.clprgtab[i].e_name;
			add_strtab(data.clstrtab_str,data.clstrtab_strp,
				data.clstrtab_str_alloc,name);
   		data.clprgtab[data.clprgtab_n].e_info = sect.clprgtab[i].e_info;
   		data.clprgtab[data.clprgtab_n].e_prgsrc 
				= sect.clprgtab[i].e_prgsrc + data.clprgsrc_n;
   		data.clprgtab[data.clprgtab_n].e_nprgsrc = sect.clprgtab[i].e_nprgsrc;
   		data.clprgtab[data.clprgtab_n].e_prgbin = data.clprgbin_n;
			/* defer clprgtab[clprgtab_n].e_nprgbin */
   		data.clprgtab[data.clprgtab_n].e_krn = data.clkrntab_n;
   		data.clprgtab[data.clprgtab_n].e_nkrn = sect.clprgtab[i].e_nkrn;

			int nprgbin = 0;

			if (en_bin) for(j=0; j<sect.clprgtab[i].e_nprgbin; j++) {

				int k = sect.clprgtab[i].e_prgbin + j;
		
				int platform_code = sect.clprgbin[k].e_platform;
				char* device_name = sect.clstrtab + sect.clprgbin[k].e_device;

				int pselect = 1;
				int pexclude = 0;
				int dselect = 1;
				int dexclude = 0;

				int l;

				{
					printf("clnm: '%s' bin [%s:%s]\n",name,
						platform_name_string[platform_code],device_name);
	
   				data.clprgbin[data.clprgbin_n].e_name 
						= (intptr_t)(data.clstrtab_strp-data.clstrtab_str);
					char* name = sect.clstrtab + sect.clprgbin[k].e_name;
					add_strtab(data.clstrtab_str,data.clstrtab_strp,
						data.clstrtab_str_alloc,name);
   				data.clprgbin[data.clprgbin_n].e_info = sect.clprgbin[k].e_info;
   				data.clprgbin[data.clprgbin_n].e_platform =sect.clprgbin[k].e_platform;
   				data.clprgbin[data.clprgbin_n].e_device = sect.clprgbin[k].e_device;
   				data.clprgbin[data.clprgbin_n].e_shndx = -1;
   				data.clprgbin[data.clprgbin_n].e_offset 
						= (intptr_t)(data.cltextbin_bufp-data.cltextbin_buf);
   				data.clprgbin[data.clprgbin_n].e_size = sect.clprgbin[k].e_size;
   				++data.clprgbin_n;

					size_t offset = (intptr_t)(data.cltextbin_bufp-data.cltextbin_buf);

					while (offset + sect.clprgbin[k].e_size > data.cltextbin_buf_alloc) {
     					data.cltextbin_buf_alloc += DEFAULT_BUF_ALLOC;
     					data.cltextbin_buf = realloc(data.cltextbin_buf,
							data.cltextbin_buf_alloc);
     					data.cltextbin_bufp = data.cltextbin_buf + offset;
   				}

					memcpy(data.cltextbin_bufp,sect.cltextbin + sect.clprgbin[k].e_offset,
						sect.clprgbin[k].e_size);

         		data.cltextbin_bufp += sect.clprgbin[k].e_size;

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

				printf("clnm: '%s' ksym %s\n",name,kname);
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

			printf("clnm: '%s' src [<generic>]\n",name);

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

	return(0);

}

