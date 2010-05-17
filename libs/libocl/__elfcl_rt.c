/* __elfcl_rt.c
 *
 * Copyright (c) 2009-2010 Brown Deer Technology, LLC.  All Rights Reserved.
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


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <elf.h>
#include <errno.h>
#define __USE_GNU
#include <dlfcn.h>

#include "elf_cl.h"

static int __need_mod_mprotect = 1;

static inline 
void __mod_mprotect()
{
	Dl_info info;
	dladdr(__mod_mprotect,&info);

	char* elfp = (char*)info.dli_fbase;
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfp;

	size_t page_mask = ~(getpagesize()-1);
	size_t offset  = ehdr->e_shoff;
	size_t offset_align  = offset&page_mask;
	size_t sz  = ehdr->e_shnum*ehdr->e_shentsize + (offset-offset_align);

	__need_mod_mprotect = mprotect(elfp+offset_align,sz,PROT_READ);
}


void* __get_shstrtab()
{
	if (__need_mod_mprotect) __mod_mprotect();

	Dl_info info;
	dladdr(__get_shstrtab,&info);

	char* elfp = (char*)info.dli_fbase;
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfp;

	Elf64_Shdr* shdr = (Elf64_Shdr*)(elfp + ehdr->e_shoff);

	char* shstrtab = elfp + shdr[ehdr->e_shstrndx].sh_offset;

	return(shstrtab);
}


size_t __get_clstrtab_sz()
{
	if (__need_mod_mprotect) __mod_mprotect();

	int i;

	Dl_info info;
	dladdr(__get_clstrtab_sz,&info);

	char* elfp = (char*)info.dli_fbase;
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfp;

	Elf64_Shdr* shdr = (Elf64_Shdr*)(elfp + ehdr->e_shoff);

	char* shstrtab = elfp + shdr[ehdr->e_shstrndx].sh_offset;
	
	for(i=0;i<ehdr->e_shnum;i++,shdr++) {
//		getpid();	
//		printf("shstrtab=%p shdr=%p\n",shstrtab,shdr); fflush(stdout);
		if (!strncmp(shstrtab+shdr->sh_name,".clstrtab",9)) break;
	}
	
	size_t clstrtab_sz = (i<ehdr->e_shnum)? shdr->sh_size : 0;

	return(clstrtab_sz);
}


void* __get_clstrtab()
{
	if (__need_mod_mprotect) __mod_mprotect();

	int i;

	Dl_info info;
	dladdr(__get_clstrtab,&info);

	char* elfp = (char*)info.dli_fbase;
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfp;

	Elf64_Shdr* shdr = (Elf64_Shdr*)(elfp + ehdr->e_shoff);

	char* shstrtab = elfp + shdr[ehdr->e_shstrndx].sh_offset;

	for(i=0;i<ehdr->e_shnum;i++,shdr++) 
		if (!strncmp(shstrtab+shdr->sh_name,".clstrtab",9)) break;
	
	char* clstrtab = (i<ehdr->e_shnum)? elfp+shdr->sh_offset : 0;

	return(clstrtab);
}


int __get_clsymtab_n()
{
	if (__need_mod_mprotect) __mod_mprotect();

	int i;

	Dl_info info;
	dladdr(__get_clsymtab_n,&info);

	char* elfp = (char*)info.dli_fbase;
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfp;

	Elf64_Shdr* shdr = (Elf64_Shdr*)(elfp + ehdr->e_shoff);

	char* shstrtab = elfp + shdr[ehdr->e_shstrndx].sh_offset;
	
	for(i=0;i<ehdr->e_shnum;i++,shdr++) 
		if (!strncmp(shstrtab+shdr->sh_name,".clsymtab",9)) break;

	int clsymtab_n = (i<ehdr->e_shnum)? 
		shdr->sh_size/sizeof(struct clsymtab_entry) : 0;

	return(clsymtab_n);
}


void* __get_clsymtab()
{
	if (__need_mod_mprotect) __mod_mprotect();

	int i;

	Dl_info info;
	dladdr(__get_clsymtab,&info);

	char* elfp = (char*)info.dli_fbase;
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfp;

	Elf64_Shdr* shdr = (Elf64_Shdr*)(elfp + ehdr->e_shoff);

	char* shstrtab = elfp + shdr[ehdr->e_shstrndx].sh_offset;
	
	for(i=0;i<ehdr->e_shnum;i++,shdr++) 
		if (!strncmp(shstrtab+shdr->sh_name,".clsymtab",9)) break;
	
	char* clsymtab = (i<ehdr->e_shnum)? elfp+shdr->sh_offset : 0;

	return(clsymtab);
}


int __get_clargtab_n()
{
	if (__need_mod_mprotect) __mod_mprotect();

	int i;

	Dl_info info;
	dladdr(__get_clargtab_n,&info);

	char* elfp = (char*)info.dli_fbase;
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfp;

	Elf64_Shdr* shdr = (Elf64_Shdr*)(elfp + ehdr->e_shoff);

	char* shstrtab = elfp + shdr[ehdr->e_shstrndx].sh_offset;
	
	for(i=0;i<ehdr->e_shnum;i++,shdr++) 
		if (!strncmp(shstrtab+shdr->sh_name,".clargtab",9)) break;
	
	int clargtab_n = (i<ehdr->e_shnum)? 
		shdr->sh_size/sizeof(struct clargtab_entry) : 0;

	return(clargtab_n);
}


void* __get_clargtab()
{
	if (__need_mod_mprotect) __mod_mprotect();

	int i;

	Dl_info info;
	dladdr(__get_clargtab,&info);

	char* elfp = (char*)info.dli_fbase;
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfp;

	Elf64_Shdr* shdr = (Elf64_Shdr*)(elfp + ehdr->e_shoff);

	char* shstrtab = elfp + shdr[ehdr->e_shstrndx].sh_offset;
	
	for(i=0;i<ehdr->e_shnum;i++,shdr++) 
		if (!strncmp(shstrtab+shdr->sh_name,".clargtab",9)) break;

	char* clargtab = (i<ehdr->e_shnum)? elfp+shdr->sh_offset : 0;

	return(clargtab);
}


void* __get_cltextb()
{
	if (__need_mod_mprotect) __mod_mprotect();

	int i;

	Dl_info info;
	dladdr(__get_clargtab,&info);

	char* elfp = (char*)info.dli_fbase;
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elfp;

	Elf64_Shdr* shdr = (Elf64_Shdr*)(elfp + ehdr->e_shoff);

	char* shstrtab = elfp + shdr[ehdr->e_shstrndx].sh_offset;
	
	for(i=0;i<ehdr->e_shnum;i++,shdr++) 
		if (!strncmp(shstrtab+shdr->sh_name,".cltextb",8)) break;

	char* clargtab = (i<ehdr->e_shnum)? elfp+shdr->sh_offset : 0;

	return(clargtab);
}


