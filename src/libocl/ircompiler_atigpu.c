/* ircompiler_atigpu.c 
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

#include "cal.h"
#include "calcl.h"

#include "elf_cl.h"
#include "compiler.h"
//#include "compiler_atigpu.h"

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

#define __XCL_TEST

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


/* XXX note that buf2 is not protected from overfull, fix this -DAR */
#define __log(p,fmt,...) do { \
	p += snprintf(p,__CLMAXSTR_LEN,fmt,##__VA_ARGS__); \
	} while(0);

/* XXX note that buf2 is not protected from overfull, fix this -DAR */
#define __execshell(command,p) do { \
	char c; \
	DEBUG(__FILE__,__LINE__,"__execshell: %s",command); \
	FILE* fp = popen(command,"r"); \
	while((c=fgetc(fp)) != EOF) *p++ = c; \
	err = pclose(fp); \
	} while(0);



/*
 *	ATI GPU compile
 */

void* ilcompile_atigpu(
	cl_device_id devid,
	unsigned char* src, size_t src_sz, 
	unsigned char* bin, size_t bin_sz, 
	char** opt, char** log 
)
{
	int i,isym,iarg;
	int err;
	char c;
	int fd;
	struct stat fst;
	FILE* fp;
	char line[1024];

	DEBUG(__FILE__,__LINE__,"ilcompile_atigpu");

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

	size_t filesz_ll = 0;
	size_t filesz_il = 0;

	unsigned char* pfile_ll = 0;
	unsigned char* pfile_il = 0;

	DEBUG(__FILE__,__LINE__,"ilcompile: work dir %s",wd);
	DEBUG(__FILE__,__LINE__,"ilcompile: filebase %s",filebase);

	if (!buf1) buf1 = malloc(DEFAULT_BUF1_SZ);
	if (!buf2) buf2 = malloc(DEFAULT_BUF2_SZ);
	size_t buf2_alloc_sz = DEFAULT_BUF2_SZ;

	bzero(buf2,DEFAULT_BUF2_SZ);

	unsigned int nsym;
	unsigned int narg;
	struct clsymtab_entry* clsymtab = 0;
	struct clargtab_entry* clargtab = 0;

	size_t clstrtab_sz;
	size_t clstrtab_alloc_sz;
	char* clstrtab = 0;

		char* p2 = buf2;

		DEBUG(__FILE__,__LINE__,"ilcompile: %p %p",src,bin);

		if (bin) { /* use binary */

			DEBUG(__FILE__,__LINE__,"ilcompile: build from binary");


			/* copy rt objects to work dir */

			__command("cp "INSTALL_LIB_DIR"/__elfcl_rt.o %s",wd);
			__log(p2,"]%s\n",buf1); \
			__execshell(buf1,p2);


			filesz_il = bin_sz;
			pfile_il = bin;


			/* compile IL with CAL */

#ifdef XCL_DEBUG
				CALuint calcl_version[3];
				calclGetVersion(
					&calcl_version[0],&calcl_version[1],&calcl_version[2]);
				DEBUG(__FILE__,__LINE__,"CAL compiler version %d.%d.%d",
					calcl_version[0],calcl_version[1],calcl_version[2]);
				switch (__resolve_devid(devid,atigpu.calinfo.target)) {
					case CAL_TARGET_600:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_600"); break;
					case CAL_TARGET_610:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_610"); break;
					case CAL_TARGET_630:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_630"); break;
					case CAL_TARGET_670:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_670"); break;
					case CAL_TARGET_7XX:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_7XX"); break;
					case CAL_TARGET_770:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_770"); break;
					case CAL_TARGET_710:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_710"); break;
					case CAL_TARGET_730:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_730"); break;
					case CAL_TARGET_CYPRESS:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_CYPRESS"); break;
					case CAL_TARGET_JUNIPER:
						DEBUG(__FILE__,__LINE__,"CAL_TARGET_JUNIPER"); break;
					default:
						WARN(__FILE__,__LINE__,"CAL TARGET UNKNOWN"); break;
				}
#endif

				DEBUG(__FILE__,__LINE__,"compiling with calclCompile ...");
				CALobject obj = 0;
				err = calclCompile(&obj,CAL_LANGUAGE_IL,pfile_il,
					__resolve_devid(devid,atigpu.calinfo.target));
				DEBUG(__FILE__,__LINE__,"calclCompile returned %d",err);


				DEBUG(__FILE__,__LINE__,"linking with calclLink ...");
				CALimage img = 0;
				err = calclLink(&img,&obj,1);
				DEBUG(__FILE__,__LINE__,"calclLink returned %d",err);

				char* pp = (char*)img;
				DEBUG(__FILE__,__LINE__,"is image an ELF? %s",pp);

				CALuint imgsz;
				calclImageGetSize(&imgsz,img);


				DEBUG(__FILE__,__LINE__,
					"log\n"
					"------------------------------------------------------------\n"
					"%s\n"
					"------------------------------------------------------------",
					buf2);

				p2=buf2;

				
				/* now extract arg data */

//// XXX for Il, question is how to treat "names" and "args" 
//// XXX hardcode now, modify xclnm to treat .il files later and replace later


				DEBUG(__FILE__,__LINE__,"extract arg data");

				char* p;
				char* p1;
				char* ps;

				narg = 1; /* start a t 1 to include the NULL marker */
				int ii = -1;					
				p = pfile_il;	
				while (p < ((char*)pfile_il + filesz_il)) {	
					p1 = index(p,'\n');
					strncpy(buf1,p,(size_t)(p1-p));
					buf1[(size_t)(p1-p)]='\0';
					if (!strncmp(buf1,";arg:",5)) {
						DEBUG(__FILE__,__LINE__,"ilcompile: %s",buf1);
						char* ps;
						char* pt0 = strtok_r(buf1,":",&ps);
						char* pt1 = strtok_r(0,":",&ps);
						if (pt1) {
							++narg;
							ii = max(ii,atoi(pt1));
						}
					}
					p = p1 + 1;
				}

				DEBUG(__FILE__,__LINE__,"ilcompile: narg=%d", narg);

				if (narg-2 != ii) {
					ERROR(__FILE__,__LINE__,
						"ilcompile: undefined kernel arguments");
					return(0);
				}


				clstrtab_sz = 0;
//				clstrtab_alloc_sz = nsym*1024;
				clstrtab_alloc_sz = (1+2+2*narg)*1024; /* best guess right now */
				clstrtab = malloc(clstrtab_alloc_sz);
				clstrtab[clstrtab_sz++] = '\0';

				strncpy(&clstrtab[clstrtab_sz],"main",256);
				unsigned int str_main = clstrtab_sz;
				clstrtab_sz += strnlen("main",256) + 1;

				strncpy(&clstrtab[clstrtab_sz],"cb0",256);
				unsigned int str_cb0 = clstrtab_sz;
				clstrtab_sz += strnlen("cb0",256) + 1;

				strncpy(&clstrtab[clstrtab_sz],"cb1",256);
				unsigned int str_cb1 = clstrtab_sz;
				clstrtab_sz += strnlen("cb1",256) + 1;



				/* XXX build clargtab first since clsymtab is derived w/IL -DAR */

				clargtab = (struct clargtab_entry*)
					calloc(narg,sizeof(struct clargtab_entry));


				/* need initial NULL marker for first entry to clargtab */
				clargtab[0] = init_clargtab_entry(0,0,0,0,0,0,0,0);


/* XXX this is a hack, copied from xclnm_gram.h, fix this! -DAR */
#define __TYPE_OPAQUE  262
#define __TYPE_VOID  263
#define __TYPE_INT8  264
#define __TYPE_INT16  265
#define __TYPE_INT32  266 
#define __TYPE_INT64  267
#define __TYPE_FLOAT  268
#define __TYPE_DOUBLE  269

/* XXX another hack, these are defined to avoi dconflict w/xclnm - DAR */
#define __ADDRSPACE_ATI_IM2D_RO	9
#define __ADDRSPACE_ATI_IM2D_WO	10


				int datatype;
				int vecn;
				int arrn;
				int addrspace;
				int ptrc;


				i=1; /* clargtab starts with a NULL marker */
				char aname[256];
				int argn = 1;
				p = pfile_il;	
				while (p < ((char*)pfile_il + filesz_il)) {	
					p1 = index(p,'\n');
					strncpy(buf1,p,(size_t)(p1-p));
					buf1[(size_t)(p1-p)]='\0';
					if (!strncmp(buf1,";arg:",5)) {
						DEBUG(__FILE__,__LINE__,"ilcompile: %s",buf1);
						char* ps;
						char* pt0 = strtok_r(buf1,":",&ps);
						char* pt1 = strtok_r(0,":",&ps);
						char* pt2 = strtok_r(0,":",&ps);

						if (pt1 && pt2) {
	
							ii = atoi(pt1);

							if (ii+1 != i) {
								DEBUG(__FILE__,__LINE__,"ii=%d i=%d",ii,i);
								ERROR(__FILE__,__LINE__,"cannot parse IL");
								exit(-1);
							}

							if (!strncmp(pt2,"f128",32)) {

								DEBUG(__FILE__,__LINE__,"argument %d is a float4",ii);

								datatype = __TYPE_FLOAT;
								vecn = 4;
								arrn = 1;
								addrspace = 0;
								ptrc = 0;
								argn = ii+2;

							} else if (!strncmp(pt2,"im2d_ro",32)) {

								DEBUG(__FILE__,__LINE__,
									"argument %d is a ro image2d_t",ii);

								datatype = __TYPE_OPAQUE;
								vecn = 1;
								arrn = 1;
								addrspace = __ADDRSPACE_ATI_IM2D_RO;
								ptrc = 0;
								argn = ii+2;

							} else if (!strncmp(pt2,"im2d_wo",32)) {

								DEBUG(__FILE__,__LINE__,
									"argument %d is a wo image2d_t",ii);

								datatype = __TYPE_OPAQUE;
								vecn = 1;
								arrn = 1;
								addrspace = __ADDRSPACE_ATI_IM2D_WO;
								ptrc = 0;
								argn = ii+2;

							} else {

								ERROR(__FILE__,__LINE__,"bad arg type in IL");
								return(0);

							}

							clargtab[i] = init_clargtab_entry(
								0,datatype,vecn,arrn,addrspace,ptrc,clstrtab_sz,argn);

//							strncpy(&clstrtab[clstrtab_sz],aname,256);
							strncpy(&clstrtab[clstrtab_sz],"<undefined>",256);
							clstrtab_sz += strnlen("<undefined>",256) + 1;

						} else { /* XXX put an error here */ 

						}

						++i;

					}
					p = p1 + 1;
				}


				if (i!=narg) {
					DEBUG(__FILE__,__LINE__,"i=%d narg=%d",i,narg);
					ERROR(__FILE__,__LINE__,"cannot parse IL");
					exit(-1);
				}

				/* fixup clargtab */
				clargtab[narg-1].e_nxt = 0;

				nsym = 1; // main
				nsym += 2; // cb0,cb1

				for(i=0;i<narg;i++) {

					DEBUG(__FILE__,__LINE__,"checking argnum=%d '%s'",
						i,&clstrtab[clargtab[i].e_name]);

					if (clargtab[i].e_addrspace == __ADDRSPACE_ATI_IM2D_RO) {

						++nsym;

					} else if (clargtab[i].e_addrspace == __ADDRSPACE_ATI_IM2D_WO) {

						++nsym;

					}

				}
				
				clsymtab = (struct clsymtab_entry*)
					calloc(nsym,sizeof(struct clsymtab_entry));


				isym = 0;

				clsymtab[isym++] = init_clsymtab_entry(
					str_main,'F',0,0,__TYPE_VOID,1,1,0,0,narg-1,1);

				clsymtab[isym++] = init_clsymtab_entry(
					str_cb0,'S',0,0,__TYPE_OPAQUE,1,1,0,0,0,0);

				clsymtab[isym++] = init_clsymtab_entry(
					str_cb1,'S',0,0,__TYPE_OPAQUE,1,1,0,0,0,0);


				int inum = 0;
				int onum = 0;
				for(iarg=0;iarg<narg;iarg++) {

					DEBUG(__FILE__,__LINE__,"checking argnum=%d '%s'",
						iarg,&clstrtab[clargtab[iarg].e_name]);

					if (clargtab[iarg].e_addrspace == __ADDRSPACE_ATI_IM2D_RO) {

						clsymtab[isym++] = init_clsymtab_entry(
                  	clstrtab_sz,'S',0,0,__TYPE_OPAQUE,1,1,0,0,0,0);

						snprintf(buf1,256,"i%d",inum++);
						strncpy(&clstrtab[clstrtab_sz],buf1,256);
						clstrtab_sz += strnlen(buf1,256) + 1;

					} else if (
						clargtab[iarg].e_addrspace == __ADDRSPACE_ATI_IM2D_WO) {

						clsymtab[isym++] = init_clsymtab_entry(
                  	clstrtab_sz,'S',0,0,__TYPE_OPAQUE,1,1,0,0,0,0);

						snprintf(buf1,256,"o%d",onum++);
						strncpy(&clstrtab[clstrtab_sz],buf1,256);
						clstrtab_sz += strnlen(buf1,256) + 1;

					}

				}				

				if (isym != nsym) {
					ERROR(__FILE__,__LINE__,"cannot parse IL");
					return(0);
				}

				DEBUG(__FILE__,__LINE__,"narg=%d nsym=%d",narg,nsym);



				/* write CAL imgage into a buffer */

				CALvoid* imgbuf = malloc(imgsz);
				calclImageWrite(imgbuf, imgsz, img);
				char* ppp = (char*)imgbuf;
				DEBUG(__FILE__,__LINE__,"is cal image written same? %s",ppp);
				DEBUG(__FILE__,__LINE__,"imgsz imgbuf %d %p\n",imgsz,imgbuf);
				struct clprgb_entry clprgb_e = {
					clstrtab_sz,0,5,0,imgsz
				};
				strncpy(clstrtab+clstrtab_sz,filebase,strnlen(filebase,256));
				clstrtab_sz += strnlen(filebase,256) + 1;


#if(0)
/****************/
CALdevice dev = 0;
   calDeviceOpen(&dev,0);
   CALcontext calctx = 0;
   calCtxCreate(&calctx,dev);
	CALmodule mod = 0;
	err = calModuleLoad(&mod,calctx,img);
	printf("calModuleLoad %d\n",err);
	CALfunc func = 0;
	err = calModuleGetEntry(&func,calctx,mod,"main");
	printf("calModuleGetEntry %d\n",err);
	err = calModuleGetEntry(&func,calctx,mod,"__OpenCL_test_arg_1_1_kern_kernel");
	printf("calModuleGetEntry %d\n",err);
	err = calModuleGetEntry(&func,calctx,mod,"test_arg_1_1_kern_kernel");
	printf("calModuleGetEntry %d\n",err);
	err = calModuleGetEntry(&func,calctx,mod,"1174");
	printf("calModuleGetEntry %d\n",err);
exit(1);	
/****************/
#endif

				/* now build elf/cl object */

				snprintf(buf1,256,"%s/%s.elfcl",wd,filebase);
				int fd = open(buf1,O_WRONLY|O_CREAT,S_IRWXU);
				err = elfcl_write(fd,
					0,0,
					0,0, 0,0,
					&clprgb_e,1, imgbuf, imgsz,
					clsymtab,nsym,
					clargtab,narg,
					clstrtab,clstrtab_sz
				);
				close(fd);

//				__mapfile(buf1,filesz_elfcl,pfile_elfcl);
//				printf("filesz_elfcl pfile_elfcl %d %p\n",filesz_elfcl,pfile_elfcl);


				/* now build .so that will be used for link */

				__command("cd %s; gcc -g -shared -Wl,-soname,%s.so -o %s.so"
					" __elfcl_rt.o %s.elfcl 2>&1",
					wd,filebase,filebase,filebase,filebase,filebase);
				__log(p2,"]%s\n",buf1); \
				__execshell(buf1,p2);

		}


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





