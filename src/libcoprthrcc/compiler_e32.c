/* compiler_e32.c 
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

#define CCFLAGS_LINK 

#define ECC_COMPILER " e-gcc "
#define ECXX_COMPILER " e-g++ "
#define EOBJCOPY " e-objcopy "

#define ECCFLAGS_OCL " -O3 -g -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -funroll-loops -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=truncate -falign-loops=8 -falign-functions=8 -w -fstack-usage -fno-exceptions -U_FORTIFY_SOURCE -fno-stack-protector -I/opt/adapteva/esdk/tools/e-gnu/epiphany-elf/sys-include -fpermissive -D__coprthr_device__ -x c++"

#define ECCFLAGS_KCALL " -O0 -g -Wall -c -fmessage-length=0 -ffast-math -ftree-vectorize -funroll-loops -Wunused-variable -ffp-contract=fast -mlong-calls -mfp-mode=round-nearest -w -fstack-usage -fno-exceptions -U_FORTIFY_SOURCE -fno-stack-protector -I/opt/adapteva/esdk/tools/e-gnu/epiphany-elf/sys-include -fpermissive -D__coprthr_device__ "


#define ECC_BLOCKED_FLAGS "-D_FORTIFY_SOURCE", "-fexceptions", \
	"-fstack-protector-all" "-fstack-protector-all" \
	"-D__link_mpi__ "


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

//#include "elf_cl.h"
#include "elfcl.h"
#include "printcl.h"
#include "compiler.h"
#include "computil_e32.h"

//#include "dmalloc.h"

//void* devmembase = 0x8e000000;
//void* devmembase = 0x8e002000;
void* devmembase = 0x8e100000;

#ifndef INSTALL_INCLUDE_DIR
#define INSTALL_INCLUDE_DIR "/usr/local/browndeer/include"
#endif
#ifndef INSTALL_LIB_DIR
#define INSTALL_LIB_DIR "/usr/local/browndeer/lib"
#endif


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>

#include "xcl_structs.h"

#define DEFAULT_BUF1_SZ 163840
#define DEFAULT_BUF2_SZ 163840
#define DEFAULT_BUILD_LOG_SZ 25600


static char* buf2 = 0;
static char* logbuf = 0;


#define CLERROR_BUILD_FAILED -1

static int __test_file( char* file ) 
{
	struct stat s;
	int err = stat(file,&s);
	printcl( CL_DEBUG "__test_file '%s' %d",file,err);
	return (stat(file,&s));
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


#if defined(__COPRTHR_TARGET_E32_NEEDHAM__)

#define __compile_e32 compile_e32_needham
#define E32PTH_IF_HDR "e32pth_if_needham.h"
#define E32PTH_CORE_MAIN_OBJ "_e32pth_core_main_needham.o"
#define E32PTH_LDF "_e32pth_core_needham.ldf"

#elif defined(__COPRTHR_TARGET_E32_NEEDHAMPRO__)

#define __compile_e32 compile_e32_needhampro
#define E32PTH_IF_HDR "e32pth_if_needhampro.h"
#define E32PTH_CORE_MAIN_OBJ "_e32pth_core_main_needhampro.o"
#define E32PTH_LDF "_e32pth_core_needhampro.ldf"

#elif defined(__COPRTHR_TARGET_E32_BLANK__)

#define __compile_e32 compile_e32_blank
#define E32PTH_IF_HDR "e32pth_if_blank.h"
#define E32PTH_CORE_MAIN_OBJ "_e32pth_core_main_blank.o"
#define E32PTH_LDF "_e32pth_core_blank.ldf"

#else

#error no target supported or specified

#endif


#ifdef USE_E32_OPENCL_EXT
#define SHELLCMD_KTHR_COMPILE_S \
	"cd %s; " \
	ECXX_COMPILER ECCFLAGS_OCL  \
	" -I" INSTALL_INCLUDE_DIR  \
	" -D __xcl_kthr__ --include=" E32PTH_IF_HDR \
	" --include=opencl_lift.h " \
	" --include=e32_opencl_ext.h " \
	" -D __STDCL_KERNEL_VERSION__=020000 -D__COPRTHR__" \
	" -D E32_DRAM_ZEROPAGE=%p " \
	" %s  -S %s.cpp "
#define SHELLCMD_KTHR_COMPILE \
	"cd %s; " \
	ECXX_COMPILER ECCFLAGS_OCL  \
	" -I" INSTALL_INCLUDE_DIR  \
	" -D __xcl_kthr__ --include=" E32PTH_IF_HDR \
	" --include=opencl_lift.h " \
	" --include=e32_opencl_ext.h " \
	" -D __STDCL_KERNEL_VERSION__=020000 -D__COPRTHR__" \
	" -D E32_DRAM_ZEROPAGE=%p " \
	" %s  -c %s.cpp -o e32_%s.o "
#else
#define SHELLCMD_KTHR_COMPILE_S \
	"cd %s; " \
	ECXX_COMPILER ECCFLAGS_OCL  \
	" -I" INSTALL_INCLUDE_DIR  \
	" -D __xcl_kthr__ --include=" E32PTH_IF_HDR \
	" --include=opencl_lift.h " \
	" -D __STDCL_KERNEL_VERSION__=020000 -D__COPRTHR__" \
	" -D E32_DRAM_ZEROPAGE=%p " \
	" %s  -S %s.cpp "
#define SHELLCMD_KTHR_COMPILE \
	"cd %s; " \
	ECXX_COMPILER ECCFLAGS_OCL  \
	" -I" INSTALL_INCLUDE_DIR  \
	" -D __xcl_kthr__ --include=" E32PTH_IF_HDR \
	" --include=opencl_lift.h " \
	" -D __STDCL_KERNEL_VERSION__=020000 -D__COPRTHR__" \
	" -D E32_DRAM_ZEROPAGE=%p " \
	" %s  -c %s.cpp -o e32_%s.o "
#endif

#define CPPFLAGS \
   " -D__coprthr_device__ " \
   " -I" INSTALL_INCLUDE_DIR

#define SHELLCMD_KCALL_GEN_WRAPPER \
	"cd %s;" \
	" cpp -x c++ " CPPFLAGS " %s %s " \
	" | awk -v prog=\\\"%s\\\" " \
	"'BEGIN { pr=0; }" \
	" { " \
	"   if($0~/^#/ && $3==prog) pr=1;" \
	"   else if ($0~/^#/) pr=0;" \
	"   if ($0!~/^#/ && pr==1) print $0;" \
	" }' | xclnm --%s -d -c -o _%s_%s.c - "

#define SHELLCMD_KCALL_COMPILE \
	"cd %s; " \
	CC_COMPILER " -O0 -fPIC" \
	" -D__xcl_kcall__ -DE32_DRAM_ZEROPAGE=%p -I%s " \
	" --include=sl_engine.h -c _kcall_%s.c "

#define SHELLCMD_KCALL2_COMPILE \
	"cd %s; " \
	CC_COMPILER " -O0 -fPIC" \
	" -D__xcl_kcall__ -DE32_DRAM_ZEROPAGE=%p -I%s " \
	" --include=sl_engine.h -c _kcall2_%s.c "

#define SHELLCMD_KCALL3_COMPILE \
	"cd %s; " \
  	ECC_COMPILER ECCFLAGS_KCALL \
	" -DE32_DRAM_ZEROPAGE=%p -I" INSTALL_INCLUDE_DIR\
  	" -D__xcl_kcall__ --include=" E32PTH_IF_HDR \
	" -c _kcall3_%s.c -o e32_kcall3_%s.o "

#define SHELLCMD_LINK_CORE_PROG \
	"cd %s;" \
	" e-ld -r -o e32.o" \
	" " E32PTH_CORE_MAIN_OBJ " e32_%s.o e32_kcall3_%s.o" \
	" -L/opt/adapteva/esdk/tools/e-gnu/epiphany-elf/lib -le-lib " 

#define SHELLCMD_LINK_CORE_PROG_MPI \
	"cd %s;" \
	" e-ld -r -o e32.o" \
	" " E32PTH_CORE_MAIN_OBJ " e32_%s.o e32_kcall3_%s.o" \
	" -L" INSTALL_LIB_DIR " -lcoprthr_mpi " \
	" -L/opt/adapteva/esdk/tools/e-gnu/epiphany-elf/lib -le-lib " 


#define SHELLCMD_GEN_SREC \
	"cd %s; "  \
  	ECC_COMPILER " -o e32.0.elf " \
	" e32.o " \
	" -T " E32PTH_LDF " -lm; " \
	EOBJCOPY " --srec-forceS3 --output-target srec" \
	" e32.0.elf e32.0.srec "

#define SHELLCMD_OBJCOPY_STEP \
	"cd %s; " \
	" objcopy -I binary -O elf32-littlearm -B arm e32.srec data_srec_e32.o "

#define SHELLCMD_GEN_PROGINFO_OBJ \
	"cd %s;" \
	"echo 'struct _program_info_struct {" \
  		"unsigned int core_local_data;" \
  		"unsigned int stack_size;" \
	"} _program_info = (struct _program_info_struct){ 0x%x, 0x%x };'" \
	" > _program_info.c;" \
	CC_COMPILER " -c _program_info.c"

#define SHELLCMD_CXXLINK_LIB \
	"cd %s; " \
	CXX_COMPILER CCFLAGS_LINK \
	" -shared -Wl,-soname,data_srec_e32.so -o data_srec_e32.so" \
	" data_srec_e32.o %s.elfcl _program_info.o "

	
static char* ecc_block_flags[] = { ECC_BLOCKED_FLAGS };


int __compile_e32(
	cl_device_id devid,
	unsigned char* src, size_t src_sz, 
	unsigned char** p_bin, size_t* p_bin_sz, 
	char* opt_in, char** log 
)
{

	printcl( CL_DEBUG "__compile_e32");

   char* env_tmpdir = getenv("TMPDIR");
   char* coprthr_tmp = getenv("COPRTHR_TMP");
   char* tmpdir= (coprthr_tmp)? strdup(coprthr_tmp)
      : (env_tmpdir)? strdup(env_tmpdir) : strdup("/tmp");

//	if (!buf1) buf1 = (char*)malloc(16384); //// XXX TEMPORARY XXXX
	char* buf1 = (char*)malloc(16384); //// XXX TEMPORARY XXXX
	printcl( CL_DEBUG "buf1=%p",buf1);

	int i;
	int err;
	struct stat fst;

	char default_opt[] = "";
	char* opt = (opt_in)? strdup(opt_in) : strdup(default_opt);

	printcl( CL_DEBUG "opt |%s|",opt);

	int link_mpi = strstr( opt, "-D__link_mpi__")? 1 : 0;

	for(i=0;i<sizeof(ecc_block_flags)/sizeof(char*);i++) {
		char* p;
		while ((p=strstr(opt,ecc_block_flags[i]))) {
			memset(p,' ',strlen(ecc_block_flags[i]));
			printcl( CL_WARNING "blocked compiler option '%s'",
				ecc_block_flags[i]);
			printcl( CL_DEBUG "new opt |%s|",opt);
		}
	}

	printcl( CL_DEBUG "opt after filter |%s|",opt);

//	char* coprthr_tmp = getenv("COPRTHR_TMP");

	if (coprthr_tmp && (stat(coprthr_tmp,&fst) || !S_ISDIR(fst.st_mode) 
		|| (fst.st_mode & S_IRWXU) != S_IRWXU) ) coprthr_tmp = 0;

	char* wdtemp = 0;

//	if (coprthr_tmp) {
//		wdtemp = (char*)malloc(strlen(coprthr_tmp) +11);
//		sprintf(wdtemp,"%s/xclXXXXXX",coprthr_tmp);
//	} else {
//		wdtemp = (char*)malloc(strlen("/tmp") +11);
//		sprintf(wdtemp,"%s/xclXXXXXX","/tmp");
//	}
	asprintf(&wdtemp,"%s/xclXXXXXX",tmpdir);


	printcl( CL_DEBUG "wdtemp %p|%s|", wdtemp,wdtemp);

	char filebase[] 	= "XXXXXX";
	char* wd = mkdtemp(wdtemp);
	mktemp(filebase);

	printcl( CL_DEBUG "wd %p|%s|", wd,wd);

	char file_cl[256];
	char file_cpp[256];
	char file_ll[256];

	snprintf(file_cl,256,"%s/%s.cl",wd,filebase);
	snprintf(file_cpp,256,"%s/%s.cpp",wd,filebase);
	snprintf(file_ll,256,"%s/%s.ll",wd,filebase);

	printcl( CL_DEBUG "compile: work dir %s",wd);
	printcl( CL_DEBUG "compile: filebase %s",filebase);

	if (!buf1) buf1 = malloc(DEFAULT_BUF1_SZ);
	if (!buf2) buf2 = malloc(DEFAULT_BUF2_SZ);
	if (!logbuf) logbuf = malloc(DEFAULT_BUF2_SZ);

	bzero(buf2,DEFAULT_BUF2_SZ);
	bzero(logbuf,DEFAULT_BUF2_SZ);

	*log = (char*)malloc(DEFAULT_BUF2_SZ);
//   (*log)[0] = '\0';
	strcpy(*log,"compiler_e32:build_log:\n");

	printcl( CL_DEBUG "PRE log size %ld",strnlen(*log,32768) );
	printcl( CL_DEBUG "----- log ------");
	printcl( CL_DEBUG "%s",*log);
	printcl( CL_DEBUG "----- log ------");


	unsigned int nsym;
	unsigned int narg;
	struct clsymtab_entry* clsymtab = 0;
	struct clargtab_entry* clargtab = 0;

	size_t clstrtab_sz;
	size_t clstrtab_alloc_sz;
	char* clstrtab = 0;

	printcl( CL_DEBUG "compile: %p",src);

	/* with cltrace LD_PRELOAD env var is problem so just prevent intercepts */
	unsetenv("LD_PRELOAD");

	printcl( CL_DEBUG "wd |%s|", wd);

	if (src) {

		printcl( CL_DEBUG "compile: build from source");

		char* cmd = 0;

		/* copy files to work dir */

		char* objs[] = { E32PTH_CORE_MAIN_OBJ, E32PTH_LDF };

		for(i=0;i<sizeof(objs)/sizeof(char*);i++) {
			__asprintf(&cmd,"\\cp "INSTALL_LIB_DIR"/%s %s ",objs[i],wd);
			err = exec_shell(cmd,log);
			__check_err( err, "obj copy failed" );
		}


		/* write cl file */

		err = write_file_cl(file_cl,src_sz,(char*)src);
		__check_err( err, "internal error: write file_cl failed");

		err = write_file_cpp(file_cpp,src_sz,(char*)src);
		__check_err( err, "internal error: write file_cpp failed");


		/* assemble to native object */

		__asprintf(&cmd,SHELLCMD_KTHR_COMPILE_S,wd,devmembase,opt,filebase);
		err = exec_shell(cmd,log);
		__check_err( err, "error: kernel compilation failed");

		__asprintf(&cmd,SHELLCMD_KTHR_COMPILE,wd,devmembase,opt,filebase,filebase);
		err = exec_shell(cmd,log);
		__check_err( err, "error: kernel compilation failed");


		printcl( CL_DEBUG "devmembase = %x", devmembase);


		/* generate kcall wrappers */

		char* wrappers[] = { "kcall", "kcall2", "kcall3" };

		for(i=0;i<sizeof(wrappers)/sizeof(char*);i++) {

        __asprintf(&cmd,SHELLCMD_KCALL_GEN_WRAPPER,
            wd,file_cl,opt,file_cl,wrappers[i],wrappers[i],filebase);
         err = exec_shell(cmd,log);
         __check_err( err, "error: gen kcall wrappers failed");

		}	

		/* gcc compile kcall wrappers */

		__asprintf(&cmd,SHELLCMD_KCALL_COMPILE,
			wd,devmembase,INSTALL_INCLUDE_DIR,filebase);
		err = exec_shell(cmd,log);
		__check_err( err, "kcall wrapper compilation failed" );


		__asprintf(&cmd,SHELLCMD_KCALL2_COMPILE,
			wd,devmembase,INSTALL_INCLUDE_DIR,filebase);
		err = exec_shell(cmd,log);
		__check_err( err, "kcall wrapper compilation failed" );

		
		printcl( CL_DEBUG "compile kcall wrapper");

		__asprintf(&cmd,SHELLCMD_KCALL3_COMPILE, wd,devmembase,filebase,filebase);
		err = exec_shell(cmd,log);
		__check_err( err, "kcall wrapper compilation failed" );


		printcl( CL_DEBUG "link core prog for all cores");

		if (link_mpi)
			__asprintf(&cmd,SHELLCMD_LINK_CORE_PROG_MPI, 
				wd,filebase,filebase);
		else
			__asprintf(&cmd,SHELLCMD_LINK_CORE_PROG, 
				wd,filebase,filebase);

		err = exec_shell(cmd,log);
		__check_err( err, "error: e32 prog link failed" );


		__asprintf(&cmd,SHELLCMD_GEN_SREC, wd);
		err = exec_shell(cmd,log);
		__check_err( err, "create srec failed" );


		__asprintf(&cmd,"cd %s; cat e32.0.srec > e32.srec",wd);
		err = exec_shell(cmd,log);
		__check_err( err, "create srec failed" );
			

		/* now extract sym arg data */

		printcl( CL_DEBUG "extract sym and arg data");

		nsym = get_nsym(wd,file_cl,opt);

		err = build_clsymtab(wd,file_cl,opt,nsym, &narg, &clsymtab,
			&clstrtab_alloc_sz, &clstrtab_sz,&clstrtab);
		__check_err(err,
         "internal error: build_clsymtab failed: cannot parse xclnm output");

      printcl( CL_DEBUG "narg %d",narg);


		unsigned int addr_core_local_data = 0;

		i=0;

		err = scan_for_sym_e32(wd,"e32.0.elf",nsym,clsymtab,clstrtab,
			&addr_core_local_data,log);
		__check_err( err, "scan for sym failed");


		err = scan_for_su_e32(wd,filebase,nsym,clsymtab,clstrtab, log);
		printcl( CL_DEBUG "A");
		__check_err( err, "scan for su failed");
		printcl( CL_DEBUG "B");


		/*** build clargtab ***/

		printcl( CL_DEBUG "C");
		err = build_clargtab(wd,file_cl,opt,nsym,narg,
			&clargtab,&clstrtab_alloc_sz,&clstrtab_sz,&clstrtab);
		printcl( CL_DEBUG "D");
		__check_err(err,
         "internal err: build_clargtab failed: cannot parse xclnm output");

		printcl( CL_DEBUG "i=%d narg=%d\n",i,narg);

		/* now build elf/cl object */

	printcl( CL_DEBUG "wd |%s|", wd);

		printcl( CL_DEBUG "/* now build elf/cl object */");

		snprintf(buf1,256,"%s/%s.elfcl",wd,filebase);
		int fd = open(buf1,O_WRONLY|O_CREAT,S_IRWXU);
		err = elfcl_write_arm32(fd,
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

	printcl( CL_DEBUG "wd |%s|", wd);

		printcl( CL_DEBUG "SHELLCMD_OBJCOPY_STEP");

      __asprintf(&cmd,SHELLCMD_OBJCOPY_STEP, wd);
      err = exec_shell(cmd,log);
      __check_err( err, "e32.srec prog embedding failed" );


		printcl( CL_DEBUG "SHELLCMD_GEN_PROGINFO_OBJ");

      __asprintf(&cmd,SHELLCMD_GEN_PROGINFO_OBJ, wd,addr_core_local_data,256);
      err = exec_shell(cmd,log);
      __check_err( err, "generate _program_info.o failed" );


		printcl( CL_DEBUG "SHELLCMD_CXXLINK_LIB");

      __asprintf(&cmd,SHELLCMD_CXXLINK_LIB, wd,filebase);
      err = exec_shell(cmd,log);
      __check_err( err, "error: kernel link failed");


	} else {

		__check_err(1,"compile: no source");

	}

	printcl( CL_DEBUG "wd |%s|", wd);

	char ofname[256];
	
	strcpy(ofname,wd);
   strcat(ofname,"/");
   strcat(ofname,"/data_srec_e32.so");

	printcl( CL_DEBUG "copy_file_to_mem");

	printcl( CL_DEBUG "wd %p|%s|", wd,wd);

   err = copy_file_to_mem(ofname,p_bin,p_bin_sz);
	printcl( CL_DEBUG "wd %p|%s|", wd,wd);
	printcl( CL_DEBUG "back from copy_file_to_mem");
   __check_err( err, "internal error");
	printcl( CL_DEBUG "after check err");

	printcl( CL_DEBUG "wd |%s|", wd);
printcl( CL_DEBUG "conditional remove work dir: before |%s|",wd);
	if (!coprthr_tmp) remove_work_dir(wd);
printcl( CL_DEBUG "conditional remove work dir: after");

	if (wdtemp) free(wdtemp);

	printcl( CL_DEBUG "log size %ld",strnlen(*log,32768) );
	printcl( CL_DEBUG "----- log ------");
	printcl( CL_DEBUG "%s",*log);
	printcl( CL_DEBUG "----- log ------");

	return(0);

}



