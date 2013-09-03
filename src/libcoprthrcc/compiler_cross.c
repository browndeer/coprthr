/* compiler_native.c 
 *
 * Copyright (c) 2009-2013 Brown Deer Technology, LLC.  All Rights Reserved.
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
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

#include "printcl.h"
#include "elfcl.h"
#include "compiler.h"
#include "computil.h"


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>

#include "xcl_structs.h"

#define DEFAULT_BUF1_SZ 16384
#define DEFAULT_BUF2_SZ 16384

#define NDK "/home/richie/android/android-ndk-r8e/"
#define CROSS_PATH NDK \
	"toolchains/arm-linux-androideabi-4.7/prebuilt/linux-x86_64/bin"
#define CROSS_SYSROOT NDK "platforms/android-14/arch-arm"
#define CROSS_SHELL "export PATH=" CROSS_PATH ":$PATH;"

#if defined(__COPRTHR_TARGET_HOST_x86_64__)

#define __compile compile_x86_64
#define __elfcl_write elfcl_write_x86_64

#define CCFLAGS_OCL \
	" -fno-exceptions -O3 -msse -msse3 " \
	" -funsafe-math-optimizations -fno-math-errno -funsafe-math-optimizations " \
	" -fschedule-insns -fschedule-insns2" \
	" -U_FORTIFY_SOURCE"

#define ECC_BLOCKED_FLAGS \
	"-D_FORTIFY_SOURCE", "-fexceptions", \
	"-fstack-protector-all" "-fstack-protector-all"

#elif defined(__COPRTHR_TARGET_HOST_i386__)

#define __compile compile_i386
#define __elfcl_write elfcl_write_i386

#define CCFLAGS_OCL \
	" -fno-exceptions -O3 -msse -msse3 " \
	" -funsafe-math-optimizations -fno-math-errno -funsafe-math-optimizations " \
	" -fschedule-insns -fschedule-insns2" \
	" -U_FORTIFY_SOURCE"

#define ECC_BLOCKED_FLAGS \
	"-D_FORTIFY_SOURCE", "-fexceptions", \
	"-fstack-protector-all" "-fstack-protector-all"

#elif defined(__COPRTHR_TARGET_HOST_arm32__)

#define __compile compile_arm32
#define __elfcl_write elfcl_write_arm32

#define CCFLAGS_OCL \
   " -fno-exceptions -O3 -marm " \
   " -mfloat-abi=softfp -mfpu=neon " \
   " -funsafe-math-optimizations -fno-math-errno -funsafe-math-optimizations " \
   " -fschedule-insns -fschedule-insns2" \
   " -U_FORTIFY_SOURCE"

#define ECC_BLOCKED_FLAGS "-D_FORTIFY_SOURCE", "-fexceptions", \
       "-fstack-protector-all" "-fstack-protector-all"

#else

#error target host not specified or not supported

#endif


/* select compiler preferernces */

#ifdef LIBCOPRTHR_CC
#define CC_COMPILER LIBCOPRTHR_CC
#else
//#define CC_COMPILER " gcc "
#define CC_COMPILER " arm-linux-androideabi-gcc "
#endif

#ifdef LIBCOPRTHR_CXX
#define CXX_COMPILER LIBCOPRTHR_CXX
#else
//#define CXX_COMPILER " g++ "
#define CXX_COMPILER " arm-linux-androideabi-g++ "
#endif


/*** compiler flags ***/

#define CCFLAGS_KTHR CCFLAGS_OCL \
	" --sysroot=" CROSS_SYSROOT \
	" -I" INSTALL_INCLUDE_DIR \
	" -D __xcl_kthr__ --include=sl_engine.h " \
	" -D __STDCL_KERNEL_VERSION__=020000" \
	" -fPIC " 

#define CCFLAGS_KCALL \
	" --sysroot=" CROSS_SYSROOT \
	" -O0 -fPIC " \
	" -D__xcl_kcall__ -I" INSTALL_INCLUDE_DIR \
	" --include=sl_engine.h "

#define CXXFLAGS_LINK_LIB \
	" --sysroot=" CROSS_SYSROOT \


/*** shell command code ***/

#define SHELLCMD_KTHR_COMPILE \
	"cd %s; " CROSS_SHELL CC_COMPILER CCFLAGS_KTHR " %s -c %s.cpp"

#define SHELLCMD_KCALL_COMPILE \
	"cd %s; " CROSS_SHELL CC_COMPILER CCFLAGS_KCALL " %s -c _kcall_%s.c"

#define SHELLCMD_KCALL_GEN_WRAPPER \
	"cd %s;" \
	" cpp -x c++ -I" INSTALL_INCLUDE_DIR " %s %s " \
	" | awk -v prog=\\\"%s\\\" " \
	" 'BEGIN { pr=0; }" \
	" { " \
	"   if($0~/^#/ && $3==prog) pr=1;" \
	"   else if ($0~/^#/) pr=0;" \
	"   if ($0!~/^#/ && pr==1) print $0;" \
	" }' | xclnm --kcall -d -c -o _kcall_%s.c - "

#define SHELLCMD_CXXLINK_LIB \
	"cd %s; CROSS_SHELL " CXX_COMPILER CXXFLAGS_LINK_LIB \
	" -shared -Wl,-soname,%s.so -o %s.so" \
	" %s.o _kcall_%s.o %s.elfcl " 


static char* ecc_block_flags[] = { ECC_BLOCKED_FLAGS };


#undef __compile
#define __compile compile_android_arm32

int __compile(
	cl_device_id devid,
	unsigned char* src, size_t src_sz, 
	unsigned char** p_bin, size_t* p_bin_sz, 
	char* opt_in, char** log 
)
{
	int i;
	int err;
	struct stat fst;

	char default_opt[] = "";
	char* opt = (opt_in)? strdup(opt_in) : strdup(default_opt);

   printcl( CL_DEBUG "opt |%s|",opt);

   for(i=0;i<sizeof(ecc_block_flags)/sizeof(char*);i++) {

      char* p;
      while ((p=strstr(opt,ecc_block_flags[i]))) {
         memset(p,' ',strlen(ecc_block_flags[i]));
         printcl( CL_WARNING "blocked compiler option '%s'",ecc_block_flags[i]);
         printcl( CL_DEBUG "new opt |%s|",opt);
      }
   }

       printcl( CL_DEBUG "opt after filter |%s|",opt);


	char* coprthr_tmp = getenv("COPRTHR_TMP");

   if (stat(coprthr_tmp,&fst) || !S_ISDIR(fst.st_mode)
               || (fst.st_mode & S_IRWXU) != S_IRWXU) coprthr_tmp = 0;

   char* wdtemp;

   if (coprthr_tmp) {
      wdtemp = (char*)malloc(strlen(coprthr_tmp) +11);
      sprintf(wdtemp,"%s/xclXXXXXX",coprthr_tmp);
   } else {
      wdtemp = strdup("/tmp/xclXXXXXX");
   }


	char filebase[] 	= "XXXXXX";
	char* wd = mkdtemp(wdtemp);
	mktemp(filebase);

	char file_cl[256];
	char file_cpp[256];
	char file_ll[256];

	snprintf(file_cl,256,"%s/%s.cl",wd,filebase);
	snprintf(file_cpp,256,"%s/%s.cpp",wd,filebase);
	snprintf(file_ll,256,"%s/%s.ll",wd,filebase);

	printcl( CL_DEBUG "compile: work dir %s",wd);
	printcl( CL_DEBUG "compile: filebase %s",filebase);

	if (!buf1) buf1 = malloc(DEFAULT_BUF1_SZ);

	*log = (char*)malloc(DEFAULT_BUF2_SZ);
   (*log)[0] = '\0';

	unsigned int nsym;
	unsigned int narg;
	struct clsymtab_entry* clsymtab = 0;
	struct clargtab_entry* clargtab = 0;

	size_t clstrtab_sz;
	size_t clstrtab_alloc_sz;
	char* clstrtab = 0;


	/* with cltrace LD_PRELOAD env var is problem so just prevent intercepts */
	unsetenv("LD_PRELOAD");


	if (src) {

		printcl( CL_DEBUG "compile: build from source");

		/*** 
		 *** write cl/cpp files 
		 ***/

		err = write_file_cl(file_cl,src_sz,src);
		__check_err( err, "internal error: write file_cl failed");

		err = write_file_cpp(file_cpp,src_sz,src);
		__check_err( err, "internal error: write file_cpp failed");


		/*** 
		 *** compile kernel 
		 ***/

		char* cmd = 0;

		asprintf(&cmd,SHELLCMD_KTHR_COMPILE,wd,opt,filebase);
		printcl( CL_DEBUG "before compile");
		err = exec_shell(cmd,log);
		printcl( CL_DEBUG "after compile");
		__check_err( err, "error: kernel compilation failed");

		/*** 
		 *** generate kcall wrapper 
		 ***/

			asprintf(&cmd,SHELLCMD_KCALL_GEN_WRAPPER,
				wd,file_cl,opt,file_cl,filebase);
			err = exec_shell(cmd,log);
			__check_err( err, "error: gen kcall wrappers failed");


		/* gcc compile kcall wrapper */

		asprintf(&cmd,SHELLCMD_KCALL_COMPILE,wd,"\0",filebase);
		err = exec_shell(cmd,log);
		__check_err( err, "kcall wrapper compilation failed" );



		/* now extract arg data */

		printcl( CL_DEBUG "extract arg data");

		nsym = get_nsym(wd,file_cl,opt);

		err = build_clsymtab(wd,file_cl,opt,nsym, &narg, &clsymtab,
				&clstrtab_alloc_sz, &clstrtab_sz,&clstrtab);
		__check_err(err,
			"internal error: build_clsymtab failed: cannot parse xclnm output");

		printcl( CL_DEBUG "narg %d",narg);

		err = build_clargtab(wd,file_cl,opt,nsym,narg,
				&clargtab,&clstrtab_alloc_sz,&clstrtab_sz,&clstrtab);
		__check_err(err,
			"internal err: build_clargtab failed: cannot parse xclnm output");

		/* now build elf/cl object */

		snprintf(buf1,256,"%s/%s.elfcl",wd,filebase);
		int fd = open(buf1,O_WRONLY|O_CREAT,S_IRWXU);
		err = __elfcl_write(fd,
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

		asprintf(&cmd,SHELLCMD_CXXLINK_LIB,
			wd,filebase,filebase,filebase,filebase,filebase,filebase);
		err = exec_shell(cmd,log);
		__check_err( err, "error: kernel link failed");

	} else {

		__check_err(1,"compile: no source");

	}

	char ofname[256];

	strcpy(ofname,wd);
	strcat(ofname,"/");
	strcat(ofname,filebase);
	strcat(ofname,".so");
	
	err = copy_file_to_mem(ofname,p_bin,p_bin_sz);
	__check_err( err, "internal error");


	if (!coprthr_tmp) remove_work_dir(wd);

	return(0);

}

