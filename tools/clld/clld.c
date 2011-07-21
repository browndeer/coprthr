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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <libelf.h>
#include <errno.h>

//#include "_version.h"


#define DEFAULT_STR_SIZE	1024
#define DEFAULT_BUF_ALLOC	1048576
#define DEFAULT_CLPRGS_NALLOC	16

char shstrtab[] = { 
		"\0" 
		".clprgs\0" /* section 1, shstrtab offset 1 */
		".cltexts\0" /* section 2, shstrtab offset 9 */
		".clprgb\0" /* section 3, shstrtab offset 18 */
		".cltextb\0" /* section 4, shstrtab offset 26 */
		".clstrtab\0" /* section 5, shstrtab offset 35 */
		".shstrtab\0" /* section 6, shstrtab offset 45 */
};

int shstrtab_offset[] = { 0,1,9,18,26,35,45 };


#if defined(__x86_64__)

struct clprgs_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Half e_shndx;
	Elf64_Addr	e_offset;
	Elf64_Xword e_size;
};

struct clprgb_entry {
	Elf64_Word e_name;
	Elf64_Word e_info;
	Elf64_Half e_shndx;
	Elf64_Addr	e_offset;
	Elf64_Xword e_size;
};

#elif defined(__i386__)

struct clprgs_entry {
	Elf32_Word e_name;
	Elf32_Word e_info;
	Elf32_Half e_shndx;
	Elf32_Addr	e_offset;
	Elf32_Xword e_size;
};

struct clprgb_entry {
	Elf32_Word e_name;
	Elf32_Word e_info;
	Elf32_Half e_shndx;
	Elf32_Addr	e_offset;
	Elf32_Xword e_size;
};

#endif

#define __clprgs_entry_sz sizeof(struct clprgs_entry)
#define __clprgb_entry_sz sizeof(struct clprgb_entry)


struct srctab_entry
{
	LIST_ENTRY(srctab_entry) srctab_list;
	char* filename;
	unsigned int platform_type;
	unsigned int device_type;
};

struct bintab_entry
{
	LIST_ENTRY(bintab_entry) bintab_list;
	char* filename;
	unsigned int platform_type;
	unsigned int device_type;
};


void usage() 
{
	printf("usage: clld [options] file...\n");
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
//	printf("BDT clld version " VERSION "\n"); 
	printf("BDT clld\n"); 
	printf(
		"Copyright (c) 2008 Brown Deer Technology, LLC.  All Rights Reserved.\n"
	);
	printf("This program is free software distributed under LGPLv3.\n");
}

void add_path( char* path_str, size_t* path_str_len, char* path )
{
	size_t len = strnlen(path,1024);

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

//		printf("(%d)|%s|\n",(intptr_t)p - (intptr_t)path_str,p);
		snprintf(fullpath,DEFAULT_STR_SIZE,"%s/%s",p,filename);
//		printf("%s\n",fullpath);

		if (!stat(fullpath,&st) && S_ISREG(st.st_mode)) return(0);

		p += strnlen(p,DEFAULT_STR_SIZE-((intptr_t)p+(intptr_t)path_str)) + 1;
	}

	*fullpath = '\0';

	return(-1);
}


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

	LIST_HEAD(srctab_head_struct, srctab_entry) srctab_head;
	LIST_HEAD(bintab_head_struct, bintab_entry) bintab_head;

	LIST_INIT(&srctab_head);
	LIST_INIT(&bintab_head);

	struct srctab_entry* sp;
	struct bintab_entry* bp;

	unsigned int clprgs_nalloc = DEFAULT_CLPRGS_NALLOC;
	struct clprgs_entry* clprgs = (struct clprgs_entry*)
		malloc(__clprgs_entry_sz*clprgs_nalloc);
	unsigned int clprgs_n = 0;

	size_t cltexts_buf_alloc = DEFAULT_BUF_ALLOC;
	char* cltexts_buf = (char*)calloc(1,cltexts_buf_alloc);
	char* cltexts_bufp = cltexts_buf;

	size_t clstrtab_str_alloc = DEFAULT_STR_SIZE;
	char* clstrtab_str = (char*)calloc(1,clstrtab_str_alloc);
	char* clstrtab_strp = clstrtab_str;
	

//	char fullpath[DEFAULT_STR_SIZE];
	char* fullpath = (char*)malloc(DEFAULT_STR_SIZE);


	i = 1;
	while (i < argc) {

      if (!strcmp(argv[i],"--cl-binary")) {

			sw_src_bin = 1;

      } else if (!strcmp(argv[i],"--cl-source")) {

			sw_src_bin = 0;

      } else if (!strcmp(argv[i],"--cl-platform")) {

			platform = argv[++i];

      } else if (!strcmp(argv[i],"--cl-device")) {

			device = argv[++i];

		} else if (!strcmp(argv[i],"--cl-path")) {

			add_path(path_str,&path_str_len,argv[++i]);

		} else if (!strcmp(argv[i],"-h")||!strcmp(argv[i],"--help")) {

			usage(); 
			exit(0);

		} else if (!strcmp(argv[i],"-v")||!strcmp(argv[i],"--version")) {

			version(); 
			exit(0);

		} else {

			switch (sw_src_bin) {

				case 0:	/* add file to srctab */

					sp = (struct srctab_entry*)malloc(sizeof(struct srctab_entry));
					sp->filename = argv[i];
					sp->platform_type = 0;
					sp->device_type = 0;
					LIST_INSERT_HEAD(&srctab_head, sp, srctab_list);
					break;

				case 1:	/* add file to bintab */

					bp = (struct bintab_entry*)malloc(sizeof(struct bintab_entry));
					bp->filename = argv[i];
					bp->platform_type = 0;
					bp->device_type = 0;
					LIST_INSERT_HEAD(&bintab_head, bp, bintab_list);
					break;

				default:
					break;
			}

//			fprintf(stderr,"unrecognized option: %s\n",argv[i]);

		}

		++i;

	}

//	printf("list all source entries with full path\n");
	for (sp = srctab_head.lh_first; sp != 0; sp = sp->srctab_list.le_next) {

//		printf("|%s|\n",sp->filename);

		if (resolve_fullpath(fullpath,sp->filename,path_str,path_str_len)<0) {
			fprintf(stderr,"clld: error: file not found: %s\n",sp->filename);
			exit(-1);
		}
	
		struct stat st;
		stat(fullpath,&st);
	
		int fd = open(fullpath,O_RDONLY);

		if (fd<0) { exit(-1); }

		if (clprgs_n >= clprgs_nalloc) {
			clprgs_nalloc *= 2;
			clprgs = (struct clprgs_entry*)
				realloc(clprgs,__clprgs_entry_sz*clprgs_nalloc);
		}
		clprgs[clprgs_n].e_name = (intptr_t)(clstrtab_strp-clstrtab_str);
		clprgs[clprgs_n].e_info = 0;
		clprgs[clprgs_n].e_shndx = -1;
		clprgs[clprgs_n].e_offset = (intptr_t)(cltexts_bufp-cltexts_buf);
		clprgs[clprgs_n].e_size = st.st_size;
		++clprgs_n;

		strncpy(clstrtab_strp,fullpath,DEFAULT_STR_SIZE);
		clstrtab_strp += strnlen(fullpath,DEFAULT_STR_SIZE) + 1;

		size_t offset = (intptr_t)cltexts_bufp-(intptr_t)cltexts_buf;
		size_t n = st.st_size;
		size_t nr;

		if (offset+n>cltexts_buf_alloc) {
			cltexts_buf_alloc *= 2;
			cltexts_buf = realloc(cltexts_buf,cltexts_buf_alloc);
			cltexts_bufp = cltexts_buf + offset;
		}

		while (n>0) {
			if ((nr=read(fd,cltexts_bufp,n))<0) {
				exit(-1);
			}
//			printf("read %d bytes\n",nr);
			n -= nr;
			cltexts_bufp += nr;
		}

		close(fd);

	}
	
//	printf("\nlist all binary entries\n");
//	for (bp = bintab_head.lh_first; bp != 0; bp = bp->bintab_list.le_next)
//		printf("%s ",bp->filename);


	/*
	 * create special elf file
	 */

	int fd;
	Elf* e;
	Elf_Scn* scn;
	Elf_Data* data;

	if (elf_version(EV_CURRENT) == EV_NONE) {
		fprintf(stderr,"clld: library initialization failed: %s",elf_errmsg(-1));
		exit(-1);
	}

	if ((fd = open("out_clld.o", O_WRONLY|O_CREAT, 0777)) < 0) {
		fprintf(stderr,"open \%s\" failed", "out_clld.o");
		exit(-1);
	}

//	if ((e = elf_begin(fd, ELF_C_WRITE, 0)) == 0) {
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
	ehdr->e_shstrndx = 6; /* set section index of .shstrtab */

#elif defined(__i386__) 

	if ((ehdr = elf32_newehdr(e)) == 0) {
		fprintf(stderr,"elf32_newehdr() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr->e_machine = EM_386; 
	ehdr->e_type = ET_NONE;
	ehdr->e_shstrndx = 6; /* set section index of .shstrtab */

#endif


	/* 
	 * construct section [1] .clprgs 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
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
	shdr->sh_entsize = __clprgs_entry_sz;


	/* 
	 * construct section [2] .cltexts 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 1;
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


	shdr->sh_name = shstrtab_offset[2];
	shdr->sh_type = SHT_PROGBITS;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = 0;


	/* 
	 * construct section [3] .clprgb 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
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
	 * construct section [4] .cltextb 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 1;
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
	 * construct section [5] .clstrtab 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr,"elf_newscn() failed: %s.",elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.",elf_errmsg(-1)); 
		exit(-1);
	}

//	data->d_align = 1;
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


	shdr->sh_name = shstrtab_offset[5];
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;


	/* 
	 * construct section [6] .shstrtab 
	 */

	if ((scn = elf_newscn(e)) == 0) {
		fprintf(stderr, "elf_newscn() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	if ((data = elf_newdata(scn)) == 0) {
		fprintf(stderr,"elf_newdata() failed: %s.", elf_errmsg(-1));
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


	shdr->sh_name = shstrtab_offset[6]; // 6;
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;



	if (elf_update(e, ELF_C_NULL) < 0)  {
//		fprintf(stderr,"%d\n",elf_errno());
		fprintf(stderr, "elf_update(NULL) failed: %s.", elf_errmsg(-1));
		exit(-1);
	}


	if (elf_update(e, ELF_C_WRITE) < 0)  {
		fprintf(stderr, "elf_update() failed: %s.", elf_errmsg(-1));
		exit(-1);
	}

	elf_end(e);
	close(fd);

	return(0);

}


