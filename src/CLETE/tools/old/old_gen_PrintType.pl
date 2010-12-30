#!/usr/bin/perl

open(fp,"PrintType.in");
@types = <fp>;
                                                        
printf "/* DAR */\n\n";

printf "#ifndef _PRINTTYPE_H\n";
printf "#define _PRINTTYPE_H\n\n";

printf "#include <iostream>\n";
printf "#include <stdcl.h>\n";
printf "#include <clvector.h>\n";
printf "#include <list>\n";
printf "#include <string>\n";
printf "#include <sstream>\n";
printf "using namespace std;\n\n";

printf "typedef cl_int2 int2;\n";
printf "typedef cl_int4 int4;\n";
printf "typedef cl_float2 float2;\n";
printf "typedef cl_float4 float4;\n";

printf "template <class T>\n";
printf "struct PrintType {\n";
printf "   inline static std::string type_str() { return \"@type-str-unknown@ \"; }\n";
printf "};\n";


foreach $t (@types) {

	$t =~ s/\n//;

	printf "\n// $t\n\n";

	printf "template <>\n";
	printf "struct PrintType< $t > {\n";
	printf "   inline static std::string type_str() { return \"$t\"; }\n";
	printf "};\n";

}

printf "\n#endif\n";


