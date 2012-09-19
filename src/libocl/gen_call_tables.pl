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
printf OUT "\n#include <CL/cl_gl.h>\n\n";

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
#for ($j = 0; $j <256; $j += 1) {
#	printf OUT "#define OCLCALL_generic_$j\t$j\n";
#}

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
		$i = $i + 1;
	}
}
#for ($j = $i; $j <256; $j += 1) {
#	printf OUT "static __inline void _generic_$j( void ) { printf(\"<generic> \" \"$j\" \"\\n\"); fflush(stdout); }\n";
#}
#for ($j = 0; $j <256; $j += 1) {
#	printf OUT "#define OCLCALL_NARG_generic_$j\t0\n";
#}

printf OUT "\nextern char* oclcallnames[];\n";
printf OUT "\nextern struct oclent_struct empty_oclent[];\n";
printf OUT "extern unsigned int oclncalls;\n";

#printf OUT "\n#define __DECL_API_CALLS( prefix, suffix ) \\\n";
#foreach $l (@lines) {
#	if (!($l =~ /^[ \t]*#.*/)) {
#		@fields = split(' ', $l);
#		$name = $fields[0];
#		if ($name =~ /^[a-zA-Z].*/) {
#			$type = $fields[1];
#			$retype = $fields[2];
#			$tlist = $fields[3];
#			$tlist =~ s/~/ /g;
#			@args = split(',',$tlist);
#			$atlist = "";
#			$alist = "";
#			$j=0;
#			foreach $a (@args) {
#				if ($j > 0) { $atlist .=",$a a$j"; }
#				else { $atlist .= "$a a$j"; }
#				if ($j > 0) { $alist .=",a$j"; }
#				else { $alist .= "a$j"; }
#				$j += 1;
#			}
#			if (!($name =~ /^reserved/)) { 
#				printf OUT "\t$retype prefix##$name##suffix($atlist);\\\n";
#			}
#		}
#	}
#}
#printf OUT "\n";

#printf OUT "\n#define __set_icd_call_vector( prefix, suffix ) { \\\n";
#$i=0;
#foreach $l (@lines) {
#	if (!($l =~ /^[ \t]*#.*/)) {
#		@fields = split(' ', $l);
#		$name = $fields[0];
#		if ($name =~ /^[a-zA-Z].*/) {
#			unless ($name =~ /reserved/) {
#				$tlist = $fields[3];
#				@args = split(',',$tlist);
#				if ($tlist =~ /^void$/) { $narg = 0; }
#				else { $narg = $#args+1; }
#				printf OUT "\tprefix##$name##suffix, \\\n";
#			}
#		}
#	$i = $i + 1;
#	}
#}
#for ($j = $i; $j <256; $j += 1) {
#	printf OUT "\t_generic_$j, \\\n";
#}
#printf OUT "\t}\n";

printf OUT "\n#endif\n";

close(OUT);


###
### generate oclcall.c
###

open(OUT, ">oclcall.c");
printf OUT "\n#include <stdio.h>\n";
printf OUT "\n#include <CL/cl.h>\n";
printf OUT "#include \"oclcall.h\"\n";
printf OUT "#include \"printcl.h\"\n";

printf OUT "\nstatic void _ocl_no_implementation()\n";
printf OUT "{\n";
printf OUT "\tERROR2(\"libocl: fatal error: \"\n";
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
#				printf OUT "\tDEBUG2(\"$name: %%p %%p\",&$name,(void*)a0);\n";
				printf OUT "\tvoid* oclent \n";
				printf OUT "\t\t= *(void**)a0;\n";
#				printf OUT "\tDEBUG2(\"\toclent=%%p\",oclent);\n";
				printf OUT "\tprintcl( CL_DEBUG \"$name (loader) %%p\",oclent);\n";
				printf OUT "\tprintcl( CL_DEBUG \"%%p\", ((void**)oclent)+OCLCALL_$name );\n";


				printf OUT "\ttypedef $retype (*pf_t) ($tlist);\n";

				printf OUT "\t$retype rv \n";
				printf OUT "\t\t= ((pf_t)(*(((void**)oclent)+OCLCALL_$name\)))($alist);\n";
				printf OUT "\treturn rv;\n";	
				printf OUT "}\n\n";

			} elsif ($type == 1) {

				printf OUT "\n$retype $name($atlist)\n{\n";
#				printf OUT "\tDEBUG2(\"$name:\");\n";
				printf OUT "\tvoid* oclent \n";
				printf OUT "\t\t= *(void**)a0;\n";
#				printf OUT "\tDEBUG2(\"\toclent=%%p\",oclent);\n";
				printf OUT "\tprintcl( CL_DEBUG \"$name (loader) %%p\",oclent);\n";

				printf OUT "\ttypedef $retype (*pf_t) ($tlist);\n";

				printf OUT "\t$retype rv \n";
				printf OUT "\t\t= ((pf_t)(*(((void**)oclent)+OCLCALL_$name\)))($alist);\n";
				printf OUT "\treturn rv;\n";	
				printf OUT "}\n\n";

			} elsif ($type == 2) {

				for($k=0; $k < $#args+1; $k+=1 ) {
					if ($args[$k] =~ /^cl_event\*$/) { last; }
				}

				if ($k == $#args + 1) {
					print "internal error: cannot find cl_event* argument\n";
					print "$name\n";
					print "$k\n";
					exit;
				}

				printf OUT "\n$retype $name($atlist)\n{\n";
#				printf OUT "\tDEBUG2(\"$name:\");\n";
				printf OUT "\tvoid* oclent \n";
				printf OUT "\t\t= *(void**)a0;\n";
#				printf OUT "\tDEBUG2(\"\toclent=%%p\",oclent);\n";
				printf OUT "\tprintcl( CL_DEBUG \"$name (loader) %%p\",oclent);\n";

				printf OUT "\ttypedef $retype (*pf_t) ($tlist);\n";

				printf OUT "\t$retype rv \n";
				printf OUT "\t\t= ((pf_t)(*(((void**)oclent)+OCLCALL_$name\)))($alist);\n";
				printf OUT "\treturn rv;\n";	
				printf OUT "}\n\n";

			}

			$i = $i + 1;
		}
	}
}

close(OUT);

