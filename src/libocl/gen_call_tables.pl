#!/usr/bin/perl
#
# Program to open the password file, read it in,
# print it, and close it again.

$master = './oclcall.master';
open(MASTER, $master);
@lines = <MASTER>;
close(MASTER);

### 
### generate oclcall.h
###

# oclcallent { call, narg, trace } 

open(OUT, ">oclcall.h");

printf OUT "\n#ifndef _OCLCALL_H\n";
printf OUT "#define _OCLCALL_H\n\n";

printf OUT "\n#include <CL/cl.h>\n\n";

printf OUT "\nstruct oclent_struct {\n";
printf OUT "\tunsigned int ocl_narg;\n";
printf OUT "\tvoid* ocl_call;\n";
printf OUT "\tstruct ocltrace_struct* ocl_trace;\n";
printf OUT "};\n";
printf OUT "\n#define oclent_sz sizeof(struct oclent_struct)\n";

printf OUT "\ntypedef void (*cl_pfn_notify_t)(const char*, const void*, size_t, void*);\n";
printf OUT "typedef void (*cl_pfn_notify2_t)(cl_program , void* );\n";
printf OUT "typedef void (*cl_user_func_t)(void*);\n\n";

$i=0;
foreach $l (@lines) {
	if (!($l =~ /^[ \t]*#.*/)) {
		@fields = split(' ', $l);
		$name = $fields[0];
		if ($name =~ /^[a-zA-Z].*/) {
			unless ($name =~ /reserved/) {
				printf OUT "#define OCLCALL_$name\t$i\n";
			}
			$i = $i + 1;
		}
	}
}

printf OUT "\n#define OCLCALL_NARG_reserved 0\n\n";

$i=0;
foreach $l (@lines) {
	if (!($l =~ /^[ \t]*#.*/)) {
		@fields = split(' ', $l);
		$name = $fields[0];
		if ($name =~ /^[a-zA-Z].*/) {
			unless ($name =~ /reserved/) {
				$tlist = $fields[3];
				@args = split(',',$tlist);
				if ($tlist =~ /^void$/) { $narg = 0; }
				else { $narg = $#args+1; }
				printf OUT "#define OCLCALL_NARG_$name\t$narg\n";
			}
		}
#		$i = $i + 1;
	}
}

printf OUT "\nextern char* oclcallnames[];\n";
printf OUT "\nextern struct oclent_struct empty_oclent[];\n";
printf OUT "extern unsigned int oclncalls;\n";

printf OUT "\n#endif\n";

close(OUT);


###
### generate oclcall.c
###

open(OUT, ">oclcall.c");
printf OUT "\n#include <stdio.h>\n";
printf OUT "\n#include <CL/cl.h>\n";
printf OUT "#include \"oclcall.h\"\n";

printf OUT "\nstatic void _ocl_no_implementation()\n";
printf OUT "{\n";
printf OUT "\tfprintf(stderr,\"libocl: fatal error: \"\n";
printf OUT "\t\t\" platform provides no implementation for this call\\n\");\n";
printf OUT "\texit(-1);\n";
printf OUT "}\n";

printf OUT "\nchar* oclcallnames[] = {\n";
$i=0;
foreach $l (@lines) {
	if (!($l =~ /^[ \t]*#.*/)) {
		@fields = split(' ', $l);
		$name = $fields[0];
		if ($name =~ /^[a-zA-Z].*/) {
			if ($i > 0) { printf OUT ",\n"; }
			printf OUT "\t\"$name\"";
			$i = $i + 1;
		}
	}
}
printf OUT "\n};\n\n";

printf OUT "\nstruct oclent_struct empty_oclent[] = {\n";
$i=0;
foreach $l (@lines) {
	if (!($l =~ /^[ \t]*#.*/)) {
		@fields = split(' ', $l);
		$name = $fields[0];
		if ($name =~ /^[a-zA-Z].*/) {
			if ($i > 0) { printf OUT ",\n"; }
			printf OUT "\t{ OCLCALL_NARG_$name, _ocl_no_implementation, 0 }";
			$i = $i + 1;
		}
	}
}
printf OUT "\n};\n\n";

printf OUT "\nunsigned int oclncalls = sizeof(oclcallnames)/sizeof(char*);\n";

###

foreach $l (@lines) {
	if (!($l =~ /^[ \t]*#.*/)) {
		@fields = split(' ', $l);
		$name = $fields[0];
		if ($name =~ /^[a-zA-Z].*/) {
			$type = $fields[1];
			$retype = $fields[2];
			$tlist = $fields[3];
			$tlist =~ s/~/ /g;
			@args = split(',',$tlist);

			$atlist = "";
			$alist = "";
			$j=0;
			foreach $a (@args) {
				if ($j > 0) { $atlist .=",$a a$j"; }
				else { $atlist .= "$a a$j"; }
				if ($j > 0) { $alist .=",a$j"; }
				else { $alist .= "a$j"; }
				$j += 1;
			}

			if ($type == 0) {

				printf OUT "\n$retype $name($atlist)\n{\n";
				printf OUT "\tvoid** cv = *(void***)a0;\n";

				printf OUT "\ttypedef $retype (*pf_t) ($tlist);\n";

				printf OUT "\t$retype rv = ((pf_t)(cv[OCLCALL_$name\]))($alist);\n";
				printf OUT "\treturn rv;\n";	
				printf OUT "}\n\n";

			} elsif ($type == 1) {

				printf OUT "\n$retype $name($atlist)\n{\n";
				printf OUT "\tvoid** cv = *(void***)a0;\n";

				printf OUT "\ttypedef $retype (*pf_t) ($tlist);\n";

				printf OUT "\t$retype rv = ((pf_t)(cv[OCLCALL_$name\]))($alist);\n";
				printf OUT "\t*(void***)rv = cv;\n";
				printf OUT "\treturn rv;\n";	
				printf OUT "}\n\n";

			}

			$i = $i + 1;
		}
	}
}

printf OUT "\ncl_int clUnloadCompiler(void)\n";
printf OUT "{ return (cl_int)CL_SUCCESS; }\n";

# custom

printf OUT "\ncl_context clCreateContext(\n";
printf OUT "\tconst cl_context_properties* a0,\n";
printf OUT "\tcl_uint a1,const cl_device_id* a2,\n";
printf OUT "\tcl_pfn_notify_t a3,void* a4,cl_int* a5\n";
printf OUT ")\n";
printf OUT "{\n";
printf OUT "\tcl_context_properties* p = (cl_context_properties*)a0;\n";
printf OUT "\tint n=0;\n";
printf OUT "\tfor(;*p != 0 && n<256; p+=2,n++)\n";
printf OUT "\t\tif (*p == CL_CONTEXT_PLATFORM) { ++p; break; }\n";
printf OUT "\tif (*p==0 || n==256) return (cl_context)0;\n";
printf OUT "\tvoid** cv = *(void***)(*p);\n";
printf OUT "\ttypedef cl_context (*pf_t) (const cl_context_properties*,\n";
printf OUT "\t\tcl_uint,const cl_device_id*,cl_pfn_notify_t,void*,cl_int*);\n";
printf OUT "\tcl_context rv \n";
printf OUT "\t\t= ((pf_t)(cv[OCLCALL_clCreateContext]))(a0,a1,a2,a3,a4,a5);\n";
printf OUT "\t*(void***)rv = cv;\n";
printf OUT "\treturn rv;\n";
printf OUT "}\n";

printf OUT "\ncl_context clCreateContextFromType(\n";
printf OUT "\tconst cl_context_properties* a0,\n";
printf OUT "\tcl_device_type a1,cl_pfn_notify_t a2,void* a3,cl_int* a4\n";
printf OUT ")\n";
printf OUT "{\n";
printf OUT "\tcl_context_properties* p = (cl_context_properties*)a0;\n";
printf OUT "\tint n=0;\n";
printf OUT "\tfor(;*p != 0 && n<256; p+=2,n++)\n";
printf OUT "\t\tif (*p == CL_CONTEXT_PLATFORM) { ++p; break; }\n";
printf OUT "\tif (*p==0 || n==256) return (cl_context)0;\n";
printf OUT "\tvoid** cv = *(void***)(*p);\n";
printf OUT "\ttypedef cl_context (*pf_t) (const cl_context_properties*,\n";
printf OUT "\t\tcl_device_type,cl_pfn_notify_t,void*,cl_int*);\n";
printf OUT "\tcl_context rv \n";
printf OUT "\t\t= ((pf_t)(cv[OCLCALL_clCreateContextFromType]))(a0,a1,a2,a3,a4);\n";
printf OUT "\t*(void***)rv = cv;\n";
printf OUT "\treturn rv;\n";
printf OUT "}\n";

printf OUT "\ncl_int clWaitForEvents(\n";
printf OUT "\tcl_uint a0,const cl_event* a1\n";
printf OUT ")\n";
printf OUT "{\n";
printf OUT "\tif (a0<1) return CL_INVALID_VALUE;\n";
printf OUT "\tif (a1=0) return CL_INVALID_EVENT;\n";
printf OUT "\tvoid** cv = *(void***)(*a1);\n";
printf OUT "\ttypedef cl_int (*pf_t) (cl_uint,const cl_event*);\n";
printf OUT "\tcl_int rv \n";
printf OUT "\t\t= ((pf_t)(cv[OCLCALL_clWaitForEvents]))(a0,a1);\n";
printf OUT "\treturn rv;\n";
printf OUT "}\n";

printf OUT "/*\n";

printf OUT "\ncl_int clCreateKernelsInProgram(\n";
printf OUT "\tcl_program a0,cl_uint a1,cl_kernel* a2,cl_uint* a3\n";
printf OUT ")\n";
printf OUT "{\n";
printf OUT "\tvoid** cv = *(void***)a0;\n";
printf OUT "\ttypedef cl_int (*pf_t) (cl_program,cl_uint,cl_kernel*,cl_uint*);\n";
printf OUT "\tcl_int rv = ((pf_t)(cv[OCLCALL_clCreateKernelsInProgram]))(a0,a1,a2,a3);\n";
printf OUT "\tint n=0;\n";
printf OUT "\tfor(n=0;n<a1;n++,a2++) *(void***)(*a2) = cv;\n";
printf OUT "\treturn rv;\n";
printf OUT "}\n";

printf OUT "*/\n";

close(OUT);

