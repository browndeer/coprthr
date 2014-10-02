/* compiler_cc.c 
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

#include "version.h"
#include "printcl.h"
#include "elfcl.h"
#include "compiler.h"
#include "computil.h"
#include "coprthr_cc.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <elf.h>

//#include "xcl_structs.h"

//#define DEFAULT_BUF1_SZ 16384
//#define DEFAULT_BUF2_SZ 16384


//#if defined(__COPRTHR_TARGET_HOST_x86_64__)

//#define __compile compile_x86_64
//#define __elfcl_write elfcl_write_x86_64

//struct str_pair {
//	char* left[];
//	char right[];
//};


typedef int (*compile_t) ( void* _reserved, const char* src, size_t src_sz,
   char** p_bin, size_t* p_bin_sz, const char* opt_in, char** log);


struct targets_entry {
	char* e_target;
	char* e_compiler_name;
	char* e_compiler_lib;
};

struct targets_entry targets[] = {
	{ COPRTHR_CC_TARGET_X86_64, "compile_x86_64", "libcoprthrcc.so" },
	{ COPRTHR_CC_TARGET_I386, "compile_i386", "libcoprthrcc.so" },
	{ COPRTHR_CC_TARGET_ARM32, "compile_arm32", "libcoprthrcc.so" },
	{ COPRTHR_CC_TARGET_E32_EMEK, "compile_e32_emek", "libcoprthrcc-e.so" },
//	{ COPRTHR_CC_TARGET_E32, "compile_e32", "libcoprthrcc-e.so" }
	{ COPRTHR_CC_TARGET_E32, "compile_e32_needham", "libcoprthrcc-e.so" }
#if defined(ENABLE_ANDROID_CROSS_COMPILE)
	,{ COPRTHR_CC_TARGET_ARM32_ANDROID, "compile_android_arm32", 
		"libcoprthrcc.so" }
#endif
#if defined(ENABLE_MIC_CROSS_COMPILE)
	,{ COPRTHR_CC_TARGET_MIC, "compile_mic", "libcoprthrcc.so" }
#endif
};

size_t nsupp_targets = (sizeof(targets)/sizeof(struct targets_entry));

coprthr_program_t coprthr_cc( 
	const char* src, size_t len, const char* opt, char** plog
)
{
	int i;

	if (!opt)
		return 0;

	char* compiler_name = 0;
	char* compiler_lib = 0;

	int icompiler = -1;

	printcl( CL_DEBUG "opt |%s|",opt);

	int report_version = strstr( opt, "--version")? 1 : 0;
	int report_targets = strstr( opt, "--targets")? 1 : report_version;

	if ( report_version || report_targets) {

		if (report_version) {
			__append_asprintf(plog,COPRTHR_VERSION_STRING "\n");
			__append_asprintf(plog,
				"Copyright (c) 2009-2013 Brown Deer Technology, LLC."
				"  All Rights Reserved.\n");
			__append_asprintf(plog,GPL3_NOTICE);
		}

		if (report_targets) {
			__append_asprintf(plog,"supported targets:");
			for(i=0;i<nsupp_targets;i++) {
				void* dh = dlopen(targets[i].e_compiler_lib,RTLD_LAZY);
   			compiler_t compile = dlsym(dh,targets[i].e_compiler_name);
				if (dh && compile) 
					__append_asprintf(plog," %s",targets[i].e_target);
			}
			__append_asprintf(plog,"\n");
		}

		return 0;
	}

	for(i=0;i<nsupp_targets;i++) {

		printcl( CL_DEBUG "compare |%s|",targets[i].e_target );

		if ( strstr( opt, targets[i].e_target) ) {
			if (icompiler < 0) {
				icompiler = i;
			} else {
				printcl( CL_ERR "multiple targets selected");
				exit(-1);
			}
		}

	}

	if (icompiler < 0) {
		printcl( CL_ERR "no supported compiler specified");
		exit(-1);
	}

	compiler_name = targets[icompiler].e_compiler_name;
	compiler_lib = targets[icompiler].e_compiler_lib;

	printcl( CL_DEBUG "selected target |%s|%s|",compiler_name,compiler_lib);

	void* dh = dlopen(compiler_lib,RTLD_LAZY);

	compiler_t compile = dlsym(dh,compiler_name);

	printcl( CL_DEBUG "dh=%p compile=%p",dh,compile);

//	unsigned char* bin;
//	size_t bin_sz;
//	char* log;
//	int err = compile(0,src,len,&bin,&bin_sz,"",&log);

//	printf("compile returned %d\n",err);

	struct coprthr_program* prg1
		= (struct coprthr_program*)malloc(sizeof(struct coprthr_program));
	bzero(prg1,sizeof(struct coprthr_program));

	prg1->src = src;
	prg1->src_sz = len;

//	prg1->build_opt = opt;
	prg1->build_opt = 0;


	/* XXX the opt string must be filtered */

	char* tmp0 = strdup(opt);
	char* saveptr;
	char* ptr = strtok_r(tmp0," ",&saveptr);
	char* opt1 = 0;
	while(ptr) {
//		printf("next|%s|\n",ptr);
//		printf("before %p\n",opt1);
		if (strncmp("-mtarget",ptr,8))
			__append_asprintf(&opt1," %s",ptr);
//		printf("after %p\n",opt1);
		ptr = strtok_r(0," ",&saveptr);
//		printf("opt1=%s\n",opt1);
	}
	prg1->build_opt = (opt1)? strdup(opt1) : 0;
	if (opt1) free(opt1);
	if (tmp0) free(tmp0);


	int err = compile( 0, prg1->src,prg1->src_sz,
		&prg1->bin, &prg1->bin_sz, prg1->build_opt, &prg1->build_log);

	if (err) {
		printcl(CL_ERR "compile returned %d\n",err);
		free(prg1);
		prg1 = 0;
	} else if (prg1->build_log) {
		 if (plog)
			__append_asprintf(plog,"%s\n",prg1->build_log);
	}

	dlclose(dh);

	return prg1;

}


coprthr_program_t coprthr_cc_read_bin( const char* path, int flags )
{
	int fd;
	struct stat fs;

	if ( stat(path,&fs) )
		return 0;

	if ( ! S_ISREG(fs.st_mode) )
		return 0;

	if ( (fd = open(path,O_RDONLY)) == -1) 
		return 0;;

	size_t len = fs.st_size;

	struct coprthr_program* prg1
		= (struct coprthr_program*)malloc(sizeof(struct coprthr_program));
	bzero(prg1,sizeof(struct coprthr_program));

	printcl( CL_DEBUG "prg1=%p",prg1);

	prg1->src = 0;
	prg1->src_sz = 0;
	prg1->build_opt = 0;

	prg1->bin = malloc(len);
	prg1->bin_sz = read(fd,prg1->bin,len);

	if (prg1->bin_sz != len) {
		free(prg1->bin);
		free(prg1);
		return 0;
	}

	return prg1;	
//	return prg1->bin_sz;	
}


size_t coprthr_cc_write_bin( const char* path, coprthr_program_t prg, 
	int flags )
{
	int fd = open(path,O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);
	size_t sz = write(fd,prg->bin,prg->bin_sz);
	close(fd);
	return(sz);
}


