/* libocl.c
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include <unistd.h>
#include <wordexp.h>
#include <libconfig.h>

#define __USE_GNU
#include <dlfcn.h>

//#include "util.h"
#include "libocl.h"
#include "oclcall.h"
#include "printcl.h"
#include "clrpc.h"

#ifndef DEFAULT_OPENCL_ICD_PATH
#define DEFAULT_OPENCL_ICD_PATH "/etc/OpenCL/vendors"
#endif

#define min(a,b) ((a<b)?a:b)

//static struct platform_struct _libocl_platforms[8] = { 0,0,0,0,0,0,0,0 };
static struct platform_struct* _libocl_platforms = 0;
static unsigned int _nplatforms = 0;

struct oclconf_platform_struct {
   const char* platform_name;
   const char* platform_lib;
   const char* platform_call_prefix;
   const char* platform_call_postfix;
};

struct oclconf_info_struct {
   size_t nplatforms;
   struct oclconf_platform_struct* platforms;
   size_t nicd_dirs;
	char** icd_dirs;
	int clrpc_enable;
	int clrpc_nservers;
	char** clrpc_servers;
};


int read_oclconf_info( struct oclconf_info_struct* info );
void free_oclconf_info( struct oclconf_info_struct* info );


struct oclent_struct*
load_oclent( void* dlh )
{
	int n;

	DEBUG2("load_oclent:");

	size_t oclent_table_sz = oclncalls*oclent_sz;

	struct oclent_struct* oclent
		= (struct oclent_struct*)malloc(oclent_table_sz);

	memcpy(oclent,empty_oclent,oclent_table_sz);

	DEBUG2("oclncalls=%d",oclncalls);

	for(n=0;n<oclncalls;n++) {
		void* sym = dlsym(dlh,oclcallnames[n]);
		if (sym) oclent[n].ocl_call = sym;
		DEBUG2("oclent[%d] load attempt '%s' %p",n,oclcallnames[n],sym);
	}

	return(oclent);
}


clGetPlatformIDs(
	cl_uint nplatforms,
   cl_platform_id* platforms,
   cl_uint* nplatforms_ret
)
{

	printcl( CL_DEBUG "clGetPlatformIDs (loader) @ %p",&clGetPlatformIDs);

	int i,j,n;

	char fullpath[256];
	char strbuf[256];

	if (!_libocl_platforms) {

		printcl( CL_DEBUG "searching for platforms");

		struct oclconf_info_struct oclconf_info;

		if ( read_oclconf_info( &oclconf_info ) ) {

			printcl( CL_WARNING 
				"cannot read ocl.conf, using ICD fallback"
				" (/etc/OpenCL/vendors");

			oclconf_info.nplatforms = 0;
			oclconf_info.platforms = 0;
			oclconf_info.nicd_dirs = 1;
			oclconf_info.icd_dirs = (char**)malloc(sizeof(char*));
			oclconf_info.icd_dirs[0] = strdup("/etc/OpenCL/vendors");
			oclconf_info.clrpc_enable = 0;
			oclconf_info.clrpc_nservers = 0;
			oclconf_info.clrpc_servers = 0;

		}

//		_nplatforms = 0;

		printcl( CL_DEBUG "conf returns nplatforms = %d",oclconf_info.nplatforms);

		printcl( CL_DEBUG "%d icd dirs specified",oclconf_info.nicd_dirs);

		for(i=0;i<oclconf_info.nicd_dirs;i++) {

			printcl( CL_DEBUG "scanning ICD dir '%s'",oclconf_info.icd_dirs[i]);

   		DIR* dirp = opendir( oclconf_info.icd_dirs[i] );
   		struct dirent* dp;

   		if (dirp) while ( (dp=readdir(dirp)) ) {

				strncpy(fullpath, oclconf_info.icd_dirs[i], 256);
				strncat(fullpath,"/",256);
				strncat(fullpath,dp->d_name,256);

      		printcl( CL_DEBUG "is this an icd file? |%s|",fullpath);

      		char* p;

      		if ( (p=strrchr(dp->d_name,'.')) && !strncasecmp(p,".icd",5) ) {

         		printcl( CL_DEBUG "found icd file |%s|",fullpath);

					struct stat st;
					stat(fullpath,&st);
					printcl( CL_DEBUG "size of icd file is %d",st.st_size);

					if (S_ISREG(st.st_mode) && st.st_size>0) {

						int fd = open(fullpath,O_RDONLY);
						read(fd,fullpath,st.st_size);

						if (fd < 0) {

							printcl( CL_WARNING "open failed on '%s'",fullpath);

						} else {

            			fullpath[strcspn(fullpath," \t\n")] = '\0';

							int duplicate = 0;
							for(j=0;j<oclconf_info.nplatforms;j++) {
	
								if (
									!strncmp(oclconf_info.platforms[j].platform_lib,
										fullpath,256)
								) duplicate = 1;

							}
							if (duplicate) {
								printcl( CL_WARNING "duplicate platform lib, skipping");
								continue;
							}

            			printcl( CL_DEBUG "adding platform '%s' (%s)",
								dp->d_name,fullpath);

							int n = ++oclconf_info.nplatforms;

							oclconf_info.platforms = (struct oclconf_platform_struct*)
								realloc(oclconf_info.platforms,
									n*sizeof(struct oclconf_platform_struct));

							oclconf_info.platforms[n-1].platform_name 
								= strdup(dp->d_name);
							oclconf_info.platforms[n-1].platform_lib 
								= strdup(fullpath);
							oclconf_info.platforms[n-1].platform_call_prefix = 0;
							oclconf_info.platforms[n-1].platform_call_postfix = 0;
							
							close(fd);

						}

					}

				}
			}
		}

		printcl( CL_DEBUG "%d local platforms found",oclconf_info.nplatforms);


		/* process conf clrpc info */

		int rpc_enable = oclconf_info.clrpc_enable;

		clrpc_server_info* rpc_servers = (clrpc_server_info*)
			calloc(oclconf_info.clrpc_nservers,sizeof(clrpc_server_info));

		if (rpc_enable) {

			for(i=0;i<oclconf_info.clrpc_nservers;i++) {

				if (oclconf_info.clrpc_servers[i]) {

					char* tmpstr = strdup(oclconf_info.clrpc_servers[i]);

					int port = 2112;
					char* psep = 0;
					if (psep = strrchr(tmpstr,':')) {
						port = atoi(psep+1);
						*psep = '\0';
					}
			
					rpc_servers[i].address = tmpstr;
					rpc_servers[i].port = (ev_uint16_t)port;			
				
					printcl( CL_DEBUG "added '%s' %d",rpc_servers[i].address,
						rpc_servers[i].port);	
				}
				
			}

//			clrpc_connect(oclconf_info.clrpc_nservers,servers);

		} 

//		cl_uint rpc_nplatforms = 0;

		int (*clrpc_connect) ( unsigned int, clrpc_server_info* );
		cl_int (*clrpc_clGetPlatformIDs) (cl_uint,cl_platform_id*,cl_uint*);
		cl_int (*clrpc_clGetPlatformInfo) (cl_platform_id,cl_platform_info,
			size_t,void*,size_t*);
	
		void* h_libclrpc = dlopen("libclrpc.so",RTLD_LAZY);
		clrpc_connect = dlsym(h_libclrpc,"clrpc_connect");
		clrpc_clGetPlatformIDs = dlsym(h_libclrpc,"clrpc_clGetPlatformIDs");
		clrpc_clGetPlatformInfo = dlsym(h_libclrpc,"clrpc_clGetPlatformInfo");

		printcl( CL_DEBUG "libclrpc: %p %p %p %p",h_libclrpc,clrpc_connect,
			clrpc_clGetPlatformIDs,clrpc_clGetPlatformInfo);


		printcl( CL_DEBUG "nservers=%d",oclconf_info.clrpc_nservers);

		if (clrpc_connect && oclconf_info.clrpc_nservers > 0) 
			clrpc_connect(oclconf_info.clrpc_nservers,rpc_servers);
		else 
			rpc_enable = 0;


		if (!clrpc_clGetPlatformIDs || !clrpc_clGetPlatformInfo) rpc_enable = 0;

		cl_uint rpc_nplatforms = 0;

		if (rpc_enable) {
			clrpc_clGetPlatformIDs( 0,0,&rpc_nplatforms );
			printcl( CL_DEBUG "%d platforms available via RPC servers",
				rpc_nplatforms);
		}


		int np = oclconf_info.nplatforms + rpc_nplatforms;

		_libocl_platforms = (struct platform_struct*)
			calloc(np,sizeof(struct platform_struct));

		for(i=0;i<oclconf_info.nplatforms;i++) {

			if (!oclconf_info.platforms[i].platform_name) {
				printcl( CL_WARNING "ocl.conf: platform name missing");
				continue;
			}
			
			if (!oclconf_info.platforms[i].platform_lib) {
				printcl( CL_WARNING "ocl.conf: platform lib missing");
				continue;
			}
			
			if (oclconf_info.platforms[i].platform_lib)
				printcl( CL_DEBUG "ocl.conf: platform '%s' use lib '%s'",
					oclconf_info.platforms[i].platform_name,
					oclconf_info.platforms[i].platform_lib);

			if (oclconf_info.platforms[i].platform_call_prefix)
				printcl( CL_DEBUG "\tuse call_prefix '%s'",
					oclconf_info.platforms[i].platform_call_prefix);

			if (oclconf_info.platforms[i].platform_call_postfix)
				printcl( CL_DEBUG "\tuse call_postfix '%s'",
					oclconf_info.platforms[i].platform_call_postfix);

  	    	void* h = dlopen(oclconf_info.platforms[i].platform_lib,RTLD_LAZY);
  	    	printcl( CL_DEBUG "dlopen dlh=%p",h);

  	    	if (h) {

  	  			printcl( CL_DEBUG "dlopen successful");

				_libocl_platforms[_nplatforms].dlh = h;

				/* XXX Now we begin the unnecessarily complicated ICD steps 
				 * XXX for loading a library.
				 * XXX Note to OpenCL standard committee - man dlopen. -DAR */

				void* pf_clIcdGetPlatformIDsKHR 
					= dlsym(h,"clIcdGetPlatformIDsKHR");

				void* pf_clGetPlatformInfo = dlsym(h,"clGetPlatformInfo");

				void* pf_clGetExtensionFunctionAddress 
					= dlsym(h,"clGetExtensionFunctionAddress");

				printcl( CL_DEBUG "pf %p %p %p",pf_clIcdGetPlatformIDsKHR,
					pf_clGetPlatformInfo,pf_clGetExtensionFunctionAddress);

				if (!pf_clIcdGetPlatformIDsKHR 
					&& pf_clGetExtensionFunctionAddress
				) {

           		typedef void* (*pf_t)(char*);
					pf_clIcdGetPlatformIDsKHR 
						= ((pf_t)pf_clGetExtensionFunctionAddress)(
						"clIcdGetPlatformIDsKHR");

				}

				printcl( CL_DEBUG "pf %p %p %p",pf_clIcdGetPlatformIDsKHR,
					pf_clGetPlatformInfo,pf_clGetExtensionFunctionAddress);

				if (!pf_clIcdGetPlatformIDsKHR || !pf_clGetPlatformInfo 
					|| !pf_clGetExtensionFunctionAddress
				) {
					printcl( CL_WARNING "missing ICD required call, skipping");
					continue;
				}

				cl_platform_id platform = 0;

				{
					typedef cl_int (*pf_t)(cl_uint,cl_platform_id*,cl_uint*);
					cl_uint n;
					int err = ((pf_t)pf_clIcdGetPlatformIDsKHR)(0,0,&n);
					printcl( CL_DEBUG 
						"clIcdGetPlatformIDsKHR shows %d platforms available",n);

					if (n>1)
						printcl( CL_WARNING 
							"library provides multiple platforms, taking first one");

					if (n>0) {

						err = ((pf_t)pf_clIcdGetPlatformIDsKHR)(1,&platform,0);

						printcl( CL_DEBUG 
							"clIcdGetPlatformIDsKHR returns platform %p",platform);

					}
				}

				if (!platform) continue;

				_libocl_platforms[_nplatforms].imp_platform_id = platform;

/* XXX keep this code to support non-ICD platforms in future -DAR 
               struct oclent_struct* oclent
                  = _libocl_platforms[_nplatforms].oclent = load_oclent(h);
               DEBUG2("platform [%d] oclent %p",_nplatforms,oclent);
               if (oclent!=0 && oclent[OCLCALL_clGetPlatformIDs].ocl_call) {
                  int n = 0;
                  DEBUG2("entry number %d",OCLCALL_clGetPlatformIDs);
						if (oclent[OCLCALL_clGetPlatformIDs].ocl_call) {

                  	typedef cl_int (*pf_t)(cl_uint,cl_platform_id*,cl_uint*);
                  	((pf_t)oclent[OCLCALL_clGetPlatformIDs].ocl_call)(0,0,&n);
                   	DEBUG2("imp nplatforms %d",n);
                   	if (n != 1) {
                 	  DEBUG2("vendor does not report nplatforms=1, skipping\n");
                    	  continue;
							}
                 	 	((pf_t)oclent[OCLCALL_clGetPlatformIDs].ocl_call)(
                 	   	1,&_libocl_platforms[_nplatforms].imp_platform_id,0);
                 	 	*(void***)_libocl_platforms[_nplatforms].imp_platform_id
                 	   	= (void**)oclent;
						}

               }
*/

  	      	++_nplatforms;

				printcl( CL_DEBUG "inc _nplatforms to %d",_nplatforms);

			}

		}

		if (rpc_nplatforms) {

				cl_platform_id* platforms = (cl_platform_id*)
					calloc(rpc_nplatforms,sizeof(cl_platform_id));

				printcl( CL_DEBUG "%d %p",rpc_nplatforms,platforms);

				clrpc_clGetPlatformIDs( rpc_nplatforms, platforms, 0);

				for(i=0;i<rpc_nplatforms;i++,_nplatforms++) {

					clrpc_clGetPlatformInfo(platforms[i],CL_PLATFORM_NAME,
						256,strbuf,0);

					printcl( CL_DEBUG "adding RPC platform name '%s'",strbuf);

					_libocl_platforms[_nplatforms].imp_platform_id = platforms[i];
				
				}

				free(platforms);
		}

		free_oclconf_info( &oclconf_info );

	}

	printcl( CL_DEBUG "libocl: found %d platforms",_nplatforms);

	if (nplatforms_ret) *nplatforms_ret = _nplatforms;

	if (platforms) {
		for(n=0;n<min(nplatforms,_nplatforms);n++) {
			platforms[n] = _libocl_platforms[n].imp_platform_id;
			printcl( CL_DEBUG "platform %d is %p",n,platforms[n]);
		}
	}

//	free_oclconf_info( &oclconf_info );

	printcl( CL_DEBUG " returning ");

	return(CL_SUCCESS);
	
}


cl_int clGetDeviceIDs( 
	cl_platform_id a0,cl_device_type a1,cl_uint a2,cl_device_id* a3,cl_uint* a4
)
{
	void* oclent = *(void**)a0;
	printcl( CL_DEBUG "clGetDeviceIDs (loader) %p",oclent);
	typedef cl_int (*pf_t) (cl_platform_id,cl_device_type,cl_uint,
		cl_device_id*,cl_uint*);
	cl_int rv = ((pf_t)(*(((void**)oclent)+OCLCALL_clGetDeviceIDs)))(
		a0,a1,a2,a3,a4);
	return rv;
}


cl_context clCreateContext(
	const cl_context_properties* a0,
	cl_uint a1,const cl_device_id* a2,
	cl_pfn_notify_t a3,void* a4,cl_int* a5
)
{
	cl_context_properties* p = (cl_context_properties*)a0;
	int n=0;
	for(;*p != 0 && n<256; p+=2,n++)
		if (*p == CL_CONTEXT_PLATFORM) { ++p; break; }
	if (*p==0 || n==256) return (cl_context)0;
	void* oclent = *(void**)(*p);
	printcl( CL_DEBUG "clCreateContext (loader) %p",oclent);
	typedef cl_context (*pf_t) (const cl_context_properties*,
		cl_uint,const cl_device_id*,cl_pfn_notify_t,void*,cl_int*);
	cl_context rv 
		= ((pf_t)(*(((void**)oclent)+OCLCALL_clCreateContext)))(
		a0,a1,a2,a3,a4,a5);
	return rv;
}


cl_context clCreateContextFromType(
	const cl_context_properties* a0,
	cl_device_type a1,cl_pfn_notify_t a2,void* a3,cl_int* a4
)
{
	cl_context_properties* p = (cl_context_properties*)a0;
	int n=0;
	for(;*p != 0 && n<256; p+=2,n++)
		if (*p == CL_CONTEXT_PLATFORM) { ++p; break; }
	if (*p==0 || n==256) return (cl_context)0;
	void* oclent = *(void**)(*p);
	printcl( CL_DEBUG "clCreateContextFromType (loader) %p",oclent);
	typedef cl_context (*pf_t) (const cl_context_properties*,
		cl_device_type,cl_pfn_notify_t,void*,cl_int*);
	cl_context rv 
		= ((pf_t)(*(((void**)oclent)+OCLCALL_clCreateContextFromType)))(
		a0,a1,a2,a3,a4);
	return rv;
}


cl_int clGetContextInfo(
	cl_context a0,cl_context_info a1,size_t a2,void* a3,size_t* a4)
{
	int j;
   void* oclent = *(void**)a0;
	printcl( CL_DEBUG "clGetContextInfo (loader) %p",oclent);
   typedef cl_int (*pf_t) (cl_context,cl_context_info,size_t,void*,size_t*);
   cl_int rv
      = ((pf_t)(*(((void**)oclent)+OCLCALL_clGetContextInfo)))(a0,a1,a2,a3,a4);
   return rv;
}


cl_int clWaitForEvents(
	cl_uint a0,const cl_event* a1
)
{
	if (a0<1) return CL_INVALID_VALUE;
	if (!a1) return CL_INVALID_EVENT;
	void* oclent = *(void**)(*a1);
	printcl( CL_DEBUG "clWaitForEvents (loader) %p",oclent);
	typedef cl_int (*pf_t) (cl_uint,const cl_event*);
	cl_int rv 
		= ((pf_t)(*(((void**)oclent)+OCLCALL_clWaitForEvents)))(a0,a1);
	return rv;
}


cl_int clCreateKernelsInProgram(
	cl_program a0,cl_uint a1,cl_kernel* a2,cl_uint* a3
)
{
	void* oclent = *(void**)a0;
	printcl( CL_DEBUG "clCreateKernelsInProgram (loader) %p",oclent);
	typedef cl_int (*pf_t) (cl_program,cl_uint,cl_kernel*,cl_uint*);
	cl_int rv = ((pf_t)(*(((void**)oclent)+OCLCALL_clCreateKernelsInProgram)))(
		a0,a1,a2,a3);
	return rv;
}


cl_int clUnloadCompiler(void)
{ return (cl_int)CL_SUCCESS; }


void free_oclconf_info( struct oclconf_info_struct* info )
{
   if (info->platforms) free(info->platforms);
}


int read_oclconf_info( struct oclconf_info_struct* info )
{
	int i;

  config_t cfg;
  config_setting_t* setting;
  config_setting_t* cfg_platforms;
  config_setting_t* cfg_platform;
  config_setting_t* cfg_icd_dirs;
  config_setting_t* cfg_clrpc;
  config_setting_t* cfg_servers;
  config_setting_t* cfg_server;
//	int ival;
//	const char* sval;
//	const char* str;

	info->nplatforms = 0;
	info->platforms = 0;
	info->nicd_dirs = 0;
	info->icd_dirs = 0;
	info->clrpc_enable = 0;
	info->clrpc_servers = 0;


  config_init(&cfg);

	char* search_paths[] = { "./ocl.conf", "./.ocl.conf", "$HOME/ocl.conf", 
		"$HOME/.ocl.conf", "/etc/ocl.conf", "$COPRTHR_PATH/ocl.conf",
		"/usr/local/browndeer/ocl.conf" };

	char* path = 0;
	for(i=0;i<sizeof(search_paths)/sizeof(char*);i++) {
		wordexp_t w;
		struct stat s;
		wordexp(search_paths[i],&w,WRDE_SHOWERR);
		path = strdup(w.we_wordv[0]);
		wordfree(&w);
		if (stat(path,&s)==0) break;
		free(path);
		path = 0;
	}	

	if (!path) return(-1);

	printcl( CL_DEBUG "selected path is '%s'",path);

	if(! config_read_file(&cfg, path)) {
		printcl( CL_ERR "%s:%d - %s", config_error_file(&cfg),
			config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		exit(-1);
	}

	if (cfg_platforms = config_lookup(&cfg,"platforms")) {

		int n = config_setting_length(cfg_platforms);

		printcl( CL_DEBUG "there are %d platforms",n);

		info->nplatforms = n;

		info->platforms = (struct oclconf_platform_struct*)
			calloc(n,sizeof(struct oclconf_platform_struct));

		for(i=0;i<n;i++) {

			cfg_platform = config_setting_get_elem(cfg_platforms,i);

			const char* platform_name = 0;
			const char* platform_lib = 0;
			const char* platform_call_prefix = 0;
			const char* platform_call_postfix = 0;

			config_setting_lookup_string(cfg_platform,"platform", &platform_name);

			config_setting_lookup_string(cfg_platform,"lib", &platform_lib);

			config_setting_lookup_string(cfg_platform,"call_prefix",
				&platform_call_prefix);

			config_setting_lookup_string(cfg_platform,"call_postfix",
				&platform_call_postfix);

			info->platforms[i].platform_name = platform_name;
			info->platforms[i].platform_lib = platform_lib;
			info->platforms[i].platform_call_prefix = platform_call_prefix;
			info->platforms[i].platform_call_postfix = platform_call_postfix;

		}

	}  else {

		info->nplatforms = 0;
		info->platforms = 0;

	}


	if (cfg_icd_dirs = config_lookup(&cfg,"icd_dirs")) {

		int n = config_setting_length(cfg_icd_dirs);

		printcl( CL_DEBUG "there are %d icd_dirs",n);

		info->nicd_dirs = n;

		info->icd_dirs = (char**) calloc(n,sizeof(char*));

		for(i=0;i<n;i++) {

			const char* icd_dir = config_setting_get_string_elem(cfg_icd_dirs,i);

			info->icd_dirs[i] = strdup(icd_dir);
		}

	} else {

		info->nicd_dirs = 0;
		info->icd_dirs = 0;

	}


	if (cfg_clrpc = config_lookup(&cfg,"clrpc")) {

		printcl( CL_DEBUG "there is a clrpc section");

		const char* enable = 0;

		if (config_setting_lookup_string(cfg_clrpc,"enable",&enable)) {

			info->clrpc_enable = (strcasecmp(enable,"yes"))? 0 : 1;

		}	
		printcl( CL_DEBUG "enble string %s",enable);

		if (cfg_servers = config_setting_get_member(cfg_clrpc,"servers")) {

			int n = config_setting_length(cfg_servers);

			info->clrpc_nservers = n;

			printcl( CL_DEBUG "there are %d RPC servers",n);

			info->clrpc_servers = (char**) calloc(n,sizeof(char*));

			for(i=0;i<n;i++) {

				if (cfg_server = config_setting_get_elem(cfg_servers,i)) {

					const char* url = 0;

					if (config_setting_lookup_string(cfg_server,"url",&url)) {

						info->clrpc_servers[i] = strdup(url);

						printcl( CL_DEBUG "add server '%s'",url);
					}

				}

			}

		}
	
	} else {

		info->clrpc_enable = 0;
		info->clrpc_nservers = 0;
		info->clrpc_servers = 0;

	}


	if (path) free(path);

	return(0);
	
}
