/* computil.h
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


#ifndef _COMPUTIL_H
#define _COMPUTIL_H


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

#define _GNU_SOURCE
#include <stdio.h>

#define DEFAULT_BUF1_SZ 16384
#define DEFAULT_BUF2_SZ 16384
#define DEFAULT_BUILD_LOG_SZ 256

#define DEFAULT_CMD_SZ 16384


static char* buf1 = 0;


#define CLERROR_BUILD_FAILED -1


#define __check_error( cmd ) do { \
	int err = cmd; \
	if (err) return(err); \
	} while(0)


static int write_file_cl( char* file, size_t filesz, char* pfile )
{
	FILE* fp = fopen(file,"w");
	printcl( CL_DEBUG "trying to write %d bytes",filesz);
	printcl( CL_DEBUG "last char is (int)%d",(int)pfile[filesz-1]);
	if (pfile[filesz-1] == '\0') {
		fprintf(fp,pfile);
	} else if (fwrite(pfile,1,filesz,fp) != filesz) {
		printcl( CL_ERR "error: write '%s' failed",file);
		return(-1);
	}
	fprintf(fp,"/* end */\n");
	fclose(fp);
	return(0);
}


static int write_file_cpp( char* file, size_t filesz, char* pfile )
{
	FILE* fp = fopen(file,"w");
	fprintf(fp,"#include \"opencl_lift.h\"\nextern \"C\" {\n");
	printcl( CL_DEBUG "trying to write %d bytes",filesz);
	if (pfile[filesz-1] == '\0') {
		fprintf(fp,pfile);
	} else if (fwrite(pfile,1,filesz,fp) != filesz) {
		printcl( CL_ERR "error: write '%s' failed",file);
		return(-1);
	}
	fprintf(fp,"\n}\n");
	fprintf(fp,"/* end */\n");
	fclose(fp);
	return(0);
}


#define __mapfile(file,filesz,pfile) \
	__check_error( mapfile(file,&filesz,&pfile) )


/*
static int mapfile( char* file, size_t* pfilesz, char** ppfile) 
{
	int fd = open(file,O_RDONLY);
	struct stat fst; fstat(fd,&fst);
	if (fst.st_size == 0 || !S_ISREG(fst.st_mode)) {
		printcl( CL_ERR "error: open failed on '%s'\n",file);
		return(CLERROR_BUILD_FAILED);
	}
	*pfilesz = fst.st_size;
	*ppfile = mmap(0,*pfilesz,PROT_READ,MAP_PRIVATE,fd,0);
	close(fd);
}
*/

#define __command(fmt,...) \
	snprintf(buf1,DEFAULT_BUF1_SZ,fmt,##__VA_ARGS__); 


/* XXX note that logbuf is not protected from overfull, fix this -DAR */
#define __log(p,fmt,...) do { \
	p += snprintf(p,__CLMAXSTR_LEN,fmt,##__VA_ARGS__); \
	} while(0);

/* XXX note that logbuf is not protected from overfull, fix this -DAR */
#define __execshell(command,p) do { \
	char c; \
	printcl( CL_DEBUG "__execshell: %s",command); \
	FILE* fp = popen(command,"r"); \
	while((c=fgetc(fp)) != EOF) *p++ = c; \
	err = pclose(fp); \
	} while(0);


static void remove_work_dir(char* wd)
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


static void append_str( char** pstr1, char* str2, char* sep, size_t n )
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
	append_str(log,msg,0,0); \
	if (!coprthr_tmp) remove_work_dir(wd); \
	return(CL_BUILD_PROGRAM_FAILURE); \
} }while(0)

/*
static int __test_file( char* file ) 
{
	struct stat s;
	int err = stat(file,&s);
	printcl( CL_DEBUG "__test_file %d", err);
	return (stat(file,&s));
}
*/

#define __shell_command( fmt, ... ) do { \
   __command(fmt,##__VA_ARGS__); \
   __log(logp,"]%s\n",buf1); \
   __execshell(buf1,logp); \
   } while(0)

#define __assert_file( fmt, ... ) do { \
      snprintf(fullpath,256,fmt,##__VA_ARGS__); \
      __check_err(__test_file(fullpath), \
         "internal error: assert file failed "); \
   } while(0)

static int copy_file_to_mem( 
	const char* path, unsigned char** p_buf, size_t* p_sz )
{

	struct stat fst;
	stat(path,&fst);

	if (S_ISREG(fst.st_mode) && fst.st_size > 0) {

		int fd = open(path,O_RDONLY,0);
		if (fd < 0) return(-1);

		size_t fsz = fst.st_size;
		*p_sz = fsz;
      *p_buf = (unsigned char*)malloc(fsz);
      void* p = mmap(0,fsz,PROT_READ,MAP_PRIVATE,fd,0);
      memcpy(*p_buf,p,fsz);
      munmap(p,fsz);
		close(fd);

		return(0);

	} else return(-1);

}

static int get_nsym( const char* wd, const char* file_cl, const char* opt )
{
	int nsym = -1;

	char cmd[DEFAULT_BUF1_SZ];	

   snprintf(cmd,DEFAULT_BUF1_SZ,"cd %s;"
      "cpp -x c++ -I" INSTALL_INCLUDE_DIR " %s %s "
      " | awk -v prog=\\\"%s\\\" "
      "'BEGIN { pr=0; }"
      " { "
      "   if($0~/^#/ && $3==prog) pr=1;"
      "   else if ($0~/^#/) pr=0;"
      "   if ($0!~/^#/ && pr==1) print $0;"
      " }' | xclnm -n -d - ",wd,file_cl,opt,file_cl);

      printcl( CL_DEBUG "%s", buf1);

	FILE* fp = popen(cmd,"r");
	fscanf(fp,"%d",&nsym);
	pclose(fp);

	return nsym;
}

static int build_clsymtab( 
	const char* wd, const char* file_cl, const char* opt,
	int nsym, unsigned int* p_narg,
	struct clsymtab_entry** p_clsymtab, 
	size_t* p_clstrtab_alloc_sz, size_t* p_clstrtab_sz, char** p_clstrtab
) 
{
	*p_clsymtab = (struct clsymtab_entry*)
         calloc(nsym,sizeof(struct clsymtab_entry));

	*p_clstrtab_sz = 0;
	*p_clstrtab_alloc_sz = nsym*1024;
	*p_clstrtab = malloc(*p_clstrtab_alloc_sz);
	(*p_clstrtab)[(*p_clstrtab_sz)++] = '\0';

	int i=0;
	*p_narg = 1;   /* starts at 1 to include the (null_arg) -DAR */
	int ii;
	unsigned char kind;
	char* name = (char*)malloc(256);
	int datatype;
	int vecn;
	int arrn;
	int addrspace;
	int ptrc;
	int n;
	int arg0;

	char cmd[DEFAULT_BUF1_SZ];	

	snprintf(cmd,DEFAULT_BUF1_SZ, 
		"cd %s; "
		" cpp -x c++ -I" INSTALL_INCLUDE_DIR " %s %s "
		" | awk -v prog=\\\"%s\\\" "
		"'BEGIN { pr=0; }"
		" { "
		"   if($0~/^#/ && $3==prog) pr=1;"
		"   else if ($0~/^#/) pr=0;"
		"   if ($0!~/^#/ && pr==1) print $0;"
		" }' | xclnm --clsymtab -d -c - ",wd,file_cl,opt,file_cl);

	FILE* fp = popen(cmd,"r");
	while (!feof(fp)) {
		if (fscanf(fp,"%d %c %s %d %d %d %d %d %d %d",
			&ii, &kind, name,
			&datatype,&vecn,&arrn,&addrspace,&ptrc,
			&n,&arg0)==EOF) break;
		if (ii!=i) {
			printcl( CL_ERR "cannot parse output of xclnm");
			exit(-2);
		}
		(*p_clsymtab)[i] = init_clsymtab_entry(*p_clstrtab_sz,kind,0,0,datatype,
			vecn,arrn,addrspace,ptrc,n,arg0,0);

		strncpy(&((*p_clstrtab)[*p_clstrtab_sz]),name,256);
		*p_clstrtab_sz += strnlen(name,256) + 1;

		++i;
		*p_narg += n;
	}
	pclose(fp);

	free(name);

	return(0);
}


static int scan_for_sym_e32( 
	const char* wd, const char* file_obj,
	int nsym,
	struct clsymtab_entry* clsymtab, char* clstrtab,
	unsigned int* p_addr_core_local_data,
	char** log
) 
{
	int i;

	char* cmd = 0;

	unsigned int addr;
	unsigned char kind;
	char name[256];
//	name[0]='\0';

//	cmd = (char*)malloc(128);

	asprintf(&cmd,"cd %s; nm --defined-only %s",wd,file_obj);
	asprintf(log,"execshell> %s\n",cmd);
	printcl( CL_DEBUG "> %s",cmd);

	FILE* fp = popen(cmd,"r");
	while (!feof(fp)) {
		addr = ~0;

		if (fscanf(fp,"%x %c %s", &addr, &kind, (char*)&name)==EOF) break;

		if (addr == ~0) break;

		if (!strcmp(name,"_core_local_data")) {
			if (!(*p_addr_core_local_data)) *p_addr_core_local_data = addr;
			else if (*p_addr_core_local_data != addr)
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

	if (!(*p_addr_core_local_data)) {
		printcl( CL_ERR "addr_core_local_data not found");
		return(-1);
	} else {
		printcl( CL_DEBUG "addr_core_local_data = 0x%x",
			*p_addr_core_local_data);
	}

	return(0);

}
		
	
static int scan_for_su_e32( 
	const char* wd, const char* filebase,
	int nsym,
	struct clsymtab_entry* clsymtab, char* clstrtab,
	char** log
) 
{
	int i;

	char* cmd = 0;

	char* name = (char*)malloc(256);
	int su;

	asprintf(&cmd,"cd %s; sed 's/(.*)//' %s.su |awk '{print $2 \" \" $3}'",
		wd,filebase);
	asprintf(log,"execshell> %s\n",cmd);
	printcl( CL_DEBUG "execshell> %s",cmd);

	FILE* fp = popen(cmd,"r");
	if (ferror(fp)) return -1;
	while (!feof(fp)) {

		if (fscanf(fp,"%s %d",name,&su)==EOF) break;
		printcl( CL_DEBUG "stack usage: '%s' %d\n",name,su);

		for(i=0;i<nsym;i++) {
			if (!strcmp(name,clstrtab + clsymtab[i].e_name)) {
				clsymtab[i].e_su = su;
				printcl( CL_DEBUG "setting su for '%s' to %d",name,su);
				break;
			}
		}

	}
	pclose(fp);


	asprintf(&cmd,
		"cd %s; awk -F: '{print $4}' e32_kcall3_%s.su"
		" | awk '{print $1 \" \" $2}'",
		wd,filebase);
	asprintf(log,"execshell> %s\n",cmd);
	printcl( CL_DEBUG "execshell> %s",cmd);

	fp = popen(cmd,"r");
	if (ferror(fp)) return -1;
	while (!feof(fp)) {
		if (fscanf(fp,"%s %d",name,&su)==EOF) break;
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

	free(name);

	return(0);
}


static int build_clargtab( 
	const char* wd, const char* file_cl, const char* opt,
	int nsym, int narg,
	struct clargtab_entry** p_clargtab, 
	size_t* p_clstrtab_alloc_sz, size_t* p_clstrtab_sz, char** p_clstrtab
) 
{
	*p_clargtab = (struct clargtab_entry*)
         calloc(narg,sizeof(struct clargtab_entry));

	int i=0;
	
	int ii;
	char* name = (char*)malloc(256);
	char* aname = (char*)malloc(256);
	int datatype;
	int vecn;
	int arrn;
	int addrspace;
	int ptrc;

	int argn;

	char cmd[DEFAULT_BUF1_SZ];	

	snprintf(cmd,DEFAULT_BUF1_SZ, 
		"cd %s;"
		"cpp -x c++ -I" INSTALL_INCLUDE_DIR " %s %s "
		" | awk -v prog=\\\"%s\\\" "
		"'BEGIN { pr=0; }"
		" { "
		"   if($0~/^#/ && $3==prog) pr=1;"
		"   else if ($0~/^#/) pr=0;"
		"   if ($0!~/^#/ && pr==1) print $0;"
		" }' | xclnm --clargtab -d -c - ",wd,file_cl,opt,file_cl);
		
	FILE* fp = popen(cmd,"r");
	while (!feof(fp)) {

		if (fscanf(fp,"%d %s %d %d %d %d %d %d %s ",
			&ii, aname,
			&datatype,&vecn,&arrn,&addrspace,&ptrc,
			&argn,name)==EOF) { break; }
		
		if (ii!=i) return(-2);

		(*p_clargtab)[i] = init_clargtab_entry(0,datatype,vecn,arrn,addrspace,
			ptrc,*p_clstrtab_sz,argn);

		strncpy(&(*p_clstrtab)[*p_clstrtab_sz],aname,256);
		*p_clstrtab_sz += strnlen(aname,256) + 1;

		++i;
	}
	pclose(fp);
	free(name);
	free(aname);
	return(0);
}


static int exec_shell(char* cmd, char** log)
{
	size_t tmplog_alloc_sz = 128;
	char* tmplog = (char*)malloc(tmplog_alloc_sz);
	char* p = tmplog;
	*(p++) = '\0';

	asprintf(log,"execshell> %s\n",cmd);
   printcl( CL_DEBUG "execshell> %s",cmd);

   FILE* fp = popen(cmd,"r"); 
	char c;
   while(!feof(fp) && !ferror(fp)) {
		c=fgetc(fp);
		if (feof(fp)) break;
		if (p-tmplog > tmplog_alloc_sz-1) {
			size_t offset = (size_t)(p-tmplog);
			tmplog_alloc_sz += 128;
			tmplog = realloc(tmplog,tmplog_alloc_sz);
			p = tmplog + offset;
		}
		*(p++) = c;
	}
   pclose(fp);

	asprintf(log,"\n%s",tmplog);

	return(0);
}

#endif

