/* clelf.c
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
//#include <elf.h>
#include <libelf/libelf.h>
#include <errno.h>

//#include "_version.h"
#include "CL/cl.h"
#include "util.h"
#include "clelf.h"

char shstrtab[] = {
      "\0"
      ".clprgtab\0"  /* section 1, 10, shstrtab offset 1 */
      ".clkrntab\0"  /* section 2, 10, shstrtab offset 11 */
      ".clprgsrc\0"  /* section 3, 10, shstrtab offset 21 */
      ".cltextsrc\0" /* section 4, 11, shstrtab offset 31 */
      ".clprgbin\0"  /* section 5, 10, shstrtab offset 42 */
      ".cltextbin\0" /* section 6, 11, shstrtab offset 52 */
      ".clstrtab\0"  /* section 7, 10, shstrtab offset 63 */
      ".shstrtab\0"  /* section 8, 10, shstrtab offset 73 */
};

int shstrtab_offset[] = { 0,1,11,21,31,42,52,63,73 };


char* platform_name_string[] = { 
		"unknown", 
		"amdapp", 
		"nvidia", 
		"coprthr", 
		"intel" 
};

char* device_name_alias[][2] = {
	"Intel(R) Core","x86_64",
	"Intel(R) Xeon","x86_64",
	"AMD Athlon","x86_64",
	"AMD Phenom","x86_64",
	"ARMv7","ARMv7"
};

int clelf_device_name_alias( char* dname )
{
	int i;
	for(i=0;i<sizeof(device_name_alias)/(2*sizeof(char*));i++)  {
		char* s = device_name_alias[i][0];
		if (!strncasecmp(dname,s,strlen(s))) {
			strcpy(dname,device_name_alias[i][1]);
			return(1);
		}
	}
	return(0);
}

int clelf_platform_code( char* name )
{
	if (!strncasecmp(name,"AMD",3)) return(CLELF_PLATFORM_CODE_AMDAPP);
	else if (!strncasecmp(name,"Nvidia",6)) return(CLELF_PLATFORM_CODE_NVIDIA);
	else if (!strncasecmp(name,"coprthr",7)) return(CLELF_PLATFORM_CODE_COPRTHR);
	else if (!strncasecmp(name,"Intel",5)) return(CLELF_PLATFORM_CODE_INTEL);
	return(0);
}

int clelf_init_data( struct clelf_data_struct* data) 
{

   data->clprgtab_nalloc = DEFAULT_CLPRGTAB_NALLOC;
   data->clprgtab = (struct clprgtab_entry*)
      malloc(__clprgtab_entry_sz * data->clprgtab_nalloc);
   data->clprgtab_n = 0;

   data->clkrntab_nalloc = DEFAULT_CLKRNTAB_NALLOC;
   data->clkrntab = (struct clkrntab_entry*)
      malloc(__clkrntab_entry_sz*data->clkrntab_nalloc);
  	data->clkrntab_n = 0;

   data->clprgsrc_nalloc = DEFAULT_CLPRGSRC_NALLOC;
   data->clprgsrc = (struct clprgsrc_entry*)
      malloc(__clprgsrc_entry_sz * data->clprgsrc_nalloc);
   data->clprgsrc_n = 0;

   data->clprgbin_nalloc = DEFAULT_CLPRGBIN_NALLOC;
   data->clprgbin = (struct clprgbin_entry*)
      malloc(__clprgbin_entry_sz * data->clprgbin_nalloc);
   data->clprgbin_n = 0;

   data->cltextsrc_buf_alloc = DEFAULT_BUF_ALLOC;
   data->cltextsrc_buf = (char*)calloc(1,data->cltextsrc_buf_alloc);
   data->cltextsrc_bufp = data->cltextsrc_buf;

   data->cltextbin_buf_alloc = DEFAULT_BUF_ALLOC;
   data->cltextbin_buf = (char*)calloc(1,data->cltextbin_buf_alloc);
   data->cltextbin_bufp = data->cltextbin_buf;

   data->clstrtab_str_alloc = DEFAULT_STR_SIZE;
   data->clstrtab_str = (char*)calloc(1,data->clstrtab_str_alloc);
   data->clstrtab_strp = data->clstrtab_str;

}



int clelf_write_file( int fd, struct clelf_data_struct* cldata )
{

	Elf* e;
	Elf_Scn* scn;
	Elf_Data* edata;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr,"clld: library initialization failed: %s",elf_errmsg(-1));
		exit(-1);
	}

	if ((e = elf_begin(fd, ELF_C_WRITE, NULL)) == 0) {
		fprintf(stderr,"elf_begin() failed: %d: %s", elf_errno(),elf_errmsg(-1));
		exit(-1);
	}

	ELF_Ehdr* ehdr = 0;
	ELF_Phdr* phdr = 0;
	ELF_Shdr* shdr = 0;

	/*
	 * construct elf header
	 */

	if ((ehdr = elf_newehdr(e)) == 0) {
		fprintf(stderr,"elf64_newehdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	ehdr->e_ident[EI_DATA] = ELFDATA2LSB;

#if defined(__x86_64__)
	ehdr->e_machine = EM_X86_64; 
#elif defined(__i386__)
	ehdr->e_machine = EM_386;
#elif defined(__arm__)
	ehdr->e_machine = EM_ARM;
#endif

//	ehdr->e_type = ET_NONE;
	ehdr->e_type = ET_REL;
	ehdr->e_version = EV_CURRENT;
	ehdr->e_shstrndx = 8; /* set section index of .shstrtab */


	/* 
	 * construct section [1] .clprgtab
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((edata = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	edata->d_align = 16;
	edata->d_off  = 0LL;
	edata->d_buf  = (char*)(cldata->clprgtab);
	edata->d_type = ELF_T_WORD;
	edata->d_size = __clprgtab_entry_sz*cldata->clprgtab_n;
	edata->d_version = EV_CURRENT;

	if ((shdr = elf_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

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

   if ((edata = elf_newdata(scn)) == 0) {
      fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1));
      exit(-1);
   }

   edata->d_align = 16;
   edata->d_off  = 0LL;
   edata->d_buf  = (char*)(cldata->clkrntab);
   edata->d_type = ELF_T_WORD;
   edata->d_size = __clkrntab_entry_sz*cldata->clkrntab_n;
   edata->d_version = EV_CURRENT;

   if ((shdr = elf_getshdr(scn)) == NULL) {
      fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
      exit(-1);
   }
 
   shdr->sh_name = shstrtab_offset[2];
   shdr->sh_type = SHT_PROGBITS;
   shdr->sh_flags = SHF_ALLOC;
   shdr->sh_entsize = __clkrntab_entry_sz;


	/* 
	 * construct section [3] .clprgsrc 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((edata = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	edata->d_align = 16;
	edata->d_off  = 0LL;
	edata->d_buf  = (char*)(cldata->clprgsrc);
	edata->d_type = ELF_T_WORD;
	edata->d_size = __clprgsrc_entry_sz * cldata->clprgsrc_n;
	edata->d_version = EV_CURRENT;

	if ((shdr = elf_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	shdr->sh_name = shstrtab_offset[3];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clprgsrc_entry_sz;


	/* 
	 * construct section [4] .cltextsrc 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((edata = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	edata->d_align = 16;
	edata->d_off  = 0LL;
	edata->d_buf  = cldata->cltextsrc_buf;
	edata->d_type = ELF_T_BYTE;
	edata->d_size = (intptr_t)(cldata->cltextsrc_bufp - cldata->cltextsrc_buf);
	edata->d_version = EV_CURRENT;

	if ((shdr = elf_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	shdr->sh_name = shstrtab_offset[4];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = 0;


	/* 
	 * construct section [5] .clprgbin 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((edata = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	edata->d_align = 16;
	edata->d_off  = 0LL;
	edata->d_buf  = (char*)(cldata->clprgbin);
	edata->d_type = ELF_T_WORD;
	edata->d_size = __clprgbin_entry_sz * cldata->clprgbin_n;
	edata->d_version = EV_CURRENT;

	if ((shdr = elf_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	shdr->sh_name = shstrtab_offset[5];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clprgbin_entry_sz;


	/* 
	 * construct section [6] .cltextbin 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((edata = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	edata->d_align = 16;
	edata->d_off  = 0LL;
	edata->d_buf  = cldata->cltextbin_buf;
	edata->d_type = ELF_T_BYTE;
	edata->d_size = (intptr_t)(cldata->cltextbin_bufp - cldata->cltextbin_buf);
	edata->d_version = EV_CURRENT;

	if ((shdr = elf_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

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

	if ((edata = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

	edata->d_align = 16;
	edata->d_off  = 0LL;
	edata->d_buf  = cldata->clstrtab_str;
	edata->d_type = ELF_T_BYTE;
	edata->d_size = (intptr_t)(cldata->clstrtab_strp - cldata->clstrtab_str);
	edata->d_version = EV_CURRENT;

	if ((shdr = elf_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

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

	if ((edata = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	edata->d_align = 16;
	edata->d_buf = shstrtab;
	edata->d_off = 0LL;
	edata->d_size = sizeof(shstrtab);
	edata->d_type = ELF_T_BYTE;
	edata->d_version = EV_CURRENT;

	if ((shdr = elf_getshdr(scn)) == NULL) {
		fprintf(stderr, "elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	shdr->sh_name = shstrtab_offset[8];
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;


	if (elf_update(e, ELF_C_NULL) < 0)  {
		fprintf(stderr, "elf_update(NULL) failed: %s.", elf_errmsg(-1));
		exit(-1);
	}


	if (elf_update(e, ELF_C_WRITE) < 0)  {
		fprintf(stderr, "elf_update() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	elf_end(e);

}


int clelf_load_sections( char* elf_ptr, struct clelf_sect_struct* sect )
{

      ELF_Ehdr* elf = (ELF_Ehdr*)elf_ptr;
      ELF_Shdr* p_shdr = (ELF_Shdr*)(elf_ptr + elf->e_shoff);
      sect->symtab = 0;
      sect->symtab_n = 0;
      sect->strtab = 0;

      DEBUG2("number of section headers %d",elf->e_shnum);

      char* shstr = (char*)elf_ptr + p_shdr[elf->e_shstrndx].sh_offset;

      sect->has_any_clelf_section = 0;
      sect->has_clprgtab = 0;
      sect->has_clkrntab = 0;
      sect->has_clstrtab = 0;
      sect->has_text = 0;

		int n; 

      for(n=1;n<=elf->e_shnum;n++) {

         DEBUG2("section offset in img %d bytes (%s) size %d",
            p_shdr->sh_offset,
            shstr + p_shdr->sh_name, p_shdr->sh_size);

         if (!strcmp(shstr+p_shdr->sh_name,".clprgtab")) {

            sect->clprgtab=(struct clprgtab_entry*)(elf_ptr+p_shdr->sh_offset);
            sect->clprgtab_n=p_shdr->sh_size/__clprgtab_entry_sz;
            ++sect->has_any_clelf_section;
            sect->has_clprgtab = 1;

         } else if (!strcmp(shstr+p_shdr->sh_name,".clkrntab")) {

            sect->clkrntab=(struct clkrntab_entry*)(elf_ptr+p_shdr->sh_offset);
            sect->clkrntab_n=p_shdr->sh_size/__clkrntab_entry_sz;
            ++sect->has_any_clelf_section;
            sect->has_clkrntab = 1;

         } else if (!strcmp(shstr+p_shdr->sh_name,".clprgsrc")) {

            sect->clprgsrc=(struct clprgsrc_entry*)(elf_ptr+p_shdr->sh_offset);
            sect->clprgsrc_n=p_shdr->sh_size/__clprgsrc_entry_sz;
            ++sect->has_any_clelf_section;

         } else if (!strncmp(shstr+p_shdr->sh_name,".cltextsrc",8)) {

            sect->cltextsrc = (char*)(elf_ptr + p_shdr->sh_offset);
            sect->cltextsrc_sz = p_shdr->sh_size;
            ++sect->has_any_clelf_section;
            if (sect->cltextsrc_sz > 0) ++sect->has_text;

         } else if (!strncmp(shstr+p_shdr->sh_name,".clprgbin",7)) {

            sect->clprgbin=(struct clprgbin_entry*)(elf_ptr+p_shdr->sh_offset);
            sect->clprgbin_n=p_shdr->sh_size/__clprgbin_entry_sz;
            ++sect->has_any_clelf_section;

         } else if (!strncmp(shstr+p_shdr->sh_name,".cltextbin",8)) {

            sect->cltextbin = (char*)(elf_ptr + p_shdr->sh_offset);
            sect->cltextbin_sz = p_shdr->sh_size;
            ++sect->has_any_clelf_section;
            if (sect->cltextbin_sz > 0) ++sect->has_text;

         } else if (!strncmp(shstr+p_shdr->sh_name,".clstrtab",9)) {

            sect->clstrtab = (char*)(elf_ptr + p_shdr->sh_offset);
            sect->clstrtab_sz = p_shdr->sh_size;
            ++sect->has_any_clelf_section;
            sect->has_clstrtab = 1;

         } else if (!strncmp(shstr+p_shdr->sh_name,".symtab",7)) {

            sect->symtab = (ELF_Sym*)(elf_ptr+p_shdr->sh_offset);
            sect->symtab_n = p_shdr->sh_size/sizeof(ELF_Sym);

         } else if (!strncmp(shstr+p_shdr->sh_name,".strtab",7)) {

            sect->strtab = (char*)(elf_ptr + p_shdr->sh_offset);

         }

         p_shdr += 1;

      }

		sect->p_shdr = p_shdr;


      DEBUG2("clelf info {"
         "\n\tclprgtab:\t%p(%d),%d"
         "\n\tclkrntab:\t%p(%d),%d"
         "\n\tclprgsrc:\t\t%p(%d),%d"
         "\n\tcltextsrc:\t%p(%d),%d"
         "\n\tclprgbin:\t\t%p(%d),%d"
         "\n\tcltextbin:\t%p(%d),%d"
         "\n\tclstrtab:\t%p(%d),%d"
         "\n}",
         sect->clprgtab,
            (intptr_t)sect->clprgtab-(intptr_t)elf_ptr,sect->clprgtab_n,
         sect->clkrntab,
            (intptr_t)sect->clkrntab-(intptr_t)elf_ptr,sect->clkrntab_n,
         sect->clprgsrc,
            (intptr_t)sect->clprgsrc-(intptr_t)elf_ptr,sect->clprgsrc_n,
         sect->cltextsrc,
            (intptr_t)sect->cltextsrc-(intptr_t)elf_ptr,sect->cltextsrc_sz,
         sect->clprgbin,
            (intptr_t)sect->clprgbin-(intptr_t)elf_ptr,sect->clprgbin_n,
         sect->cltextbin,
            (intptr_t)sect->cltextbin-(intptr_t)elf_ptr,sect->cltextbin_sz,
         sect->clstrtab,
            (intptr_t)sect->clstrtab-(intptr_t)elf_ptr,sect->clstrtab_sz
      );

}


int clelf_check_hash( 
	char* elf_ptr, struct clelf_sect_struct* sect 
)
{

	int bad_hash = 0;

	int j;

   if (sect->cltextsrc && sect->cltextsrc_sz > 0) {

      unsigned int hash[4];

      const unsigned char* ptr = (const unsigned char*)sect->cltextsrc;
      size_t len = sect->cltextsrc_sz;

      clelf_md5(ptr, len, (unsigned char*)hash);

      unsigned hash0 = 0, hash1 = 0, hash2 = 0, hash3 = 0;

      if (sect->symtab && sect->symtab_n > 0 && sect->strtab) {

         for(j=0;j<sect->symtab_n;j++) {

            if (!strcmp(sect->strtab + sect->symtab[j].st_name,"_CLTEXTSHASH0"))
               hash0 = sect->symtab[j].st_value;

            if (!strcmp(sect->strtab + sect->symtab[j].st_name,"_CLTEXTSHASH1"))
               hash1 = sect->symtab[j].st_value;

            if (!strcmp(sect->strtab + sect->symtab[j].st_name,"_CLTEXTSHASH2"))
               hash2 = sect->symtab[j].st_value;

            if (!strcmp(sect->strtab + sect->symtab[j].st_name,"_CLTEXTSHASH3"))
               hash3 = sect->symtab[j].st_value;

         }

      } else {

         bad_hash = 1;

      }

      if (hash0==0 || hash1==0 || hash2==0 || hash3 == 0 || hash0!=hash[0] 
			|| hash1!=hash[1] || hash2!=hash[2] || hash3!=hash[3]) {

			ERROR2("CLELF HASH MISMATCH %x|%x %x|%x %x|%x %x|%x\n",
				hash0,hash[0],hash1,hash[1],hash2,hash[2],hash3,hash[3]);

         bad_hash = 2;

    	}

   }

   if (sect->cltextbin && sect->cltextbin_sz > 0) {

      unsigned int hash[4];

      const unsigned char* ptr = (const unsigned char*)sect->cltextbin;
      size_t len = sect->cltextbin_sz;

      clelf_md5(ptr, len, (unsigned char*)hash);

      unsigned int hash0 = 0, hash1 = 0, hash2 = 0, hash3 = 0;

      if (sect->symtab && sect->symtab_n > 0 && sect->strtab) {

        	for(j=0;j<sect->symtab_n;j++) {

           	if (!strcmp(sect->strtab + sect->symtab[j].st_name,"_CLTEXTBHASH0"))
              	hash0 = sect->symtab[j].st_value;

           	if (!strcmp(sect->strtab + sect->symtab[j].st_name,"_CLTEXTBHASH1"))
              	hash1 = sect->symtab[j].st_value;

           	if (!strcmp(sect->strtab + sect->symtab[j].st_name,"_CLTEXTBHASH2"))
              	hash2 = sect->symtab[j].st_value;

           	if (!strcmp(sect->strtab + sect->symtab[j].st_name,"_CLTEXTBHASH3"))
              	hash3 = sect->symtab[j].st_value;

        	}

      } else {

         bad_hash = 1;

      }

      if (hash0==0 || hash1==0 || hash2==0 || hash3==0 || hash0!=hash[0] 
			|| hash1!=hash[1] || hash2!=hash[2] || hash3!=hash[3]) {

			ERROR2("CLELF HASH MISMATCH %x|%x %x|%x %x|%x %x|%x\n",
				hash0,hash[0],hash1,hash[1],hash2,hash[2],hash3,hash[3]);

         bad_hash = 2;

      }

   }

	return(bad_hash);

}

