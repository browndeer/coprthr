/* compiler_e32_blank.c 
 *
 * Copyright (c) 2009-2012 Brown Deer Technology, LLC.  All Rights Reserved.
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

#include "xcl_config.h"
//#include "e32_config.h"

#ifdef LIBCOPRTHR_CC
#define CC_COMPILER LIBCOPRTHR_CC
#else
#define CC_COMPILER " gcc "
#endif

#ifdef LIBCOPRTHR_CXX
#define CXX_COMPILER LIBCOPRTHR_CXX
#else
#define CXX_COMPILER " g++ "
#endif

#define CCFLAGS_OCL_O2 \
	" -fthread-jumps -fcrossjumping -foptimize-sibling-calls " \
	" -fcse-follow-jumps  -fcse-skip-blocks -fgcse -fgcse-lm  " \
	" -fexpensive-optimizations -fstrength-reduce " \
	" -frerun-cse-after-loop -frerun-loop-opt -fcaller-saves " \
	" -fpeephole2 -fschedule-insns -fschedule-insns2 " \
	" -fsched-interblock  -fsched-spec -fregmove " \
	" -fstrict-aliasing -fdelete-null-pointer-checks " \
	" -freorder-blocks -freorder-functions " \
	" -falign-functions -falign-jumps -falign-loops  -falign-labels " \
	" -ftree-vrp -ftree-pre" 

#define XXX_GCC_HACK_FLAG " -fschedule-insns -fschedule-insns2"

/* XXX note that most flags suposedly enabled by -O2 are added explicitly
 * XXX for CCFLAGS_OCL because this inexplicably improves performance by 2%.
 * XXX the primary issue seems to be -fschedule-insns -fschedule-insns2 .
 * XXX also, do not raise CCFLAGS_KCALL, effect is to break everything. -DAR */

//#define CCFLAGS_OCL " -O2 -msse3 " CCFLAGS_OCL_O2
#define CCFLAGS_OCL " -fno-exceptions -O3 -msse3 -funsafe-math-optimizations -fno-math-errno -funsafe-math-optimizations " XXX_GCC_HACK_FLAG 
#define CCFLAGS_KCALL " -O0 "
#define CCFLAGS_LINK 

#define ECC_COMPILER " e-gcc "
#define EOBJCOPY " e-objcopy "

//#define ECCFLAGS_OCL " -O3 -g -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -funroll-loops -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=round-nearest -w -fstack-usage -fno-exceptions -U_FORTIFY_SOURCE -fno-stack-protector "
#define ECCFLAGS_OCL " -O0 -g -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -funroll-loops -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=truncate -falign-loops=8 -falign-functions=8 -w -fstack-usage -fno-exceptions -U_FORTIFY_SOURCE -fno-stack-protector -I/usr/local/adapteva/elib -fpermissive -D__coprthr_device__ "

#define ECCFLAGS_KCALL " -O0 -g -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -funroll-loops -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=round-nearest -w -fstack-usage -fno-exceptions -U_FORTIFY_SOURCE -fno-stack-protector -I/usr/local/adapteva/elib -fpermissive -D__coprthr_device__ "

#define ECCFLAGS_LINK -lm

#define ECC_BLOCKED_FLAGS "-D_FORTIFY_SOURCE", "-fexceptions", \
	"-fstack-protector-all" "-fstack-protector-all"


#define _GNU_SOURCE
#include <link.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

#include "elf_cl.h"
#include "printcl.h"
#include "compiler.h"

#include "dmalloc.h"

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

//#define NMFILTER "grep -v -e barrier -e get_work_dim -e get_local_size -e get_local_id -e get_num_groups -e get_global_size -e get_group_id -e get_global_id -e __read_imagei_image2d2i32"

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
#define DEFAULT_BUILD_LOG_SZ 256


static char* buf1 = 0;
static char* buf2 = 0;
static char* logbuf = 0;


#define CLERROR_BUILD_FAILED -1


#if(0)
#define __writefile(file,filesz,pfile) do { \
	FILE* fp = fopen(file,"w"); \
	fprintf(fp,"#include \"opencl_lift.h\"\n"); \
	printcl( CL_DEBUG "trying to write %d bytes",filesz); \
	if (fwrite(pfile,1,filesz,fp) != filesz) { \
		ERROR(__FILE__,__LINE__,"error: write '%s' failed",file); \
		return((void*)CLERROR_BUILD_FAILED); \
	} \
	fprintf(fp,"/* end */\n"); \
	fclose(fp); \
	} while(0);

//	fprintf(fp,"#include \"opencl_lift.h\"\nextern \"C\" {\n"); 

#define __writefile_cpp(file,filesz,pfile) do { \
	FILE* fp = fopen(file,"w"); \
	fprintf(fp,"extern \"C\" { "); \
	printcl( CL_DEBUG "trying to write %d bytes",filesz); \
	if (fwrite(pfile,1,filesz,fp) != filesz) { \
		ERROR(__FILE__,__LINE__,"error: write '%s' failed",file); \
		return((void*)CLERROR_BUILD_FAILED); \
	} \
	fprintf(fp,"\n}\n"); \
	fprintf(fp,"/* end */\n"); \
	fclose(fp); \
	} while(0);
#endif
#define __writefile(file,filesz,pfile) do { \
	FILE* fp = fopen(file,"w"); \
	printcl( CL_DEBUG "trying to write %d bytes",filesz); \
	printcl( CL_DEBUG "last char is (int)%d",(int)pfile[filesz-1]); \
	if (pfile[filesz-1] == '\0') { \
		fprintf(fp,pfile); \
	} else if (fwrite(pfile,1,filesz,fp) != filesz) { \
		ERROR(__FILE__,__LINE__,"error: write '%s' failed",file); \
		return((void*)CLERROR_BUILD_FAILED); \
	} \
	fprintf(fp,"/* end */\n"); \
	fclose(fp); \
	} while(0);

#define __writefile_cpp(file,filesz,pfile) do { \
	FILE* fp = fopen(file,"w"); \
	fprintf(fp,"#include \"opencl_lift.h\"\nextern \"C\" {\n"); \
	printcl( CL_DEBUG "trying to write %d bytes",filesz); \
	if (pfile[filesz-1] == '\0') { \
		fprintf(fp,pfile); \
	} else if (fwrite(pfile,1,filesz,fp) != filesz) { \
		ERROR(__FILE__,__LINE__,"error: write '%s' failed",file); \
		return((void*)CLERROR_BUILD_FAILED); \
	} \
	fprintf(fp,"\n}\n"); \
	fprintf(fp,"/* end */\n"); \
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
	int c; \
	printcl( CL_DEBUG "__execshell: %s",command); \
	FILE* fp = popen(command,"r"); \
	while((c=fgetc(fp)) != EOF) *p++ = (char)c; \
	err = pclose(fp); \
	} while(0);

static void __remove_work_dir(char* wd)
{
	char fullpath[256];
	DIR* dirp = opendir(wd);
	struct dirent* dp;
	while ( (dp=readdir(dirp)) ) {
		if (strncmp(dp->d_name,".",2) || strncmp(dp->d_name,"..",3)) {
			strncpy(fullpath,wd,256);
			strncat(fullpath,"/",256);
			strncat(fullpath,dp->d_name,256);
			printcl( CL_DEBUG "removing '%s'",fullpath);
			unlink(fullpath);
		}
	}
	printcl( CL_DEBUG "removing '%s'",wd);
	rmdir(wd);
}


static void __append_str( char** pstr1, char* str2, char* sep, size_t n )
{
   if (!*pstr1 || !str2) return;

   size_t len = strlen(str2);

   if (sep) {
      *pstr1 = (char*)realloc(*pstr1,strlen(*pstr1)+len+2);
      strcat(*pstr1,sep);
   } else {
      *pstr1 = (char*)realloc(*pstr1,strlen(*pstr1)+len+1);
   }
   strncat(*pstr1,str2, ((n==0)?len:n) );

}

#define __check_err( err, msg ) do { if (err) { \
	__append_str(log,msg,0,0); \
	if (!coprthr_tmp) { \
		printcl( CL_DEBUG "removing work directory"); \
		__remove_work_dir(wd); \
	} \
	return(CL_BUILD_PROGRAM_FAILURE); \
} }while(0)

static int __test_file( char* file ) 
{
	struct stat s;
	int err = stat(file,&s);
	printcl( CL_DEBUG "__test_file '%s' %d",file,err);
	return (stat(file,&s));
}

#define __shell_command( fmt, ... ) do { \
	__command(fmt,##__VA_ARGS__); \
	__log(logp,"]%s\n",buf1); \
	__execshell(buf1,logp); \
	} while(0)

#define __assert_file( fmt, ... ) do { \
      snprintf(fullpath,256,fmt,##__VA_ARGS__); \
      __check_err(__test_file(fullpath), \
     		"compiler_x86_64: internal error: assert file failed "); \
	} while(0)


static char* ecc_block_flags[] = { ECC_BLOCKED_FLAGS };


int compile_e32_blank(
	cl_device_id devid,
	unsigned char* src, size_t src_sz, 
	unsigned char** p_bin, size_t* p_bin_sz, 
	char* opt_in, char** log 
)
{
	int i;
	int err;
	char c;
	int fd;
	struct stat fst;
	FILE* fp;
	char line[1024];

	char default_opt[] = "";
	char* opt = (opt_in)? strdup(opt_in) : strdup(default_opt);

	printcl( CL_DEBUG "opt |%s|",opt);

	for(i=0;i<sizeof(ecc_block_flags)/sizeof(char*);i++) {
		char* p;
		while (p=strstr(opt,ecc_block_flags[i])) {
			memset(p,' ',strlen(ecc_block_flags[i]));
			printcl( CL_WARNING "blocked compiler option '%s'",
				ecc_block_flags[i]);
			printcl( CL_DEBUG "new opt |%s|",opt);
		}
	}

	printcl( CL_DEBUG "opt after filter |%s|",opt);

//#ifdef __XCL_TEST
//	char wdtemp[] = "./";
//	char filebase[] 	= "XXXXXX";
//	char* wd = wdtemp;
//#else

	char* coprthr_tmp = getenv("COPRTHR_TMP");
	if (stat(coprthr_tmp,&fst) || !S_ISDIR(fst.st_mode) 
		|| fst.st_mode & S_IRWXU != S_IRWXU) coprthr_tmp = 0;

	char* wdtemp;

	if (coprthr_tmp) {
		wdtemp = (char*)malloc(strlen(coprthr_tmp) +11);
		sprintf(wdtemp,"%s/xclXXXXXX",coprthr_tmp);
	} else {
		wdtemp = strdup("/tmp/xclXXXXXX");
	}

//	char wdtemp[] = "/tmp/xclXXXXXX";

	char filebase[] 	= "XXXXXX";
	char* wd = mkdtemp(wdtemp);
	mktemp(filebase);
//#endif

	char file_cl[256];
	char file_cpp[256];
	char file_ll[256];
	char fullpath[256];

	snprintf(file_cl,256,"%s/%s.cl",wd,filebase);
	snprintf(file_cpp,256,"%s/%s.cpp",wd,filebase);
	snprintf(file_ll,256,"%s/%s.ll",wd,filebase);

	size_t filesz_cl = 0;
	size_t filesz_ll = 0;
	size_t filesz_textb = 0;
	size_t filesz_elfcl = 0;

	unsigned char* pfile_cl = 0;
	unsigned char* pfile_ll = 0;
	unsigned char* pfile_textb = 0;
	unsigned char* pfile_elfcl = 0;

	printcl( CL_DEBUG "compile: work dir %s",wd);
	printcl( CL_DEBUG "compile: filebase %s",filebase);

	if (!buf1) buf1 = malloc(DEFAULT_BUF1_SZ);
	if (!buf2) buf2 = malloc(DEFAULT_BUF2_SZ);
	if (!logbuf) logbuf = malloc(DEFAULT_BUF2_SZ);
	size_t buf2_alloc_sz = DEFAULT_BUF2_SZ;
	size_t logbuf_alloc_sz = DEFAULT_BUF2_SZ;

	bzero(buf2,DEFAULT_BUF2_SZ);
	bzero(logbuf,DEFAULT_BUF2_SZ);

	*log = (char*)malloc(DEFAULT_BUF2_SZ);
   (*log)[0] = '\0';

	unsigned int nsym;
	unsigned int narg;
	struct clsymtab_entry* clsymtab = 0;
	struct clargtab_entry* clargtab = 0;

	size_t clstrtab_sz;
	size_t clstrtab_alloc_sz;
	char* clstrtab = 0;

	char* p2 = buf2;
	char* logp = logbuf;
	char* p2_prev;

	printcl( CL_DEBUG "compile: %p",src);

	/* with cltrace LD_PRELOAD env var is problem so just prevent intercepts */
	unsetenv("LD_PRELOAD");

	if (src) {

		printcl( CL_DEBUG "compile: build from source");


		/* copy files to work dir */

#if defined(USE_E32SER)
		char* objs[] = { "_e32ser_core_main.o", "_e32ser_core.ldf" };
#elif defined(USE_E32SL)
		char* objs[] = { "_e32sl_core_main.o", "_e32sl_core.ldf" };
#elif defined(USE_E32PTH)
		char* objs[] = { "_e32pth_core_main_blank.o", 
			"_e32pth_core_blank.ldf" };
#else
#error 
#endif

		for(i=0;i<sizeof(objs)/sizeof(char*);i++) {
			__command(
				"\\cp "INSTALL_LIB_DIR"/%s %s ",objs[i],wd);
			__log(logp,"]%s\n",buf1);
			__execshell(buf1,logp);
			snprintf(fullpath,256,"%s/%s",wd,objs[i]);
			__check_err(__test_file(fullpath),
				"compiler_x86_64: internal error: obj copy failed.");
		}


		/* write cl file */

		filesz_cl = src_sz;
		pfile_cl = src;
		printcl( CL_DEBUG "compile: writefile %s %d %p",
			file_cl,filesz_cl,pfile_cl);
		__writefile(file_cl,filesz_cl,pfile_cl);
		printcl( CL_DEBUG "%s written\n",buf1);
		__check_err(__test_file(file_cl),
			"compiler_x86_64: internal error: write file cl failed.");

		printcl( CL_DEBUG "compile: writefile_cpp %s %d %p",
			file_cpp,filesz_cl,pfile_cl);
		__writefile_cpp(file_cpp,filesz_cl,pfile_cl);
		printcl( CL_DEBUG "%s written\n",buf1);
		__check_err(__test_file(file_cpp),
			"compiler_x86_64: internal error: write file cpp failed.");


		/* assemble to native object */

		__command(
			"cd %s; "
			ECC_COMPILER ECCFLAGS_OCL 
			" -I" INSTALL_INCLUDE_DIR 
			" -D __xcl_kthr__ --include=e32pth_if_blank.h "
			" --include=opencl_lift.h "
#ifdef USE_E32_OPENCL_EXT
			" --include=e32_opencl_ext.h "
#endif
			" -D __STDCL_KERNEL_VERSION__=020000 -D__COPRTHR__"
			" -D E32_DRAM_ZEROPAGE=0x%x "
			" %s "
			" -S %s.cpp 2>&1",
			wd,devmembase,opt,filebase); 
		__log(p2,"]%s\n",buf1);
		p2_prev = p2;
		__execshell(buf1,p2);
		if (p2 != p2_prev) __append_str(log,p2_prev,0,0);
		snprintf(fullpath,256,"%s/%s.s",wd,filebase);
		__check_err(__test_file(fullpath),
			"compiler_x86_64: error: kernel compilation failed.");



		__command(
			"cd %s; "
			ECC_COMPILER ECCFLAGS_OCL 
			" -I" INSTALL_INCLUDE_DIR 
			" -D __xcl_kthr__ --include=e32pth_if_blank.h "
#ifdef USE_E32_OPENCL_EXT
			" --include=e32_opencl_ext.h "
#endif
			" -D __STDCL_KERNEL_VERSION__=020000"
			" -D E32_DRAM_ZEROPAGE=0x%x "
			" %s "
			" -c %s.cpp -o e32_%s.o 2>&1",
			wd,devmembase,opt,filebase,filebase); 
		__log(p2,"]%s\n",buf1);
		p2_prev = p2;
		__execshell(buf1,p2);
		if (p2 != p2_prev) __append_str(log,p2_prev,0,0);
		snprintf(fullpath,256,"%s/e32_%s.o",wd,filebase);
		__check_err(__test_file(fullpath),
			"compiler_x86_64: error: kernel compilation failed.");

		printcl( CL_DEBUG "devmembase = %x", devmembase);



//		printcl( CL_WARNING "e-gcc has been silenced for all warnings" );


		/* generate kcall wrappers */

		char* wrappers[] = { "kcall", "kcall2", "kcall3" };

		for(i=0;i<sizeof(wrappers)/sizeof(char*);i++) {

			__shell_command( "cd %s;"
				" cpp -x c++ -I" INSTALL_INCLUDE_DIR " %s "
				" | awk -v prog=\\\"%s\\\" "
				"'BEGIN { pr=0; }"
				" { "
				"   if($0~/^#/ && $3==prog) pr=1;"
				"   else if ($0~/^#/) pr=0;"
				"   if ($0!~/^#/ && pr==1) print $0;"
				" }' | xclnm --%s -d -c -o _%s_%s.c - 2>&1",
				wd,file_cl,file_cl,wrappers[i],wrappers[i],filebase);

			printcl( CL_DEBUG "%s", buf1 );

			__assert_file( "%s/_%s_%s.c",wd,wrappers[i],filebase );
		}	

		/* gcc compile kcall wrappers */

#if defined(__FreeBSD__)
		__command("cd %s; gcc -O0 -fPIC"
			" -D__xcl_kcall__ -DE32_DRAM_ZEROPAGE=0x%x -I%s --include=sl_engine.h -c _kcall_%s.c 2>&1",
			wd,devmembase,INSTALL_INCLUDE_DIR,filebase); 
#else
		__command(
			"cd %s; "
			CC_COMPILER CCFLAGS_KCALL " -fPIC"
			" -D__xcl_kcall__ -DE32_DRAM_ZEROPAGE=0x%x -I%s --include=sl_engine.h -c _kcall_%s.c 2>&1",
			wd,devmembase,INSTALL_INCLUDE_DIR,filebase); 
#endif
		__log(logp,"]%s\n",buf1);
		__execshell(buf1,logp);
		snprintf(fullpath,256,"%s/_kcall_%s.o",wd,filebase);
		__check_err(__test_file(fullpath),
			"compiler_x86_64: internal error: kcall wrapper compilation failed.");

#if defined(__FreeBSD__)
		__command("cd %s; gcc -O0 -fPIC"
			" -D__xcl_kcall__ -DE32_DRAM_ZEROPAGE=0x%x -I%s --include=ser_engine.h -c _kcall2_%s.c 2>&1",
			wd,devmembase,INSTALL_INCLUDE_DIR,filebase); 
#else
		__command(
			"cd %s; "
			CC_COMPILER CCFLAGS_KCALL " -fPIC"
			" -D__xcl_kcall__ -DE32_DRAM_ZEROPAGE=0x%x -I%s --include=ser_engine.h -c _kcall2_%s.c 2>&1",
			wd,devmembase,INSTALL_INCLUDE_DIR,filebase); 
#endif
		__log(logp,"]%s\n",buf1);
		__execshell(buf1,logp);
		snprintf(fullpath,256,"%s/_kcall2_%s.o",wd,filebase);
		__check_err(__test_file(fullpath),
			"compiler_x86_64: internal error: kcall2 wrapper compilation failed.");

/****************/
/****************/
/****************/


		printcl( CL_DEBUG "compile kcall wrapper");

		__shell_command( "cd %s; " 
        	ECC_COMPILER ECCFLAGS_KCALL 
			" -DE32_DRAM_ZEROPAGE=0x%x -I" INSTALL_INCLUDE_DIR
        	" -D__xcl_kcall__ --include=e32pth_if_blank.h" 
			" -c _kcall3_%s.c -o e32_kcall3_%s.o 2>&1",
			wd,devmembase,filebase,filebase);
		__assert_file( "%s/e32_kcall3_%s.o",wd,filebase );

		__command(
			"cd %s;"
			" e-ld -r -o e32.o"
			" _e32pth_core_main_blank.o e32_%s.o e32_kcall3_%s.o"
			" -L/usr/local/adapteva/elib -lelib"
			" 2>&1",
			wd,filebase,filebase);
		__log(p2,"]%s\n",buf1); 
		p2_prev = p2;
		__execshell(buf1,p2);
		if (p2 != p2_prev) __append_str(log,p2_prev,0,0);
		snprintf(fullpath,256,"%s/e32.o",wd,filebase);
		__check_err(__test_file(fullpath),
			"compiler_x86_64: error: e32 prog link failed.");

			printcl( CL_DEBUG "link core prog for all cores");

			__shell_command( "cd %s; " 
         	ECC_COMPILER " -o e32.0.elf"
				" "
				" e32.o "
				" -T _e32pth_core_blank.ldf -lm;"
				EOBJCOPY " --srec-forceS3 --output-target srec"
				" e32.0.elf e32.0.srec", wd);
			__assert_file( "%s/e32.0.srec",wd );

		size_t sz = DEFAULT_BUF1_SZ;
		char* p = buf1 + snprintf(buf1,sz,"cd %s; cat ",wd);
		sz -= (intptr_t)(p - buf1);

			p += snprintf(p,sz," e32.0.srec");
			sz -= (intptr_t)(p - buf1);

		p += snprintf(p,sz," > e32.srec");
		sz -= (intptr_t)(p - buf1);

		__execshell(buf1,logp);

		__assert_file( "%s/e32.srec",wd );
		

/****************/
/****************/
/****************/

		printcl( CL_DEBUG 
			"log\n"
			"------------------------------------------------------------\n"
			"%s\n"
			"------------------------------------------------------------",
			logbuf);

		logp=logbuf;


		/* now extract sym arg data */

		printcl( CL_DEBUG "extract sym and arg data");

		__command("cd %s;"
			"cpp -x c++ -I" INSTALL_INCLUDE_DIR " %s %s "
			" | awk -v prog=\\\"%s\\\" "
			"'BEGIN { pr=0; }"
            " { "
            "   if($0~/^#/ && $3==prog) pr=1;"
            "   else if ($0~/^#/) pr=0;"
            "   if ($0!~/^#/ && pr==1) print $0;"
            " }' | xclnm -n -d - ",wd,file_cl,opt,file_cl); 

		printcl( CL_DEBUG "%s", buf1);

		fp = popen(buf1,"r");
		fscanf(fp,"%d",&nsym);
		pclose(fp); 

		clsymtab 
			= (struct clsymtab_entry*)calloc(nsym,sizeof(struct clsymtab_entry));

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

		__command("cd %s; "
			" cpp -x c++ -I" INSTALL_INCLUDE_DIR " %s %s "
			" | awk -v prog=\\\"%s\\\" "
			"'BEGIN { pr=0; }"
            " { "
            "   if($0~/^#/ && $3==prog) pr=1;"
            "   else if ($0~/^#/) pr=0;"
            "   if ($0!~/^#/ && pr==1) print $0;"
            " }' | xclnm --clsymtab -d -c - ",wd,file_cl,opt,file_cl); 

		printcl( CL_DEBUG "%s", buf1 );

		fp = popen(buf1,"r");
		while (!feof(fp)) {
			if (fscanf(fp,"%d %c %s %d %d %d %d %d %d %d",
				&ii, &kind, &name, 
				&datatype,&vecn,&arrn,&addrspace,&ptrc,
				&n,&arg0)==EOF) break;
			if (ii!=i) {
				printcl( CL_ERR "cannot parse output of xclnm");
				__append_str(log,
					"compiler_x86_64: internal error: cannot parse output of xclnm",
					0,0);
				if (!coprthr_tmp) {
					printcl( CL_DEBUG "removing work directory");
					__remove_work_dir(wd);
				}
				return(CL_BUILD_PROGRAM_FAILURE);
			}

#if defined(__i386__) || defined(__arm__)
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
				(Elf32_Half)arg0,
				(Elf32_Word)0 };
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
				(Elf64_Half)arg0,
				(Elf64_Word)0 };
#else
#error unsupported ELF format
#endif
			strncpy(&clstrtab[clstrtab_sz],name,256);
			clstrtab_sz += strnlen(name,256) + 1;

			++i;
			narg += n;
		} 
		pclose(fp); 

/**************/
/**************/
/**************/

		unsigned int addr;

		unsigned int addr_core_local_data = 0;


			__command("cd %s; nm --defined-only e32.0.elf",wd);
			fp = popen(buf1,"r");
			while (!feof(fp)) {
				addr = ~0;
				if (fscanf(fp,"%x %c %s", &addr, &kind, &name)==EOF) break;
			
				if (addr == ~0) {
					break;
					printcl( CL_ERR "cannot parse output of xclnm");
					__append_str(log, "compiler_x86_64: internal error:"
						" cannot parse output of xclnm", 0,0);
					if (!coprthr_tmp) {
						printcl( CL_DEBUG "removing work directory");
						__remove_work_dir(wd);
					}
					return(CL_BUILD_PROGRAM_FAILURE);
				}

				if (!strcmp(name,"_core_local_data")) {
					if (!addr_core_local_data) addr_core_local_data = addr;
					else if (addr_core_local_data != addr) 
						printcl( CL_ERR "addr_core_local_data mismatch");
					continue;
				}

				for(i=0;i<nsym;i++) {

					char kcall3_name[1024];
					strncpy(kcall3_name,"__XCL_call_",1024);
					strncat(kcall3_name,clstrtab+clsymtab[i].e_name,1024);
					if (name[0]=='_'&&!strcmp(name+1,kcall3_name)) {
						if (!clsymtab[i].e_sym) {
							clsymtab[i].e_sym = addr;
							printcl( CL_DEBUG "set sym '%s' 0x%x",
								clstrtab + clsymtab[i].e_name,addr);
						} else if (clsymtab[i].e_sym != addr) {
							printcl( CL_ERR "sym '%s' mismatch 0x%x 0x%x",
                        clstrtab + clsymtab[i].e_name,clsymtab[i].e_sym,addr);
						}
						break;
					}
				}
			} 
			pclose(fp);


		if (!addr_core_local_data) {
         printcl( CL_ERR "addr_core_local_data not found");
			return(CL_BUILD_PROGRAM_FAILURE);
		} else {
			printcl( CL_DEBUG "addr_core_local_data = 0x%x",
				addr_core_local_data);
		}

		snprintf(fullpath,256,"%s/%s.su",wd,filebase);
	
		if (!__test_file(fullpath)) {
			char name[256];
			int su;
			__command("cd %s; sed 's/(.*)//' %s.su |awk '{print $2 \" \" $3}'",
				wd,filebase);
			fp = popen(buf1,"r");
			while (!feof(fp)) {

				if (fscanf(fp,"%s %d",&name,&su)==EOF) break;
				printcl( CL_DEBUG "stack usage: '%s' %d\n",name,su);

				for(i=0;i<nsym;i++) {

					char thr_name[1024];

					if (!strcmp(name,clstrtab + clsymtab[i].e_name)) {

						clsymtab[i].e_su = su;

						printcl( CL_DEBUG "setting su for '%s' to %d",name,su);

						break;
					}

				}

			}
			pclose(fp); 


		} else {
			printcl( CL_WARNING 
				"no .su file generated, no stack usage information avaialble");
		}	

		snprintf(fullpath,256,"%s/e32_kcall3_%s.su",wd,filebase);
	
		if (!__test_file(fullpath)) {
			char name[256];
			int su;
			__command(
				"cd %s; awk -F: '{print $4}' e32_kcall3_%s.su"
				" | awk '{print $1 \" \" $2}'",
				wd,filebase);
			fp = popen(buf1,"r");
			while (!feof(fp)) {

				if (fscanf(fp,"%s %d",&name,&su)==EOF) break;
				printcl( CL_DEBUG "stack usage: '%s' %d\n",name,su);

				for(i=0;i<nsym;i++) {

					char kcall3_name[1024];
					strncpy(kcall3_name,"__XCL_call_",1024);
					strncat(kcall3_name,clstrtab+clsymtab[i].e_name,1024);

					if (!strcmp(name,kcall3_name)) {

						clsymtab[i].e_su += su;

						printcl( CL_DEBUG "adding %d su for '%s'",su,name);

						break;
					}

				}

			}
			pclose(fp); 


		} else {
			printcl( CL_WARNING 
				"no .su file generated, no stack usage information avaialble");
		}	

/**************/
/**************/
/**************/

		/*** build clargtab ***/

		clargtab = (struct clargtab_entry*)
			calloc(narg,sizeof(struct clargtab_entry));

		i=0;
		char aname[256];
		int argn;

		__command("cd %s;"
				"cpp -x c++ -I" INSTALL_INCLUDE_DIR " %s %s "
				" | awk -v prog=\\\"%s\\\" "
				"'BEGIN { pr=0; }"
            " { "
            "   if($0~/^#/ && $3==prog) pr=1;"
            "   else if ($0~/^#/) pr=0;"
            "   if ($0!~/^#/ && pr==1) print $0;"
            " }' | xclnm --clargtab -d -c - ",wd,file_cl,opt,file_cl); 

		printcl( CL_DEBUG "%s",buf1);

		fp = popen(buf1,"r");

		printcl( CL_DEBUG "fp = %d",fp);

		while (!feof(fp)) {

			if (fscanf(fp,"%d %s %d %d %d %d %d %d %s ",
				&ii, &aname, 
				&datatype,&vecn,&arrn,&addrspace,&ptrc,
				&argn,&name)==EOF) { break; }

			if (ii!=i) {
				printcl( CL_ERR "cannot parse output of xclnm");
				__append_str(log,
					"compiler_x86_64: internal error: cannot parse output of xclnm",
					0,0);
				__remove_work_dir(wd);
				return(CL_BUILD_PROGRAM_FAILURE);
				if (!coprthr_tmp) {
					printcl( CL_DEBUG "removing work directory");
					__remove_work_dir(wd);
				}
			}

			printcl( CL_DEBUG "%d %s %d %d %d %d %d %d %s",
				ii, aname,
            datatype,vecn,arrn,addrspace,ptrc,
            argn,name);

#if defined(__i386__) || defined(__arm__)
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

		if (i!=narg) {
			ERROR(__FILE__,__LINE__,"cannot parse output of xclnm");
			__append_str(log,
				"compiler_x86_64: internal error: cannot parse output of xclnm",
				0,0);
			__remove_work_dir(wd);
			return(CL_BUILD_PROGRAM_FAILURE);
				if (!coprthr_tmp) {
					printcl( CL_DEBUG "removing work directory");
					__remove_work_dir(wd);
				}
		}


		/* now build elf/cl object */

		snprintf(buf1,256,"%s/%s.elfcl",wd,filebase);
		int fd = open(buf1,O_WRONLY|O_CREAT,S_IRWXU);
		err = elfcl_write(fd,
			0,0,
			0,0, 0,0,
			0,0, 0,0,
			clsymtab,nsym,
			clargtab,narg,
			clstrtab,clstrtab_sz
		);
		close(fd);
		__check_err(err, "compiler_x86_64: internal error: elfcl_write failed.");


		/* now build .so that will be used for link */

		__command(
			"cd %s;"
//			" objcopy -I binary -O elf64-x86-64 -B i386 e32.srec data_srec_e32.o"
			" objcopy -I binary -O elf32-littlearm -B arm e32.srec data_srec_e32.o"
			" 2>&1",
			wd,filebase,filebase);
		__log(p2,"]%s\n",buf1); 
		p2_prev = p2;
		__execshell(buf1,p2);
		if (p2 != p2_prev) __append_str(log,p2_prev,0,0);
		snprintf(fullpath,256,"%s/data_srec_e32.o",wd);
		__check_err(__test_file(fullpath),
			"compiler_x86_64: error: e32.srec prog embedding failed.");

		__command(
			"cd %s;"
			"echo 'struct _program_info_struct {"
         	"unsigned int core_local_data;"
        		"unsigned int stack_size;"
				"} _program_info = (struct _program_info_struct){ 0x%x, 0x%x };'"
				" > _program_info.c;"
			CC_COMPILER " -c _program_info.c",
			wd,addr_core_local_data,256);
		__log(p2,"]%s\n",buf1);
      p2_prev = p2;
      __execshell(buf1,p2);
      if (p2 != p2_prev) __append_str(log,p2_prev,0,0);
		snprintf(fullpath,256,"%s/_program_info.o",wd);
      __check_err(__test_file(fullpath),
         "compiler_x86_64: error: generate _program_info.o failed");
	
		__command(
			"cd %s; "
			CXX_COMPILER CCFLAGS_LINK 
			" -shared -Wl,-soname,data_srec_e32.so -o data_srec_e32.so"
			" data_srec_e32.o %s.elfcl _program_info.o 2>&1",
			wd,filebase);
		__log(p2,"]%s\n",buf1); 
		p2_prev = p2;
		__execshell(buf1,p2);
		if (p2 != p2_prev) __append_str(log,p2_prev,0,0);
		snprintf(fullpath,256,"%s/data_srec_e32.so",wd,filebase);
		__check_err(__test_file(fullpath),
			"compiler_x86_64: error: data_e32.so link failed.");

	} else {

		/* error no source */

		WARN(__FILE__,__LINE__,"compile: no source");
		__remove_work_dir(wd);
		return(CL_INVALID_PROGRAM);

	}

/* XXX here we are going to pass back the new format by filling bin,bin_sz */

	char ofname[256];
	snprintf(ofname,256,"%s/data_srec_e32.so",wd,filebase);

	int ofd = open(ofname,O_RDONLY,0);
	if (ofd < 0) return(CL_BUILD_PROGRAM_FAILURE);


	struct stat ofst; 
	stat(ofname,&ofst);

	if (S_ISREG(ofst.st_mode) && ofst.st_size > 0) {
		size_t ofsz = ofst.st_size;
		*p_bin_sz = ofsz;
		*p_bin = (char*)malloc(ofsz);
		void* p = mmap(0,ofsz,PROT_READ,MAP_PRIVATE,ofd,0);
		memcpy(*p_bin,p,ofsz);
		munmap(p,ofsz);
	} else {
		close(ofd);
		__remove_work_dir(wd);
		return(CL_BUILD_PROGRAM_FAILURE);
	}

	close(ofd);

	printcl( CL_DEBUG "COPRTHR_TMP = '%s'",coprthr_tmp);

	if (!coprthr_tmp) {
	printcl( CL_DEBUG "removing work directory");
	__remove_work_dir(wd);
	}

	if (wdtemp) free(wdtemp);

	return(0);

}



