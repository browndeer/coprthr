/* elf_cl.c
 *
 * Copyright (c) 2008-2010 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation.
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

#include <stdio.h>
#include <stdlib.h>
//#include <elf.h>
//#include <libelf.h>
#include <libelf/libelf.h>

#include "version.h"
#include "util.h"
#include "elf_cl.h"


static
char shstrtab[] = {
	"\0"
	".cldev\0"     /* section 1, shstrtab offset 1 */
	".clprgs\0"    /* section 2, shstrtab offset 8 */
	".cltexts\0"   /* section 3, shstrtab offset 16 */
	".clprgb\0"    /* section 4, shstrtab offset 25 */
	".cltextb\0"   /* section 5, shstrtab offset 33 */
	".clsymtab\0"  /* section 6, shstrtab offset 42 */
	".clargtab\0"  /* section 6, shstrtab offset 52 */
	".clstrtab\0"  /* section 7, shstrtab offset 62 */
	".shstrtab\0"  /* section 8, shstrtab offset 72 */
};

static
int shstrtab_offset[] = { 0,1,8,16,25,33,42,52,62,72 };

#define __cldev_entry_sz sizeof(struct cldev_entry)
#define __clprgs_entry_sz sizeof(struct clprgs_entry)
#define __clprgb_entry_sz sizeof(struct clprgb_entry)
#define __clsymtab_entry_sz sizeof(struct clsymtab_entry)
#define __clargtab_entry_sz sizeof(struct clargtab_entry)


int elfcl_write( 
	int fd, 
	struct cldev_entry* cldev, unsigned int cldev_n,
	struct clprgs_entry* clprgs, unsigned int clprgs_n,
	char* cltexts_buf, size_t cltexts_sz,
	struct clprgb_entry* clprgb, unsigned int clprgb_n,
	char* cltextb_buf, size_t cltextb_sz,
	struct clsymtab_entry* clsymtab, size_t clsymtab_n,
	struct clargtab_entry* clargtab, size_t clargtab_n,
	char* clstrtab, size_t clstrtab_sz
)
{

	/*
	 * create special elf file
	 */

	Elf* e;
	Elf_Scn* scn;
	Elf_Data* data;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: libelf initialization failed: %s",elf_errmsg(-1));
		exit(-1);
	}

	if ((e = elf_begin(fd, ELF_C_WRITE, 0)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_begin() failed: %s", elf_errmsg(-1));
		exit(-1);
	}


#if defined(__x86_64__)

DEBUG(__FILE__,__LINE__,"__x86_64__");

	Elf64_Ehdr* ehdr = 0;
	Elf64_Phdr* phdr = 0;
	Elf64_Shdr* shdr = 0;

#elif defined(__i386__) 

DEBUG(__FILE__,__LINE__,"__i386__");

	Elf32_Ehdr* ehdr = 0;
	Elf32_Phdr* phdr = 0;
	Elf32_Shdr* shdr = 0;

#endif


	/*
	 * construct elf header
	 */

#if defined(__x86_64__)

	if ((ehdr = elf64_newehdr(e)) == 0) {
		int err = elf_errno();
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_newehdr() failed: %d %s.", err, elf_errmsg(err));
		exit(-1);
	}

	ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr->e_machine = EM_X86_64; 
	ehdr->e_type = ET_NONE;
	ehdr->e_shstrndx = 9; /* set section index of .shstrtab */

#elif defined(__i386__) 

	if ((ehdr = elf32_newehdr(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_newehdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr->e_machine = EM_386; 
	ehdr->e_type = ET_NONE;
	ehdr->e_shstrndx = 6; /* set section index of .shstrtab */

#endif

	/* 
	 * construct section [1] .cldev
	 */

	if ((scn = elf_newscn(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 4;
	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = (char*)cldev;
	data->d_type = ELF_T_WORD;
	data->d_size = __cldev_entry_sz*cldev_n;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[1];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __cldev_entry_sz;



	/* 
	 * construct section [2] .clprgs 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 4;
	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = (char*)clprgs;
	data->d_type = ELF_T_WORD;
	data->d_size = __clprgs_entry_sz*clprgs_n;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif

	shdr->sh_name = shstrtab_offset[2];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clprgs_entry_sz;



	/* 
	 * construct section [3] .cltexts 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 1;
	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = cltexts_buf;
	data->d_type = ELF_T_BYTE;
	data->d_size = cltexts_sz;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif


	shdr->sh_name = shstrtab_offset[3];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = 0;



	/* 
	 * construct section [4] .clprgb 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 4;
	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = (char*)clprgb;
	data->d_type = ELF_T_WORD;
	data->d_size = __clprgb_entry_sz*clprgb_n;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif


	shdr->sh_name = shstrtab_offset[4];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clprgs_entry_sz;



	/* 
	 * construct section [5] .cltextb 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 1;
	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = cltextb_buf;
	data->d_type = ELF_T_BYTE;
	data->d_size = cltextb_sz;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif


	shdr->sh_name = shstrtab_offset[5];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = 0;



	/* 
	 * construct section [6] .clsymtab
	 */

	if ((scn = elf_newscn(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 4;
	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = (char*)clsymtab;
	data->d_type = ELF_T_WORD;
	data->d_size = __clsymtab_entry_sz*clsymtab_n;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif


	shdr->sh_name = shstrtab_offset[6];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clsymtab_entry_sz;



	/* 
	 * construct section [7] .clargtab
	 */

	if ((scn = elf_newscn(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 4;
	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = (char*)clargtab;
	data->d_type = ELF_T_WORD;
	data->d_size = __clargtab_entry_sz*clargtab_n;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif


	shdr->sh_name = shstrtab_offset[7];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = __clargtab_entry_sz;



	/* 
	 * construct section [8] .clstrtab 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 1;
	data->d_align = 16;
	data->d_off  = 0LL;
	data->d_buf  = clstrtab;
	data->d_type = ELF_T_BYTE;
	data->d_size = clstrtab_sz;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif


	shdr->sh_name = shstrtab_offset[8];
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;



	/* 
	 * construct section [9] .shstrtab 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newscn() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_newdata() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

//	data->d_align = 1;
	data->d_align = 16;
	data->d_buf = shstrtab;
	data->d_off = 0LL;
	data->d_size = sizeof(shstrtab);
	data->d_type = ELF_T_BYTE;
	data->d_version = EV_CURRENT;

#if defined(__x86_64__)
	if ((shdr = elf64_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf64_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#elif defined(__i386__)
	if ((shdr = elf32_getshdr(scn)) == 0) {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf32_getshdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}
#endif


	shdr->sh_name = shstrtab_offset[9];;
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;



	printf("EV_CURRENT %d\n",EV_CURRENT);

	if (elf_update(e, ELF_C_NULL) < 0)  {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_update(0) failed: %s.", elf_errmsg(-1));
		exit(-1);
	}


	if (elf_update(e, ELF_C_WRITE) < 0)  {
		ERROR(__FILE__,__LINE__,
			"write_elf_cl: elf_update() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	elf_end(e);

	return(0);

}


#if(0)

unsigned int
elfcl_get_clprgb_n( void* p )
{
	int i;
	char* elfp = (char*)p;

	if (strncmp(elfp+1,"ELF",3)) {
		WARN(__FILE__,__LINE__,"elfcl_get_clprgb: not an elf object");
		return(0);
	}

	Elf64_Ehdr* elfhdr = (Elf64_Ehdr*)elfp;

	printf("%d\n",elfhdr->e_shoff);

	Elf64_Shdr* elfsh = (Elf64_Shdr*)(elfp + elfhdr->e_shoff);

	printf("%d\n",elfhdr->e_shstrndx);

	char* shstrtab = elfp + elfsh[elfhdr->e_shstrndx].sh_offset;

	printf("%d\n",elfsh[elfhdr->e_shstrndx].sh_offset);

	printf("shstrtab ~ %s\n",shstrtab);	
	printf("shstrtab ~ %s\n",shstrtab+elfsh[elfhdr->e_shstrndx].sh_name);	
	if (strncmp(shstrtab+elfsh[elfhdr->e_shstrndx].sh_name,".shstrtab",9)) {
		WARN(__FILE__,__LINE__,"elfcl_get_clprgb: bad shstrtab");
		return(0);
	}

	for(i=0;i<elfhdr->e_shnum;i++,elfsh++)
		if (!strncmp(shstrtab+elfsh->sh_name,".clprgb",7)) 
			return(elfsh->sh_size/elfsh->sh_entsize);

	WARN(__FILE__,__LINE__,"elfcl_get_clprgb: clprgb section not found");

	return(0);
}

void* 
elfcl_get_clprgb( void* p )
{
	int i;
	char* elfp = (char*)p;

	if (strncmp(elfp+1,"ELF",3)) {
		WARN(__FILE__,__LINE__,"elfcl_get_clprgb: not an elf object");
		return(0);
	}

	Elf64_Ehdr* elfhdr = (Elf64_Ehdr*)elfp;

	printf("%d\n",elfhdr->e_shoff);

	Elf64_Shdr* elfsh = (Elf64_Shdr*)(elfp + elfhdr->e_shoff);

	printf("%d\n",elfhdr->e_shstrndx);

	char* shstrtab = elfp + elfsh[elfhdr->e_shstrndx].sh_offset;

	printf("%d\n",elfsh[elfhdr->e_shstrndx].sh_offset);

	printf("shstrtab ~ %s\n",shstrtab);	
	printf("shstrtab ~ %s\n",shstrtab+elfsh[elfhdr->e_shstrndx].sh_name);	
	if (strncmp(shstrtab+elfsh[elfhdr->e_shstrndx].sh_name,".shstrtab",9)) {
		WARN(__FILE__,__LINE__,"elfcl_get_clprgb: bad shstrtab");
		return(0);
	}

	for(i=0;i<elfhdr->e_shnum;i++,elfsh++)
		if (!strncmp(shstrtab+elfsh->sh_name,".clprgb",7)) 
			return(elfp+elfsh->sh_offset);

	WARN(__FILE__,__LINE__,"elfcl_get_clprgb: clprgb section not found");

	return(0);
}

void* 
elfcl_get_cltextb( void* p )
{
	int i;
	char* elfp = (char*)p;

	if (strncmp(elfp+1,"ELF",3)) {
		WARN(__FILE__,__LINE__,"elfcl_get_cltextb: not an elf object");
		return(0);
	}

	Elf64_Ehdr* elfhdr = (Elf64_Ehdr*)elfp;

	printf("%d\n",elfhdr->e_shoff);

	Elf64_Shdr* elfsh = (Elf64_Shdr*)(elfp + elfhdr->e_shoff);

	printf("%d\n",elfhdr->e_shstrndx);

	char* shstrtab = elfp + elfsh[elfhdr->e_shstrndx].sh_offset;

	printf("%d\n",elfsh[elfhdr->e_shstrndx].sh_offset);

	printf("shstrtab ~ %s\n",shstrtab);	
	printf("shstrtab ~ %s\n",shstrtab+elfsh[elfhdr->e_shstrndx].sh_name);	
	if (strncmp(shstrtab+elfsh[elfhdr->e_shstrndx].sh_name,".shstrtab",9)) {
		WARN(__FILE__,__LINE__,"elfcl_get_clprgb: bad shstrtab");
		return(0);
	}

	for(i=0;i<elfhdr->e_shnum;i++,elfsh++)
		if (!strncmp(shstrtab+elfsh->sh_name,".cltextb",8)) 
			return(elfp+elfsh->sh_offset);

	WARN(__FILE__,__LINE__,"elfcl_get_clprgb: cltextb section not found");

	return(0);
}

#endif

