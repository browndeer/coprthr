/* libocl.c
 *
 * Copyright (c) 2009-2011 Brown Deer Technology, LLC.  All Rights Reserved.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#define __USE_GNU
#include <dlfcn.h>

#include "util.h"
#include "libocl.h"
#include "oclcall.h"


#define min(a,b) ((a<b)?a:b)

static struct platform_struct _libocl_platforms[8] = { 0,0,0,0,0,0,0,0 };
static unsigned int _nplatforms = 0;


struct oclent_struct*
load_oclent( void* dlh )
{
	int n;

	printf("load_oclent:\n");

	size_t oclent_table_sz = oclncalls*oclent_sz;

	struct oclent_struct* oclent
		= (struct oclent_struct*)malloc(oclent_table_sz);

	printf("here\n"); fflush(stdout);
	memcpy(oclent,empty_oclent,oclent_table_sz);

	printf("here\n"); fflush(stdout);

	printf("oclncalls=%d\n",oclncalls);

	for(n=0;n<oclncalls;n++) {
		void* sym = dlsym(dlh,oclcallnames[n]);
		if (sym) oclent[n].ocl_call = sym;
		printf("oclent[%d] load attempt '%s' %p\n",n,oclcallnames[n],sym);
	}

	return(oclent);
}


clGetPlatformIDs(
	cl_uint nplatforms,
   cl_platform_id* platforms,
   cl_uint* nplatforms_ret
)
{
	int n;
	char fullpath[256];
   DIR* dirp = opendir("/etc/OpenCL/vendors/");
   struct dirent* dp;
   while ( (dp=readdir(dirp)) ) {
		strncpy(fullpath,"/etc/OpenCL/vendors/",256);
		strncat(fullpath,dp->d_name,256);
      printf("is this an icd file |%s|\n",fullpath);
      char* p;
      if ( (p=strrchr(dp->d_name,'.')) && !strncasecmp(p,".icd",5) ) {
         printf("this is an icd file |%s|\n",fullpath);
			struct stat st;
			stat(fullpath,&st);
			printf("size of icd file is %d\n",st.st_size);
			if (S_ISREG(st.st_mode) && st.st_size>0) {
				int fd = open(fullpath,O_RDONLY);
				read(fd,fullpath,st.st_size);
				close(fd);
				fullpath[strcspn(fullpath," \t\n")] = '\0';
         	printf("lib is |%s|\n",fullpath);
				void* h = dlopen(fullpath,RTLD_LAZY);
				printf("%p\n",h);
				if (h) {
					printf("dlopen successful\n");
					_libocl_platforms[_nplatforms].dlh = h;
					struct oclent_struct* oclent 
						= _libocl_platforms[_nplatforms].oclent = load_oclent(h);
					if (oclent!=0 && oclent[OCLCALL_clGetPlatformIDs].ocl_call) {
						int dummy = 0;
						printf("entry number %d\n",OCLCALL_clGetPlatformIDs);
						typedef cl_int (*pf_t)(cl_uint,cl_platform_id*,cl_uint*);
						((pf_t)oclent[OCLCALL_clGetPlatformIDs].ocl_call)(0,0,&dummy);
						printf("dummy %d\n",dummy);
					}	
					++_nplatforms;
				}
			}
      }
   }
   closedir(dirp);

	DEBUG2("libocl: found %d platforms",_nplatforms);

// foreach platform ...

//	dlopen lib
//	test to ensure valid platform implementation (icd and non-icd checks)
// 	allocate calltab and init to loaders
//	put dl handle in first calltab slot
//	if NOW then for each call use dlsym to get ptr ad store in calltab

	if (nplatforms_ret) *nplatforms_ret = _nplatforms;

	if (platforms) {
		for(n=0;n<min(nplatforms,_nplatforms);n++) 
			platforms[n] = (cl_platform_id)(_libocl_platforms+n);
	}

	return(CL_SUCCESS);
	
}


