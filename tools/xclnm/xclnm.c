/* xclnm2.c	(dev)
 *
 * Copyright (c) 2008-2010 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 (GPLv3)
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "version.h"
#include "elf_cl.h"
#include "xclnm.h"
#include "xclnm_node.h"
#include "xclnm_gram.h"
//#include "../../src/libocl/ocl_types.h"
#include "ocl_types.h"


struct type_entry {
	int ival;
	size_t sz;
	char* c_type;
	char* cl_type;
};

struct type_entry type_table[] = {
	{ TYPEID_OPAQUE, 8, "void", "void" },
	{ TYPEID_VOID, 0, "void", "void" },
	{ TYPEID_CHAR, 1, "char", "char" },
	{ TYPEID_SHORT, 2, "short", "short" },
	{ TYPEID_INT, 4, "int", "int" },
	{ TYPEID_LONG, 8, "long", "long" },
	{ TYPEID_UCHAR, 1, "unsigned char", "uchar" },
	{ TYPEID_USHORT, 2, "unsigned short", "ushort" },
	{ TYPEID_UINT, 4, "unsigned int", "uint" },
	{ TYPEID_ULONG, 8, "unsigned long", "ulong" },
	{ TYPEID_FLOAT, 4, "float", "float" },
	{ TYPEID_DOUBLE, 8, "double", "double" },
};

int ntypes = sizeof(type_table)/sizeof(struct type_entry);


char* symbuf = 0;
int symbuf_sz = 0;
node_t* cur_nptr;

char* typbuf[16384];
int ntypbuf = 0;

char* locbuf;
size_t locbufsz;


int 
add_str( char* buf, int* sz, const char* s)
{
	int i;
	if (STRBUFSIZE-(*sz)-1 < strnlen(s,STRBUFSIZE-1)) return(0);
	for(i=0; i< (*sz); i+=strnlen(buf+i,STRBUFSIZE-i-1)+1) 
		if (!strncmp(buf+i,s,STRBUFSIZE-i)) return(i);
	strncpy(buf+i,s,STRBUFSIZE-i);
	(*sz) += strnlen(s,STRBUFSIZE-i)+1;
	return(i);
}


int
add_typedef( char* sym )
{ typbuf[ntypbuf++] = sym; }

int
is_type(char* s)
{
	int i;
	for(i=0;i<ntypbuf;i++) 
		if (!strncmp(typbuf[i],s,256)) return(1);
	return(0);
}


int calc_arg_sz( node_t* arg );
int calc_args_sz( node_t* args ); 
void fprintf_sym(FILE*, char*, int ll_style, int c_style);
//void fprintf_arg( FILE*, node_t* arg ); 
void fprintf_type( FILE*, node_t* arg ); 
void fprintf_c_type( FILE*, node_t* arg ); 



int type_ival2index(int ival);

extern FILE* __xclnm_yyin;


int main( int argc, char** argv)
{
	int i,j,k;
	char sbuf[1024];

	char* in_filename = 0;
	char* out_filename = 0;

	int opt_n = 0;
	int opt_nm = 1;
	int opt_kcall = 0;
	int opt_clsymtab = 0;
	int opt_clargtab = 0;
	int opt_func_def = 1;
	int opt_func_dec = 1;
	int opt_global_dec = 1;
	int opt_ll_style = 0;
	int opt_c_style = 0;
	int opt_kcall_debug = 0;
	int opt_debug = 0;
	int opt_verbose = 0;
	int opt_version = 0;
	int opt_help = 0;


	int n = 1;
	while(n<argc) {

		char* arg = argv[n++];

		if (!strncmp(arg,"-n",4)) {
			opt_n = 1;
			opt_nm = 0;
			opt_kcall = 0;
			opt_clsymtab = 0;
			opt_clargtab = 0;
		}

		else if (!strncmp(arg,"--nm",4)) {
			opt_n = 0;
			opt_nm = 1;
			opt_kcall = 0;
			opt_clsymtab = 0;
			opt_clargtab = 0;
		}

		else if (!strncmp(arg,"--kcall-debug",13)) {
			opt_debug = 1;
		}

		else if (!strncmp(arg,"--kcall",7) || !strncmp(arg,"-k",2)) {
			opt_n = 0;
			opt_nm = 0;
			opt_kcall = 1;
			opt_clsymtab = 0;
			opt_clargtab = 0;
		}

		else if (!strncmp(arg,"--clsymtab",10) || !strncmp(arg,"-s",2)) {
			opt_n = 0;
			opt_nm = 0;
			opt_kcall = 0;
			opt_clsymtab = 1;
			opt_clargtab = 0;
		}

		else if (!strncmp(arg,"--clargtab",10) || !strncmp(arg,"-a",2)) {
			opt_n = 0;
			opt_nm = 0;
			opt_kcall = 0;
			opt_clsymtab = 0;
			opt_clargtab = 1;
		}


		else if (!strncmp(arg,"--func-def-only",15) || !strncmp(arg,"-d",2)) {
				opt_func_def = 1;
				opt_func_dec = 0;
				opt_global_dec = 0;
		}

		else if (!strncmp(arg,"--func-def",10)) opt_func_def = 1;

		else if (!strncmp(arg,"--no-func-def",13)) opt_func_def = 0;


		else if (!strncmp(arg,"--func-dec-only",15)) {
			opt_func_def = 0;
			opt_func_dec = 1;
			opt_global_dec = 0;
		}

		else if (!strncmp(arg,"--func-dec",10)) opt_func_dec = 1;

		else if (!strncmp(arg,"--no-func-dec",13)) opt_func_dec = 0;


		else if (!strncmp(arg,"--global-dec-only",17)) {
			opt_func_def = 0;
			opt_func_dec = 0;
			opt_global_dec = 1;
		}

		else if (!strncmp(arg,"--global-dec",12)) opt_global_dec = 1;

		else if (!strncmp(arg,"--no-global-dec",15)) opt_global_dec = 0;


		else if (!strncmp(arg,"-l",2)) {
			opt_ll_style = 1;
			opt_c_style = 0;
		}

		else if (!strncmp(arg,"-c",2)) {
			opt_ll_style = 0;
			opt_c_style = 1;
		}

		else if (!strncmp(arg,"-v",2)) opt_verbose = 1;

		else if (!strncmp(arg,"-q",2)) opt_verbose = 0;

		else if (!strncmp(arg,"--debug",7)) {
			opt_debug = 1;
		}

		else if ( !strncmp(arg,"--version",9)
			|| !strncmp(arg,"-V",2) ) opt_version=1;
		
		else if ( !strncmp(arg,"--help",6)
			|| !strncmp(arg,"-h",2) ) opt_help=1;
		
		else if ( !strncmp(arg,"-o",2)) {
			if (out_filename) {
				fprintf(stderr,"xclnm: error: multiple output files\n");
				opt_help = 1;
			} else {
				out_filename = argv[n++];
			}
		}
		
		else if (in_filename) {
			fprintf(stderr,"xclnm: error: multiple input files\n");
			opt_help = 1;
		}

		else {
			in_filename = arg;
		}

	}


	if (opt_version) {

		printf("xclnm ( " COPRTHR_VERSION_STRING " )\n" );
		printf("Copyright 2008-2011 Brown Deer Technology, LLC.\n");
		printf( GPL3_NOTICE );

		exit(0);

	} else if (opt_help) {

		printf("USAGE: xclnm [options] <input .llvm file>\n");

		printf("\n-n\t\t\tReport number of symbols\n");
		printf("--nm\t\t\tReport symbol information in nm style\n");
		printf("--kcall\t\t\tGenerate kernel call wrappers for libocl/x86_64\n");
		printf("--clsymtab\t\tGenerate CL symbol table\n");
		printf("--clargtab\t\tGenerate CL argument table\n");
		printf("--func-def\t\tInclude function definitions\n");
		printf("--no-func-def\t\tDo not include function definitions\n");
		printf("-d,--func-def-only\tOnly include function definitions\n");
		printf("--func-dec\t\tInclude function declarations\n");
		printf("--no-func-dec\t\tDo not include function declarations\n");
		printf("--func-dec-only\t\tOnly include function declarations\n");
		printf("--global-dec\t\tInclude global declarations\n");
		printf("--no-global-dec\t\tDo not include global declarations\n");
		printf("--global-dec-only\tOnly include global declarations\n");
		printf("-l\t\t\tUse LLVM-style for symbol names\n");
		printf("-c\t\t\tUse C-style for symbol names\n");
		printf("--kcall-debug\t\tAdd debug code to kcall wrappers\n");
		printf("-v\t\t\tVerbose\n");
		printf("-q\t\t\tQuiet\n");
		printf("--debug\t\t\tOutput xclnm debug information\n");
		printf("-V,--version\t\tOutput xclnm version information\n");
		printf("-h,--help\t\tOutput this help information\n");
		printf("-o <output file>\tRedirect output to file\n");
		printf("\n");

		exit(0);

	}


	symbuf = (char*)malloc(STRBUFSIZE);
	symbuf[symbuf_sz++] = '\0';
	locbuf = (char*)malloc(262144);
	locbufsz = 0;

	node_t* root = node_create();
	cur_nptr = root;



	/*
	 * open input file, call yyparse
	 * if successful, yyparse will assign a tree representing 
	 * the parsed assembly code to root
	 */

	if (in_filename) __xclnm_yyin = fopen(in_filename,"r");

	if (!__xclnm_yyin) fprintf(stderr,"xclnm: no input file\n");

	__xclnm_parse();

	if (opt_debug) {
		node_fprintf(stdout,root,0);
	}

	fclose(__xclnm_yyin);



	/* 
	 *	open output file, use stdout if none specified
	 */

	FILE* fp = (out_filename)? fopen(out_filename,"w") : stdout;

	if (!fp) {
		fprintf(stderr,
			"xclnm: error: open '%s' for output failed\n",out_filename);
		exit(-1);
	}


	node_t* nptr1;
	node_t* nptr2;
	node_t* nptr3;


	/* 
	 * print count of symbols
	 */

	if (opt_n) {

		int count = 0;

		for(nptr1=root;nptr1;nptr1=nptr1->next) switch(nptr1->ntyp) {

			case NTYP_FUNC:

				if (opt_func_def && (nptr1->n_func.flags)&F_FUNC_DEF) ++count;
				if (opt_func_dec && (nptr1->n_func.flags)&F_FUNC_DEC) ++count;
				break;

			default: break;

		}

		fprintf(fp,"%d\n",count);
		exit(0);

	}


	/* 
	 * print nm-style table 
	 */

	if (opt_nm) {

		for(nptr1=root;nptr1!=0;nptr1=nptr1->next) switch(nptr1->ntyp) {

			case NTYP_EMPTY: break;

			case NTYP_FUNC:

				if ((opt_func_def && (nptr1->n_func.flags)&F_FUNC_DEF) 
					|| (opt_func_dec && (nptr1->n_func.flags)&F_FUNC_DEC)) {

					nptr2 = nptr1->n_func.args;

					n = node_count(nptr2);
				
					fprintf(fp,"%7d ",calc_args_sz(nptr2));

					if (nptr1->n_func.flags&F_FUNC_DEC) fprintf(fp,"f ");
					else if (nptr1->n_func.flags&F_FUNC_DEF) fprintf(fp,"F ");
					else fprintf(fp,"?");

					fprintf_sym(fp,
						symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);

					fprintf(fp,"\t%d",n);

					for(i=0;i<n;i++,nptr2=nptr2->next) 
						fprintf(fp," %d",calc_arg_sz(nptr2));

					fprintf(fp,"\n");

				}

				break;

			default: break;

		}

		exit(0);

	}


	/* 
	 * generate call wrappers
	 */

	char* s;

	if (opt_kcall) {

		fprintf(fp,"#include <stdio.h>\n");
		fprintf(fp,"#define __xcl_kcall__\n");
		fprintf(fp,"#include \"vcore.h\"\n");

		for(nptr1=root;nptr1;nptr1=nptr1->next) switch(nptr1->ntyp) {

			case NTYP_EMPTY: break;

			case NTYP_FUNC:

				if (nptr1->n_func.flags & F_FUNC_DEF) {

					n = node_count(nptr1->n_func.args);

			
					nptr2=nptr1->n_func.args;

					fprintf(fp,"\n/* F ");

					fprintf_sym(fp,
						symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);

					fprintf(fp," %d",calc_args_sz(nptr2));

					fprintf(fp,"\t%d",n);

					for(i=0;i<n;i++,nptr2=nptr2->next) 
						fprintf(fp," %d",calc_arg_sz(nptr2));

					fprintf(fp," */\n");


					/* print typedef */

					fprintf(fp,"typedef %s",type_table[type_ival2index(
						nptr1->n_func.rettype->n_type.datatype)].c_type);

					fprintf(fp,"(*__XCL_func_");

					fprintf_sym(fp,
						symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);

					fprintf(fp,"_t)(");

					for(i=0,nptr2=nptr1->n_func.args;i<n;i++,nptr2=nptr2->next) {
						if (i>0) fprintf(fp,", ");	
						fprintf_c_type(fp,nptr2->n_arg.argtype);
					}

					fprintf(fp,");\n");


					/* print call wrapper */

					fprintf(fp,"%s ",type_table[type_ival2index(
						nptr1->n_func.rettype->n_type.datatype)].c_type);

					fprintf(fp," __attribute__((noreturn))\n");

					fprintf(fp,"__XCL_call_");

					fprintf_sym(fp,
						symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);

					fprintf(fp,"(void* p)");

					fprintf(fp,"{\n");

					if (opt_kcall_debug) {
						fprintf(fp,"\tprintf(\"__XCL_call_");
						fprintf_sym(fp,
							symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);
						fprintf(fp,":\\n\");\n");
					}

					fprintf(fp,"\tstruct vcengine_data* edata"
						" = (struct vcengine_data*)p;\n");

					fprintf(fp,"\tstruct vc_data* data = __getvcdata();\n");


//					fprintf(fp,"\tint vcid = data->vcid;\n");

					if (opt_kcall_debug) {
						fprintf(fp,"\tprintf(\"__XCL_call_");
						fprintf_sym(fp,
							symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);
						fprintf(fp,": vcore[%%d] running\\n\",vcid);\n");
					}

					fprintf(fp,"\t++(edata->vc_runc);\n");

					fprintf(fp,
//						"\tif (!(setjmp(*(data->this_jbufp))))"
						"\tif (!(__vc_setjmp(*(data->this_jbufp))))"
//         			"longjmp(*(data->vcengine_jbufp),vcid+1);\n");
//         			"longjmp(*(data->vcengine_jbufp),0);\n");
         			"__vc_longjmp(*(data->vcengine_jbufp),0);\n");


					fprintf(fp,"\t((__XCL_func_");

					fprintf_sym(fp,
						symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);

					fprintf(fp,"_t)(edata->funcp))(\n");

					nptr2 = nptr1->n_func.args;

					for(i=0,nptr2=nptr1->n_func.args;i<n;i++,nptr2=nptr2->next) {

						fprintf(fp,"\t\t");
						if (i>0) fprintf(fp,",");	
						fprintf(fp,"*(");
						fprintf_c_type(fp,nptr2->n_arg.argtype);
						fprintf(fp,"*)edata->pr_arg_vec[%d]\n",i);

					}

					fprintf(fp,"\t);\n");

					fprintf(fp,"\t--(edata->vc_runc);\n");

//					fprintf(fp,"\tlongjmp(*(data->vcengine_jbufp),vcid+1);\n");
//					fprintf(fp,"\tlongjmp(*(data->vcengine_jbufp),0);\n");
					fprintf(fp,"\t__vc_longjmp(*(data->vcengine_jbufp),0);\n");

					if (opt_kcall_debug) {
						fprintf(fp,"\tprintf(\"__XCL_call_");
						fprintf_sym(fp,
							symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);
						fprintf(fp,": vcore[%%d] halt\\n\",vcid);\n");
					}

					fprintf(fp,"}\n");

				}

				break;

			default: break;

		}

		exit(0);

	}


	/* 
	 * print clsymtab table 
	 */

	if (opt_clsymtab) {

		j = 0;
		k = 0;

		for(nptr1=root;nptr1!=0;nptr1=nptr1->next) switch(nptr1->ntyp) {

			case NTYP_EMPTY: break;

			case NTYP_FUNC:

				if ((opt_func_def && (nptr1->n_func.flags)&F_FUNC_DEF) 
					|| (opt_func_dec && (nptr1->n_func.flags)&F_FUNC_DEC)) {

					nptr2 = nptr1->n_func.args;
					n = node_count(nptr2);
				
					fprintf(fp,"%4d ",j);

					if (nptr1->n_func.flags&F_FUNC_DEC) fprintf(fp,"f ");
					else if (nptr1->n_func.flags&F_FUNC_DEF) fprintf(fp,"F ");
					else fprintf(fp,"?");

					fprintf_sym(fp,
						symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);

					nptr3 = nptr1->n_func.rettype;
					fprintf(fp," %d %d %d %d %d %d %d",
						nptr3->n_type.datatype,
						nptr3->n_type.vecn,
						nptr3->n_type.arrn,
						nptr3->n_type.addrspace,
						nptr3->n_type.ptrc,
						n,k+1					/* the +1 allows a null in clargtab DAR */
					);

					fprintf(fp,"\n");

					++j;
					k += n;

				}

				break;

			default: break;

		}

		exit(0);

	}


	/* 
	 * print clargtab table 
	 */

	if (opt_clargtab) {

		j = 0;
		k = 0;

		fprintf(fp,"   0 (null) 0 0 0 0 0 0 ()\n");
		++j;

		for(nptr1=root;nptr1!=0;nptr1=nptr1->next) switch(nptr1->ntyp) {

			case NTYP_EMPTY: break;

			case NTYP_FUNC:

				if ((opt_func_def && (nptr1->n_func.flags)&F_FUNC_DEF) 
					|| (opt_func_dec && (nptr1->n_func.flags)&F_FUNC_DEC)) {

					nptr2 = nptr1->n_func.args;
					n = node_count(nptr2);
					k = 1;
	
					for(;nptr2!=0;nptr2=nptr2->next) {

						fprintf(fp,"%4d ",j);

//					if (nptr1->n_func.flags&F_FUNC_DEC) fprintf(fp,"f ");
//					else if (nptr1->n_func.flags&F_FUNC_DEF) fprintf(fp,"F ");
//					else fprintf(fp,"?");

					fprintf_sym(fp,
						symbuf+nptr2->n_arg.sym,opt_ll_style,opt_c_style);

					nptr3 = nptr2->n_arg.argtype;
					fprintf(fp," %d %d %d %d %d %d",
						nptr3->n_type.datatype,
						nptr3->n_type.vecn,
						nptr3->n_type.arrn,
						nptr3->n_type.addrspace,
						nptr3->n_type.ptrc,
						((k<n)?j+1:0)
					);

					fprintf(fp," (");
					fprintf_sym(fp,
						symbuf+nptr1->n_func.sym,opt_ll_style,opt_c_style);
					fprintf(fp,")");

					fprintf(fp,"\n");

					++j;
					++k;

					}

				}

				break;

			default: break;

		}

		exit(0);

	}


	fclose(fp);

}


int calc_arg_sz( node_t* arg )
{
	int i;

	if (arg->n_arg.argtype->n_type.ptrc > 0) {

		switch(arg->n_arg.argtype->n_type.addrspace) {

			case 0:
			case 3:
			case 5:
			default: 
				return(sizeof(void*));
			
		}

	} else for(i=0;i<ntypes;i++) {

		if (type_table[i].ival == arg->n_arg.argtype->n_type.datatype) 
			return(arg->n_arg.argtype->n_type.vecn * type_table[i].sz);

	}
}


int calc_args_sz( node_t* args ) 
{
	int sz = 0;
	while(args) {
		sz += calc_arg_sz(args);
		args=args->next;
	}
	return(sz);
}


int type_ival2index(int ival) 
{
	int i;
	for(i=0;i<ntypes;i++) if (type_table[i].ival == ival) return(i);
	return(-1);
}


void fprintf_sym( FILE* fp, char* sym, int ll_style, int c_style)
{
	char sbuf[1024];
	char* s = sym;

	if (!ll_style && (*s=='@' || *s=='%')) ++s;

	strncpy(sbuf,s,1024);

	if (c_style) {
		if (!strncmp(sbuf,"__OpenCL_",9)) strncpy(sbuf,sbuf+9,1024);
		size_t len = strnlen(sbuf,1024);
		if (!strncmp(sbuf+len-7,"_kernel",7)) sbuf[len-7]='\0';
	}

	fprintf(fp,"%s",sbuf);
}


void fprintf_type( FILE* fp, node_t* t )
{
	int j;
	fprintf(fp,"%s",type_table[type_ival2index(t->n_type.datatype)].cl_type);
	if (t->n_type.vecn>1) fprintf(fp,"%d ",t->n_type.vecn);
	for(j=0;j<t->n_type.ptrc;j++) fprintf(fp,"*");
}

void fprintf_c_type( FILE* fp, node_t* t )
{
	int j;
	fprintf(fp,"%s",type_table[type_ival2index(t->n_type.datatype)].c_type);
	if (t->n_type.vecn>1) fprintf(fp,"%d ",t->n_type.vecn);
	for(j=0;j<t->n_type.ptrc;j++) fprintf(fp,"*");
}


