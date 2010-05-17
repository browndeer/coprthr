/* compiler.c 
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

#define _GNU_SOURCE
#include <link.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "elf_cl.h"
#include "compiler.h"

struct dummy { char* name; void* addr; };

static int
callback(struct dl_phdr_info *info, size_t size, void *data)
{
	int j;
	struct dummy* dum = (struct dummy*)data;

	if (!strncmp(dum->name,info->dlpi_name,256)) 
		{ dum->addr=(void*)info->dlpi_addr; return(1); }

   return 0;
}


/* XXX on certain failures, program is left in /tmp, the original fork
 * XXX design was to prevent this, put it back  -DAR */

/* XXX this file uses 256 for certain string buffers, potential issue -DAR */

//#define __XCL_TEST

#ifndef INSTALL_INCLUDE_DIR
#define INSTALL_INCLUDE_DIR "/usr/local/browndeer/include"
#endif
#ifndef INSTALL_LIB_DIR
#define INSTALL_LIB_DIR "/usr/local/browndeer/lib"
#endif

#define NMFILTER "grep -v -e barrier -e get_work_dim -e get_local_size -e get_local_id -e get_num_groups -e get_global_size -e get_group_id -e get_global_id"

/*

use ELF as a container.  build creates the ELF object and returns pointer.
use following sections.

.cldev general list of devices, possibly redundant, and the index devnum
			can later be used as reference to a specific device
			presently not used, build is called per device 


.clprgs { e_name, e_info, e_shndx, e_offset, e_size }
.cltexts raw source text

.clprgb { e_name, e_info, e_shndx, e_offset, e_size }
.cltextb raw binary text

.clsymtab { e_kind, e_type, e_next }

.clstrtab raw string text

__do_build() will build prg by iterating over devices and calling build()
appropriate for a given device.  result is an ELF file containing all
information necessary to build out the prg info.  this makes

*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>

#include "xcl_structs.h"

#define DEFAULT_BUF1_SZ 16384
#define DEFAULT_BUF2_SZ 16384

static char shstrtab[] = { 
	"\0" 
	".cldev\0"		/* section 1, shstrtab offset 1 */ 
	".clprgs\0"		/* section 2, shstrtab offset 8 */ 
	".cltexts\0"	/* section 3, shstrtab offset 16 */ 
	".clprgb\0"		/* section 4, shstrtab offset 25 */ 
	".cltextb\0"	/* section 5, shstrtab offset 33 */ 
	".clsymtab\0"	/* section 6, shstrtab offset 42 */ 
	".clstrtab\0"	/* section 7, shstrtab offset 52 */ 
	".shstrtab\0"	/* section 8, shstrtab offset 62 */ 
};

static int shstrtab_offset[] = { 0,1,8,16,25,33,42,52,62 };



static char* buf1 = 0;
static char* buf2 = 0;
static char* logbuf = 0;

/*
int shell( char* command, char* options, char* output ); 
{

	if (buf1) buf1 = malloc(DEFAULT_BUF1_SZ);

	FILE* fp = popen(command,"r");

	fread(buf,256,1,fp);
	
	int err = pclose(fp);

	return(err);
}
*/

#define CLERROR_BUILD_FAILED -1



#define __writefile(file,filesz,pfile) do { \
	FILE* fp = fopen(file,"w"); \
	DEBUG(__FILE__,__LINE__,"trying to write %d bytes",filesz); \
	if (fwrite(pfile,1,filesz,fp) != filesz) { \
		ERROR(__FILE__,__LINE__,"error: write '%s' failed",file); \
		return((void*)CLERROR_BUILD_FAILED); \
	} \
	fclose(fp); \
	} while(0);


#define __mapfile(file,filesz,pfile) do { \
	int fd = open(file,O_RDONLY); \
	struct stat fst; fstat(fd,&fst); \
	if (fst.st_size == 0 || !S_ISREG(fst.st_mode)) { \
		fprintf(stderr,"error: open failed on '%s'\n",file); \
		return((void*)CLERROR_BUILD_FAILED); \
	} \
	filesz = fst.st_size; \
	pfile = mmap(0,filesz,PROT_READ,MAP_PRIVATE,fd,0); \
	close(fd); \
	} while(0);


#define __command(fmt,...) \
	snprintf(buf1,DEFAULT_BUF1_SZ,fmt,##__VA_ARGS__); 


/* XXX note that logbuf is not protected from overfull, fix this -DAR */
#define __log(p,fmt,...) do { \
	p += snprintf(p,__CLMAXSTR_LEN,fmt,##__VA_ARGS__); \
	} while(0);

/* XXX note that logbuf is not protected from overfull, fix this -DAR */
#define __execshell(command,p) do { \
	char c; \
	DEBUG(__FILE__,__LINE__,"__execshell: %s",command); \
	FILE* fp = popen(command,"r"); \
	while((c=fgetc(fp)) != EOF) *p++ = c; \
	err = pclose(fp); \
	} while(0);


void* compile_x86_64(
	cl_device_id devid,
	unsigned char* src, size_t src_sz, 
	unsigned char* bin, size_t bin_sz, 
	char** opt, char** log 
)
{
	int i;
	int err;
	char c;
	int fd;
	struct stat fst;
	FILE* fp;
	char line[1024];

#ifdef __XCL_TEST
	char wdtemp[] = "./";
	char filebase[] 	= "XXXXXX";
	char* wd = wdtemp;
#else
	char wdtemp[] = "/tmp/xclXXXXXX";
	char filebase[] 	= "XXXXXX";
	char* wd = mkdtemp(wdtemp);
	mktemp(filebase);
#endif

	char file_cl[256];
	char file_ll[256];

	snprintf(file_cl,256,"%s/%s.cl",wd,filebase);
	snprintf(file_ll,256,"%s/%s.ll",wd,filebase);

	size_t filesz_cl = 0;
	size_t filesz_ll = 0;
	size_t filesz_textb = 0;
	size_t filesz_elfcl = 0;

	unsigned char* pfile_cl = 0;
	unsigned char* pfile_ll = 0;
	unsigned char* pfile_textb = 0;
	unsigned char* pfile_elfcl = 0;

	DEBUG(__FILE__,__LINE__,"compile: work dir %s",wd);
	DEBUG(__FILE__,__LINE__,"compile: filebase %s",filebase);

	if (!buf1) buf1 = malloc(DEFAULT_BUF1_SZ);
	if (!buf2) buf2 = malloc(DEFAULT_BUF2_SZ);
	if (!logbuf) logbuf = malloc(DEFAULT_BUF2_SZ);
	size_t buf2_alloc_sz = DEFAULT_BUF2_SZ;
	size_t logbuf_alloc_sz = DEFAULT_BUF2_SZ;

	bzero(buf2,DEFAULT_BUF2_SZ);
	bzero(logbuf,DEFAULT_BUF2_SZ);

	unsigned int nsym;
	unsigned int narg;
	struct clsymtab_entry* clsymtab = 0;
	struct clargtab_entry* clargtab = 0;

	size_t clstrtab_sz;
	size_t clstrtab_alloc_sz;
	char* clstrtab = 0;

		char* p2 = buf2;
		char* logp = logbuf;

		DEBUG(__FILE__,__LINE__,"compile: %p %p",src,bin);

		if (!bin) { /* use source */

			if (src) {

				DEBUG(__FILE__,__LINE__,"compile: build from source");


				/* copy rt objects to work dir */

				__command("cp "INSTALL_LIB_DIR"/__elfcl_rt.o %s",wd);
				__log(logp,"]%s\n",buf1); \
				__execshell(buf1,logp);

				__command("cp "INSTALL_LIB_DIR"/__vcore_rt.bc %s",wd);
				__log(logp,"]%s\n",buf1); \
				__execshell(buf1,logp);

				__command("cp "INSTALL_INCLUDE_DIR"/vcore.h %s",wd);
				__log(logp,"]%s\n",buf1); \
				__execshell(buf1,logp);


				/* write cl file */

				filesz_cl = src_sz;
				pfile_cl = src;
				DEBUG(__FILE__,__LINE__,"compile: writefile %s %d %p",
					file_cl,filesz_cl,pfile_cl);
				__writefile(file_cl,filesz_cl,pfile_cl);

				DEBUG(__FILE__,__LINE__,"%s written\n",buf1);


				/* clc compile */

				__command("cd %s; clc -o __%s.ll %s 2>&1",wd,filebase,file_cl); 
				__log(logp,"]%s\n",buf1); \
				__execshell(buf1,logp);


				/* strip target specification  */
				__command("cd %s;"
					" grep -v -e 'target datalayout' -e 'target triple' __%s.ll"
					" > %s.ll ",wd,filebase,filebase);
				__log(logp,"]%s\n",buf1); \
				__execshell(buf1,logp);


				/* assemble to bc */

				__command("cd %s; llvm-as -f %s.ll 2>&1",wd,filebase); 
				__log(logp,"]%s\n",buf1); \
				__execshell(buf1,logp);


            /* extract undefined symbols from builtins */

            __command(
               "cd %s; llvm-nm %s.bc|" NMFILTER 
					"|awk '/^[ \t]+U / {printf $2 \",\";}'",
               wd,filebase);
            *buf2 = '\0';
            DEBUG(__FILE__,__LINE__,"%s",buf1);
            fp = popen(buf1,"r");
            fgets(buf2,DEFAULT_BUF2_SZ,fp);
            DEBUG(__FILE__,__LINE__,"%s",buf2);
            pclose(fp);
            int need_builtins = (strnlen(buf2,DEFAULT_BUF2_SZ)>0)? 1 : 0;
            DEBUG(__FILE__,__LINE__,"need_builtins %d",need_builtins);
            if (need_builtins) {
               __command(
                  "cd %s;"
                  " llvm-ex -f -func=%s %s/lib/x86_64/builtins_x86-64.bc"
                  " -o builtins.bc",
                  wd,buf2,ATISTREAMSDK);
               __log(logp,"]%s\n",buf1);
               __execshell(buf1,logp);
            }


				/* link with vcore_rt bc */

//				__command(
//					"cd %s; llvm-link -f -o _link_%s.bc %s.bc __vcore_rt.bc 2>&1",
//					wd,filebase,filebase); 
//				__log(logp,"]%s\n",buf1); \
//				__execshell(buf1,logp);


          /* link bc */

//need_builtins = 0;

            if (need_builtins) {
               __command(
                  "cd %s; llvm-ld -b _link_%s.bc %s.bc __vcore_rt.bc"
						" builtins.bc  2>&1",
                  wd,filebase,filebase);
            } else {
               __command(
                  "cd %s; llvm-ld -b _link_%s.bc %s.bc __vcore_rt.bc 2>&1",
                  wd,filebase,filebase);
            }
            __log(logp,"]%s\n",buf1); \
            __execshell(buf1,logp);


				/* inline optimization */

				__command(
					"cd %s; opt -f -std-compile-opts -inline -O3"
					" -o _opt_%s.bc _link_%s.bc 2>&1",
					wd,filebase,filebase); 
				__log(logp,"]%s\n",buf1); \
				__execshell(buf1,logp);


				/* assemble bc to native asm */

				__command("cd %s; llc -O3 -march=x86-64"
					" --relocation-model=pic _opt_%s.bc 2>&1",wd,filebase); 
				__log(logp,"]%s\n",buf1); \
				__execshell(buf1,logp);


				/* assemble to native object */

				__command("cd %s; gcc -O3 -fPIC -g -c _opt_%s.s 2>&1",wd,filebase); 
				__log(p2,"]%s\n",buf1); \
				__execshell(buf1,p2);


				/* generate kcall wrappers */

				__command("cd %s; xclnm --kcall -d -c %s -o _kcall_%s.c 2>&1",
					wd,file_ll,filebase); 
				__log(p2,"]%s\n",buf1); \
				__execshell(buf1,p2);


				/* gcc compile kcall wrappers */

#if defined(USE_FAST_SETJMP)
				__command(
					"cd %s; gcc -g -fPIC -DUSE_FAST_SETJMP -I%s -c _kcall_%s.c 2>&1",
					wd,INSTALL_INCLUDE_DIR,filebase); 
#else
				__command("cd %s; gcc -g -fPIC -I%s -c _kcall_%s.c 2>&1",
					wd,INSTALL_INCLUDE_DIR,filebase); 
#endif
				__log(logp,"]%s\n",buf1); \
				__execshell(buf1,logp);


				DEBUG(__FILE__,__LINE__,
					"log\n"
					"------------------------------------------------------------\n"
					"%s\n"
					"------------------------------------------------------------",
					logbuf);

				logp=logbuf;


				/* now extract arg data */

				DEBUG(__FILE__,__LINE__,"extract arg data");

				__command("cd %s; xclnm -n -d %s",wd,file_ll); 
				fp = popen(buf1,"r");
				fscanf(fp,"%d",&nsym);
				pclose(fp); 

				clsymtab = (struct clsymtab_entry*)
					calloc(nsym,sizeof(struct clsymtab_entry));

				clstrtab_sz = 0;
				clstrtab_alloc_sz = nsym*1024;
				clstrtab = malloc(clstrtab_alloc_sz);
				clstrtab[clstrtab_sz++] = '\0';

				i=0;
				narg = 1;	/* starts at 1 to include the (null_arg) -DAR */
				int ii;
				unsigned char kind;
				char name[256];
				int datatype;
				int vecn;
				int arrn;
				int addrspace;
				int ptrc;
				int n;
				int arg0;
				__command("cd %s; xclnm --clsymtab -d -c %s.ll",wd,filebase);
				fp = popen(buf1,"r");
				while (!feof(fp)) {
					if (fscanf(fp,"%d %c %s %d %d %d %d %d %d %d",
						&ii, &kind, &name, 
						&datatype,&vecn,&arrn,&addrspace,&ptrc,
						&n,&arg0)==EOF) break;
					if (ii!=i) {
						ERROR(__FILE__,__LINE__,"cannot parse output of xclnm");
						exit(-2);
					}

#if defined(__i386__)
					clsymtab[i] = (struct clsymtab_entry){
						(Elf32_Half)clstrtab_sz,
						(Elf32_Half)kind,
						(Elf32_Addr)0,
						(Elf32_Addr)0,
						(Elf32_Half)datatype,
						(Elf32_Half)vecn,
						(Elf32_Half)arrn,
						(Elf32_Half)addrspace,
						(Elf32_Half)ptrc,
						(Elf32_Half)n,
						(Elf32_Half)arg0 };
#elif defined(__x86_64__)
					clsymtab[i] = (struct clsymtab_entry){
						(Elf64_Half)clstrtab_sz,
						(Elf64_Half)kind,
						(Elf64_Addr)0,
						(Elf64_Addr)0,
						(Elf64_Half)datatype,
						(Elf64_Half)vecn,
						(Elf64_Half)arrn,
						(Elf64_Half)addrspace,
						(Elf64_Half)ptrc,
						(Elf64_Half)n,
						(Elf64_Half)arg0 };
#else
#error unsupported ELF format
#endif
					strncpy(&clstrtab[clstrtab_sz],name,256);
					clstrtab_sz += strnlen(name,256) + 1;

					++i;
					narg += n;
				} 
				pclose(fp); 

				clargtab = (struct clargtab_entry*)
					calloc(narg,sizeof(struct clargtab_entry));

				i=0;
				char aname[256];
				int argn;
				__command("cd %s; xclnm --clargtab -d -c %s.ll",wd,filebase);
				fp = popen(buf1,"r");
				while (!feof(fp)) {

					if (fscanf(fp,"%d %s %d %d %d %d %d %d %s ",
						&ii, &aname, 
						&datatype,&vecn,&arrn,&addrspace,&ptrc,
						&argn,&name)==EOF) { break; }

					if (ii!=i) {
						ERROR(__FILE__,__LINE__,"cannot parse output of xclnm");
						exit(-2);
					}

#if defined(__i386__)
					clargtab[i] = (struct clargtab_entry){
						(Elf32_Half)0,
						(Elf32_Half)datatype,
						(Elf32_Half)vecn,
						(Elf32_Half)arrn,
						(Elf32_Half)addrspace,
						(Elf32_Half)ptrc,
						(Elf32_Half)clstrtab_sz,
						(Elf32_Half)argn };
#elif defined(__x86_64__)
					clargtab[i] = (struct clargtab_entry){
						(Elf64_Half)0,
						(Elf64_Half)datatype,
						(Elf64_Half)vecn,
						(Elf64_Half)arrn,
						(Elf64_Half)addrspace,
						(Elf64_Half)ptrc,
						(Elf64_Half)clstrtab_sz,
						(Elf64_Half)argn };
#else
#error unsupported ELF format
#endif

					strncpy(&clstrtab[clstrtab_sz],aname,256);
					clstrtab_sz += strnlen(aname,256) + 1;

					++i;
				} 
				pclose(fp); 

DEBUG(0,0,"HERE");

				if (i!=narg) {
					ERROR(__FILE__,__LINE__,"cannot parse output of xclnm");
					exit(-1);
				}

/* XXX not implemented 
				snprintf(buf1,256,"_opt_%s.o",filebase);
				__mapfile(buf1,filesz_textb,pfile_textb);
				DEBUG(__FILE__,__LINE__,"filesz_textb pfile_textb %d %p\n",
					filesz_textb,pfile_textb);

				struct clprgb_entry clprgb_e = {
					clstrtab_sz,0,5,0,filesz_textb
				};
				strncpy(clstrtab+clstrtab_sz,filebase,strnlen(filebase,256));
				clstrtab_sz += strnlen(filebase,256) + 1;
*/



				/* now build elf/cl object */

				snprintf(buf1,256,"%s/%s.elfcl",wd,filebase);
				int fd = open(buf1,O_WRONLY|O_CREAT,S_IRWXU);
				err = elfcl_write(fd,
					0,0,
					0,0, 0,0,
//					&clprgb_e,1, pfile_textb,filesz_textb,
					0,0, 0,0,
					clsymtab,nsym,
					clargtab,narg,
					clstrtab,clstrtab_sz
				);
				close(fd);

//				__mapfile(buf1,filesz_elfcl,pfile_elfcl);
//				printf("filesz_elfcl pfile_elfcl %d %p\n",filesz_elfcl,pfile_elfcl);


				/* now build .so that will be used for link */

				__command("cd %s; gcc -g -shared -Wl,-soname,%s.so -o %s.so"
					" _opt_%s.o _kcall_%s.o __elfcl_rt.o"
//					" fast_setjmp.o"
					" %s.elfcl 2>&1",
					wd,filebase,filebase,filebase,filebase,filebase);
				__log(p2,"]%s\n",buf1); \
				__execshell(buf1,p2);



			} else {

				/* error no source or binary */

				WARN(__FILE__,__LINE__,"compile: no source or binary");

			}

		} else {

			DEBUG(__FILE__,__LINE__,"compile: build from binary");

			/* test binary */

			/* test size of binary, error if implausibly small */

			if (filesz_ll < 8) return((void*)CLERROR_BUILD_FAILED);

			/* test if BC */

			if (bin[0]=='B' && bin[1]=='C') {}

			/* test if ELF */

			if (bin[1]=='E' && bin[2]=='L' && bin[3]=='F') {}

		}

		/* assume LL */

		/*
		 * xclnm .ll > .clsymtab
		 * llvm-as .bc -> .o
		 * cc .o > .so
		 */

	snprintf(buf1,256,"%s/%s.so",wd,filebase);
	dlerror();
	void* h = dlopen(buf1,RTLD_LAZY);
	char* dlerr = dlerror();
	if (dlerr) fprintf(stderr,"%s\n",dlerr);
	DEBUG(__FILE__,__LINE__,"dlopen handle %p\n",h);


	/* XXX problems with mmap protections, mark entire .so as READ|EXEC 
	 * XXX and note that this is reckless, be more precise later -DAR */
	struct dummy dum;
	dum.name = buf1;

	int rt = dl_iterate_phdr(callback, &dum);

	if (rt) printf("matching baseaddr %p\n",dum.addr);

	{
	struct stat fst; stat(buf1,&fst);
   size_t sz = fst.st_size; 
	size_t page_mask = ~(getpagesize()-1);
	intptr_t p = ((intptr_t)dum.addr)&page_mask;
	sz += (size_t)((intptr_t)dum.addr-p);
	mprotect((void*)p,sz,PROT_READ|PROT_EXEC);
	}
	
	return(h);

}





