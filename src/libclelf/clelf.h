/* clelf.h
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

#ifndef _CLELF_H
#define _CLELF_H

#define CLCC_TEST

/* DAR */

#define CL_CONTEXT_OFFLINE_DEVICES_AMD 0x403F

#define CLELF_PLATFORM_CODE_AMDAPP	1
#define CLELF_PLATFORM_CODE_NVIDIA	2
#define CLELF_PLATFORM_CODE_COPRTHR	3
#define CLELF_PLATFORM_CODE_INTEL	4

#define DEFAULT_STR_SIZE			1024
#define DEFAULT_BUF_ALLOC			1048576
#define DEFAULT_CLPRGTAB_NALLOC	16
#define DEFAULT_CLKRNTAB_NALLOC	16
#define DEFAULT_CLPRGSRC_NALLOC	16
#define DEFAULT_CLPRGBIN_NALLOC	128


#if defined(__x86_64__)

struct clprgtab_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Half e_prgsrc;
	Elf64_Half e_nprgsrc;
	Elf64_Half e_prgbin;
	Elf64_Half e_nprgbin;
	Elf64_Half e_krn;
	Elf64_Half e_nkrn;
};

struct clkrntab_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Half e_prg;
};

struct clprgsrc_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Word e_platform;
	Elf64_Word e_device;
	Elf64_Half e_shndx;
	Elf64_Addr	e_offset;
	Elf64_Xword e_size;
};

struct clprgbin_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Word e_platform;
	Elf64_Word e_device;
	Elf64_Half e_shndx;
	Elf64_Addr	e_offset;
	Elf64_Xword e_size;
};

#elif defined(__i386__)

struct clprgtab_entry {
	Elf32_Word e_name;
	Elf32_Word e_info;
	Elf32_Half e_prgsrc;
	Elf32_Half e_nprgsrc;
	Elf32_Half e_prgbin;
	Elf32_Half e_nprgbin;
};

struct clprgsrc_entry {
	Elf32_Word e_name;
	Elf32_Word e_info;
	Elf32_Half e_platform;
	Elf32_Half e_device;
	Elf32_Half e_shndx;
	Elf32_Addr	e_offset;
	Elf32_Xword e_size;
};

struct clprgbin_entry {
	Elf32_Word e_name;
	Elf32_Word e_info;
	Elf32_Half e_platform;
	Elf32_Half e_device;
	Elf32_Half e_shndx;
	Elf32_Addr	e_offset;
	Elf32_Xword e_size;
};

#endif

#define __clprgtab_entry_sz sizeof(struct clprgtab_entry)
#define __clkrntab_entry_sz sizeof(struct clkrntab_entry)
#define __clprgsrc_entry_sz sizeof(struct clprgsrc_entry)
#define __clprgbin_entry_sz sizeof(struct clprgbin_entry)


#define LANG_OPENCL  1
#define LANG_STDCL   2
#define LANG_CUDA    3


struct clelf_sect_struct {

#if defined(__x86_64__)
      Elf64_Shdr* p_shdr;
      Elf64_Sym* symtab;
      size_t symtab_n;
      char* strtab;
#elif defined(__i386__)
      Elf32_Shdr* p_shdr;
      Elf32_Sym* symtab;
      size_t symtab_n;
      char* strtab;
#endif
	
   unsigned int clprgtab_n;
   struct clprgtab_entry* clprgtab;

   unsigned int clkrntab_n;
   struct clkrntab_entry* clkrntab;

   unsigned int clprgsrc_n;
   struct clprgsrc_entry* clprgsrc;

   size_t cltextsrc_sz;
   char* cltextsrc;

   unsigned int clprgbin_n;
   struct clprgbin_entry* clprgbin;

   size_t cltextbin_sz;
   char* cltextbin;

   size_t clstrtab_sz;
   char* clstrtab;

   int has_any_clelf_section;
   int has_clprgtab;
   int has_clkrntab;
   int has_clstrtab;
   int has_text;

};

struct clelf_data_struct{

   unsigned int clprgtab_nalloc;
   struct clprgtab_entry* clprgtab;
   unsigned int clprgtab_n;
   
   unsigned int clkrntab_nalloc;
   struct clkrntab_entry* clkrntab;
   unsigned int clkrntab_n;
   
   unsigned int clprgsrc_nalloc;
   struct clprgsrc_entry* clprgsrc;
   unsigned int clprgsrc_n;
   
   unsigned int clprgbin_nalloc;
   struct clprgbin_entry* clprgbin ;
   unsigned int clprgbin_n;

   size_t cltextsrc_buf_alloc;
   char* cltextsrc_buf;
   char* cltextsrc_bufp;

   size_t cltextbin_buf_alloc;
   char* cltextbin_buf;
   char* cltextbin_bufp;

   size_t clstrtab_str_alloc;
   char* clstrtab_str;
   char* clstrtab_strp;

};


extern int shstrtab_offset[];

int clelf_init_data( struct clelf_data_struct* data );
int clelf_write_file( int fd, struct clelf_data_struct* );
int clelf_load_sections( char* elf_ptr, struct clelf_sect_struct* );
int clelf_check_hash( char* elf_ptr, struct clelf_sect_struct* );


#endif

